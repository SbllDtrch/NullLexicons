/* expected-counts.c
 *
 * modified (c) Mark Johnson, 12th July 2004
 *
 * The main routine here is expected_counts, at the bottom of this file.
 */

#include "digamma.h"
#include "expected-counts.h"
#include "mmm.h"		/* memory debugger */
#include "vindex.h"
#include "hash.h"
#include "hash-templates.h"
#include "grammar.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>

/* EPSILON max allowable relative error in preterm outside prob
 *  An error is thrown if inside*outside is greater than this for
 *  any terminal.
 */

#define EPSILON  1e-6    

/****************************************************************************
 *                                                                          *
 *                          Basic chart stuff                               *
 *                                                                          *
 ****************************************************************************/

#define CHART_SIZE(n)			(n)*((n)+1)/2
#define CHART_ENTRY(chart, i, j)	chart[(j)*((j)-1)/2+(i)]

HASH_HEADER_ADD(sihashf, si_index, FLOAT)
HASH_CODE_ADD(sihashf, si_index, FLOAT, IDENTITY, NEQ, IDENTITY,
	      NO_OP, 0.0, NO_OP)

typedef sihashf *chart;

static chart
chart_make(size_t n)
{
  size_t  i, nn = CHART_SIZE(n);
  chart   c = MALLOC(nn*sizeof(sihashf));
  
  for (i=0; i<nn; i++) 
    c[i] = NULL;	/* chart cell will be constructed in apply_unary() */ 
  
  return c;
}

static void
chart_free(chart c, size_t n)
{
  size_t i;

  for (i=0; i<CHART_SIZE(n); i++)
    free_sihashf(c[i]);

  FREE(c);
}


static void
chart_entry_display(FILE *fp, sihashf chart_entry, si_t si)
{
  sihashfit hit;

  for (hit=sihashfit_init(chart_entry); sihashfit_ok(hit); 
       hit = sihashfit_next(hit))
    fprintf(fp, " %s: %g\n", si_index_string(si, hit.key), (float) hit.value);
}

void
chart_display(FILE *fp, chart c, size_t n, si_t si)
{
  size_t left, gap;

  for (gap=1; gap<=n; gap++)
    for (left=0; left<=n-gap; left++) {
      sihashf chart_cell = CHART_ENTRY(c, left, left+gap);
      if (sihashf_size(chart_cell) > 0) {
	fprintf(fp, "cell %d-%d\n", (int) left, (int) (left+gap));
	chart_entry_display(fp, chart_cell, si);
      }
    }
}


/****************************************************************************
 *                                                                          *
 *                             Inside pass                                  *
 *                                                                          *
 ****************************************************************************/

static void
binary_inside(const grammar g, sihashf parent_entry, sihashf parent_completes,
	      sihashf left_entry, sihashf right_entry)
{
  si_index  right;

  /* if either the left chart cell or the right chart cell is empty,
   * then we have nothing to do
   */

  if (sihashf_size(left_entry) == 0 || sihashf_size(right_entry) == 0)
    return;

  for (right=1; right<=g->nnts; right++) {
    brule bp = g->brules[right];
    if (bp) {
      /* look up the rule's right category */
      FLOAT cr = sihashf_ref(right_entry, right);  
      if (cr>0.0)				   
	/* such categories exist in this cell */
	for ( ; bp; bp=bp->next) {
	  FLOAT cl;
	  cl = sihashf_ref(left_entry, bp->left);
	  if (cl>0.0) {
	    rulelist rp;
	    if (bp->active_parent)
	      /* actives go straight into chart */
	      sihashf_inc(parent_entry, bp->active_parent, cl*cr);
	    for (rp=bp->completes; rp; rp=rp->next) {
	      si_index parent_cat = g->rules[rp->ruleid]->e[0];
	      assert(parent_cat <= g->nnts);
	      sihashf_inc(parent_completes, parent_cat, 
			  cl*cr*g->weights[rp->ruleid]);
	    }}}}}
}


static void
unary_closure_inside(const grammar g, sihashf parent_entry, 
		     sihashf parent_completes)
{
  sihashfit childit;

  for (childit = sihashfit_init(parent_completes); 
       sihashfit_ok(childit); childit = sihashfit_next(childit)) {
    catproblist pp;
    for (pp = g->child_parentprob[childit.key]; pp; pp = pp->next)
      sihashf_inc(parent_entry, pp->cat, childit.value*pp->prob);
  }
}


static chart
inside_chart(vindex terms, grammar g, si_t si, FLOAT wordscale)
{
  int left, right, mid;
  chart c = chart_make(terms->n);

  /* parent_completes is a sihashf of completed categories (i.e., not the
   * new, active categories produced by binarization).  Unary closure
   * applies to these categories.
   * The pre-unary-closure parent weights are stored in parent_completes
   * before unary closure is applied to them
   */

  /* Inside pass */

  /* insert lexical items */

  for (left=0; left< (int) terms->n; left++) {
    rulelist    rl;
    si_index	terminal = terms->e[left];
    sihashf	chart_entry = make_sihashf(NLABELS);

    CHART_ENTRY(c, left, left+1) = chart_entry;

    assert(terminal>0);
    if (terminal<=g->nnts) {
      fprintf(stderr, 
	      "Error in inside_chart() in expected-counts.c: "
	      "input contains nonterminal symbol %s\n", 
	      si_index_string(si, terminal));
      exit(EXIT_FAILURE);
    }
    if (terminal>g->ncats) {
      fprintf(stderr, 
	      "Error in inside_chart() in expected-counts.c:"
	      " input contains unknown terminal %s\n", 
	      si_index_string(si, terminal));
      exit(EXIT_FAILURE);
    }

    /* no need to actually enter terminal into chart */
    /* sihashf_set(chart_entry, terminal, 1.0); */

    rl = g->urules[terminal];
    assert(rl);   /* check there are rules for this terminal */
    for ( ; rl; rl=rl->next) {
      si_index preterminal = g->rules[rl->ruleid]->e[0];
      FLOAT preterminal_prob = g->weights[rl->ruleid]*wordscale;
      catproblist pp;
      /* assert(rl->ruleid<g->nrules); */
      assert(g->child_parentprob[preterminal]);
      for (pp = g->child_parentprob[preterminal]; pp; pp = pp->next)
	sihashf_inc(chart_entry, pp->cat, preterminal_prob*pp->prob);
    }

    /* fprintf(stderr, "Chart entry %d-%d\n", (int) left, (int) left+1);
       chart_entry_display(stderr, chart_entry, si); */
  }

  for (right=2; right<= (int) terms->n; right++)
    for (left=right-2; left>=0; left--) {
      sihashf parent_completes = make_sihashf(COMPLETE_CELLS);
      sihashf chart_entry = make_sihashf(CHART_CELLS);   
      CHART_ENTRY(c, left, right) = chart_entry;

      for (mid=left+1; mid<right; mid++) 
	binary_inside(g, chart_entry, parent_completes,
		      CHART_ENTRY(c,left,mid), CHART_ENTRY(c,mid,right));

      unary_closure_inside(g, chart_entry, parent_completes);
      free_sihashf(parent_completes);
      
      /* fprintf(stdout, "Chart entry %d-%d\n", (int) left, (int) right); */
      /* chart_entry_display(stdout, CHART_ENTRY(inside,left,right), si); */   
    }

  return c;
}


/*****************************************************************************
 *                                                                           *
 *                             Outside pass                                  *
 *                                                                           *
 *****************************************************************************/

static void
increment_unary_counts(const grammar g, 
		       const sihashf inside_cell, const sihashf outside_cell, 
		       FLOAT root_prob, FLOAT *rule_counts)
{ 
  si_index child_cat;

  for (child_cat=1; child_cat<=g->nnts; child_cat++) {  
    rulelist rp;
    FLOAT inside_weight = sihashf_ref(inside_cell, child_cat);
    if (inside_weight>0.0)
      for (rp=g->urules[child_cat]; rp; rp=rp->next) {
	FLOAT outside_weight = sihashf_ref(outside_cell, 
					   g->rules[rp->ruleid]->e[0]);
	if (outside_weight>0.0) 
	  rule_counts[rp->ruleid] += inside_weight*outside_weight*
	                             g->weights[rp->ruleid]/root_prob;
      }}}


static void
binary_outside(const grammar g, size_t left_pos, size_t right_pos, 
	       size_t nwords, const chart inside_chart, chart outside_chart, 
	       FLOAT root_prob, FLOAT *rule_counts)
{
  si_index  right_cat;
  FLOAT	    *completes;
  sihashf   child_outside = make_sihashf(CHART_CELLS);
  sihashf   child_inside = CHART_ENTRY(inside_chart, left_pos, right_pos);

  CHART_ENTRY(outside_chart, left_pos, right_pos) = child_outside;
  
  /* if the inside chart cell is empty, then there's no point calculating
   * the outside cell
   */

  if (sihashf_size(child_inside) == 0)
    return;

  completes = MALLOC((g->nnts+1)*sizeof(FLOAT));
  { size_t child_cat;
    for (child_cat=1; child_cat<=g->nnts; child_cat++)
      completes[child_cat] = 0.0;   
  } 

  /* try to combine with cells on left */

  for (right_cat=1; right_cat<=g->nnts; right_cat++) {
    brule bp = g->brules[right_cat];
    FLOAT right_inside_weight = sihashf_ref(child_inside, right_cat);

    /* rules and inside category? */
    if (bp&&(right_inside_weight>0.0)) {
      for ( ; bp; bp=bp->next) {
	size_t far_left_pos;
	for (far_left_pos=0; far_left_pos<left_pos; far_left_pos++) {
	  FLOAT far_left_inside_weight =
	    sihashf_ref(CHART_ENTRY(inside_chart, far_left_pos, left_pos), 
			bp->left);
	  if (far_left_inside_weight>0.0) {
	    rulelist rp;
	    if (bp->active_parent) {
	      FLOAT parent_outside_weight =
		sihashf_ref(CHART_ENTRY(outside_chart,far_left_pos,right_pos), 
			    bp->active_parent);
	      if (parent_outside_weight>0.0)
		completes[right_cat] += parent_outside_weight*
		                        far_left_inside_weight;
	    }
	    for (rp=bp->completes; rp; rp=rp->next) {
	      FLOAT parent_outside_weight = 
		sihashf_ref(CHART_ENTRY(outside_chart,far_left_pos,right_pos), 
			    g->rules[rp->ruleid]->e[0]);
	      if (parent_outside_weight>0.0) {
		FLOAT parent_left_rule_weight = parent_outside_weight*
		                                far_left_inside_weight*
                                                g->weights[rp->ruleid];
		completes[right_cat] += parent_left_rule_weight ;
		rule_counts[rp->ruleid] += parent_left_rule_weight*
		                           right_inside_weight/root_prob;
	      }}}}}}}

  /* try to combine with cells on right */

  for (right_cat=1; right_cat<=g->nnts; right_cat++) {
    brule bp;
    size_t far_right_pos;
    for (bp=g->brules[right_cat]; bp; bp=bp->next)
      for (far_right_pos=right_pos+1; far_right_pos<=nwords; far_right_pos++) {
	FLOAT far_right_inside_weight = 
	      sihashf_ref(CHART_ENTRY(inside_chart, right_pos, far_right_pos),
			  right_cat);
	si_index child_cat=bp->left;
	FLOAT    child_inside_weight = sihashf_ref(child_inside, child_cat);
	if ((far_right_inside_weight>0.0)&&(child_inside_weight>0.0)) {
	  rulelist rp;
	  if (bp->active_parent) {
	    FLOAT parent_outside_weight =
	      sihashf_ref(CHART_ENTRY(outside_chart, left_pos, far_right_pos),
			  bp->active_parent);
	    if (parent_outside_weight>0.0) {
	      if (child_cat<=g->nnts)
		completes[child_cat] += parent_outside_weight*
		                        far_right_inside_weight;
	      else {
		assert(child_cat>g->ncats); /* otherwise child_cat is a term */
		sihashf_inc(child_outside, child_cat, 
			    parent_outside_weight*far_right_inside_weight);
	      }
	    }}
	  for (rp=bp->completes; rp; rp=rp->next) {
	    FLOAT parent_outside_weight =
	      sihashf_ref(CHART_ENTRY(outside_chart, left_pos, far_right_pos),
			  g->rules[rp->ruleid]->e[0]);
	    if (parent_outside_weight>0.0) {
	      FLOAT parent_right_rule_weight = parent_outside_weight*
		                               far_right_inside_weight*
		                               g->weights[rp->ruleid];
	      if (child_cat<=g->nnts)
		completes[child_cat] += parent_right_rule_weight;
	      else {
		assert(child_cat>g->ncats); /* otherwise child_cat is a term */
		sihashf_inc(child_outside,child_cat,parent_right_rule_weight);
	      }
	      /* don't double count the rule! 
               * it's been counted before from the left 
	       */
	      /* rule_counts[rp->ruleid] += parent_right_rule_weight*
                                            child_inside_weight/root_prob; 
	       */
	    }}}}}

  /* unary closure */  
  /* unary closure for root cell done in outside_chart() */

  { si_index parent_cat;
  
    for (parent_cat=1; parent_cat<=g->nnts; parent_cat++) {
      FLOAT parent_outside_weight = completes[parent_cat];
      if (parent_outside_weight>0.0) {
	catproblist cp;
	for (cp = g->parent_childprob[parent_cat]; cp; cp = cp->next) 
	  if (sihashf_ref(child_inside, cp->cat) > 0.0)
	    sihashf_inc(child_outside, cp->cat, cp->prob*parent_outside_weight);
      }}}

  /* increment unary rule_counts */ 
  /* rule counts for root cell done in outside_chart() */
  increment_unary_counts(g, child_inside, child_outside, root_prob,
			 rule_counts);

  FREE(completes);
}

static chart
outside_chart(const grammar g, const si_t si, const chart inside_chart,
	      const vindex terms, FLOAT yieldweight, FLOAT *rule_counts)
{
  int      left, right;
  size_t   nwords = terms->n;
  sihashf  root_inside_cell = CHART_ENTRY(inside_chart, 0, nwords);
  FLOAT    root_prob = sihashf_ref(root_inside_cell, g->root_label);
  chart    outside_chart = chart_make(nwords);
  sihashf  root_outside_cell = make_sihashf(CHART_CELLS);
  catproblist cp;
  
  root_prob /= yieldweight; /* pretend we saw this sentence this many times */

  /* install root cell */
  CHART_ENTRY(outside_chart, 0, nwords) = root_outside_cell; 

  for (cp = g->parent_childprob[g->root_label]; cp; cp = cp->next) 
    if (sihashf_ref(root_inside_cell, cp->cat) > 0.0)
      sihashf_set(root_outside_cell, cp->cat, cp->prob);

  increment_unary_counts(g, root_inside_cell, root_outside_cell, root_prob, 
			 rule_counts);

  for (right=nwords; right>=1; right--)
    for (left=0; left<right; left++)
      if ((left!=0)||(right!=nwords))         /* skip root cell */
	binary_outside(g, left, right, nwords, 
		       inside_chart, outside_chart, root_prob, rule_counts);

  /* now update counts for unary rules expanding to terminals */

  for (left=0; left < nwords; left++) {
    rulelist rl;
    sihashf  outside_cell = CHART_ENTRY(outside_chart, left, left+1);
    for (rl=g->urules[terms->e[left]]; rl; rl=rl->next) {
      FLOAT outside_prob = sihashf_ref(outside_cell,
				       g->rules[rl->ruleid]->e[0]) *
                           g->weights[rl->ruleid];
      rule_counts[rl->ruleid] += outside_prob/root_prob;
      sihashf_inc(outside_cell, terms->e[left], outside_prob);
    }
  }

  return outside_chart;
}

int
consistent_preterm_outsides(chart outside, vindex terms,
			    FLOAT root_prob)
{
  size_t i;

  for (i=0; i<terms->n; i++) 
    if (fabs(sihashf_ref(CHART_ENTRY(outside,i,i+1),terms->e[i])-root_prob)/
	 root_prob 
	> EPSILON)
      return(0);
  
  return(1);
}

static vindex
read_terms(int weighted_yields_flag, FILE *fp, si_t si, float *yieldweight)
{
  *yieldweight = 1.0;
  if (weighted_yields_flag) {
    int result = fscanf(fp, "%g", yieldweight);

    if (result == EOF) 
      return(NULL);
    if (result != 1) {
      fprintf(stderr, "Error in read_terms in expected_counts.c: "
	      "Unable to read a weight for yield\n");
      exit(EXIT_FAILURE);
    }
  }
  
  {
    size_t i = 0, nsize = 10;
    vindex v = make_vindex(nsize);
    si_index term;

    while ((term = read_cat(fp, si))) {
      if (i >= nsize) {
	nsize *= 2;
	vindex_resize(v, nsize);
      }
      assert(i < nsize);
      vindex_ref(v,i++) = term;
    }
    
    if (i > 0) {
      v->n = i;
      vindex_resize(v, v->n);
      return (v);
    }
    else {
      vindex_free(v);
      return(NULL);
    }
  }
}

FLOAT 
expected_rule_counts(const grammar g, const si_t si, FILE *yieldfp, 
		     FILE *tracefp, FILE *summaryfp, int debuglevel,
		     int maxsentlen, FLOAT minruleprob, FLOAT wordscale,
		     FLOAT *rule_counts, FLOAT *sum_yieldweights,
		     int weighted_yields_flag)
{
  vindex  terms;
  FLOAT	  root_prob;
  chart	  inside, outside;
  long    sentenceno = 0, parsed_sentences = 0, failed_sentences = 0;
  double  sum_neglog_prob = 0.0;
  float   yieldweight;

  *sum_yieldweights = 0;
  /*  FLOAT *rule_counts = CALLOC(g->nrules, sizeof(FLOAT)); */

  { size_t i;                    /* zero rule counts */
    for (i=0; i<g->nrules; i++)
      rule_counts[i] = 0.0;
  }

  compute_unary_closure(g, minruleprob); /* compute unary_close */

  rewind(yieldfp);               /* rewind the tree file */

  while ((terms = read_terms(weighted_yields_flag, yieldfp, si, &yieldweight))) {
    sentenceno++;

    if (summaryfp && debuglevel >= 10000) {
      size_t	i;
      fprintf(tracefp, "\nSentence %ld:\n", sentenceno);

      for (i=0; i<terms->n; i++)
	fprintf(tracefp, " %s", si_index_string(si, terms->e[i]));
      fprintf(tracefp, "\n");
    }
 
    /* skip if sentence is too long */
    if (!maxsentlen || (int) terms->n <= maxsentlen) {
      inside = inside_chart(terms, g, si, wordscale);
      /* chart_display(stdout, inside, terms->n, si);  */
      root_prob = sihashf_ref(CHART_ENTRY(inside, 0, terms->n), 
			      g->root_label);

      if (root_prob > 0.0) {
	if (tracefp && debuglevel >= 10000)
	  fprintf(tracefp, "Sum of derivation weights = %g\n", 
		  root_prob);
	sum_neglog_prob -= yieldweight*(log(root_prob)-terms->n*log(wordscale));
	*sum_yieldweights += yieldweight*terms->n;
	parsed_sentences++;
	outside = outside_chart(g, si, inside, terms, yieldweight, rule_counts);
	/* assert(consistent_preterm_outsides(outside, terms, root_prob)); */
	/* chart_display(stdout, outside, terms->n, si); */
	chart_free(outside, terms->n);
      }
      else {
	failed_sentences++;
	if (tracefp && debuglevel >= 10000)
	  fprintf(tracefp, "Failed to parse.\n");
      }
      chart_free(inside, terms->n);		/* free the chart */
    }
    else { 					/* sentence too long */
      if (tracefp && debuglevel >= 10000)
	fprintf(tracefp, "Too long to parse.\n");
    }
    vindex_free(terms);				/*  and its terms */
  }

  /* free unary closure */
  free_unary_closure(g);

  if (summaryfp && debuglevel >= 1000) {
    if (failed_sentences>0)
      fprintf(summaryfp, " %ld sentences failed to parse",
	      (long) failed_sentences);
  }
  return(sum_neglog_prob);
}

void add_bias(grammar g, FLOAT *rule_counts)
{
  size_t i;
  for (i = 0; i < g->nrules; i++) 
    if (g->bias[i] != 0)
      {
	FLOAT rule_count = rule_counts[i] + g->bias[i];
	if (rule_count > 0)
	  rule_counts[i] = rule_count;
	else
	  rule_counts[i] = 0;
      }
}

void
set_rule_weights(grammar g, FLOAT *rule_counts, int VariationalBayes)
{
  FLOAT *parent_sum = MALLOC((g->nnts+1)*sizeof(FLOAT));
  size_t i;

  for (i=0; i<=g->nnts; i++)
    parent_sum[i] = 0.0;

  for (i=0; i<g->nrules; i++) {
    assert(g->rules[i]->e[0] <= g->nnts);
    assert(rule_counts[i] >= 0.0);
    parent_sum[g->rules[i]->e[0]] += rule_counts[i];
  }

  for (i=0; i<g->nrules; i++) {
    if (rule_counts[i] > 0.0) {
      if (VariationalBayes)
	g->weights[i] = exp(digamma(rule_counts[i]));// - digamma(parent_sum[g->rules[i]->e[0]]));
      else
	g->weights[i] = rule_counts[i];///parent_sum[g->rules[i]->e[0]];
    }
    else
      g->weights[i] = 0.0;
  }
  FREE(parent_sum);
}
    

void
scale_weights(grammar g, FLOAT beta)
{
  int i;
  for (i=0; i<g->nrules; i++) 
    if (g->weights[i] > 0)
      g->weights[i] = pow(g->weights[i], beta);
}

FLOAT
inside_outside(grammar g, const si_t si, FILE *yieldfp, 
	       FILE *tracefp, FILE *summaryfp, int debuglevel,
	       int maxsentlen, int minits, int maxits,
	       FLOAT stoptol, FLOAT minruleprob,
	       FLOAT jitter, int VariationalBayes, FLOAT wordscale,
	       FLOAT annealstart, FLOAT annealstop, int nanneal,
	       int weighted_yields_flag)
{
  FLOAT *rule_counts = CALLOC(g->nrules, sizeof(FLOAT));
  FLOAT sum_neglog_prob0;
  FLOAT sum_neglog_prob;
  int   iteration = 0;
  size_t nrules, nrules0;
  FLOAT sum_yieldweights;
  FLOAT temperature = annealstart;

  nrules = g->nrules;

  if (summaryfp && debuglevel >= 1000) {
    if (debuglevel < 5000)
      fprintf(summaryfp, "# Iteration\ttemperature\tnrules\t-logP\tbits/token\n%d\t%g\t%d", 
	      iteration, temperature, (int) nrules);
    else
      fprintf(summaryfp, "# Iteration %d, temperature = %g, %d rules, ",
	      iteration, temperature, (int) nrules);
    fflush(summaryfp);
  }

  sum_neglog_prob0 = expected_rule_counts(g, si, yieldfp, tracefp, 
					  summaryfp, debuglevel,
					  maxsentlen, minruleprob, wordscale,
					  rule_counts, &sum_yieldweights,
					  weighted_yields_flag);

  if (summaryfp && debuglevel >= 1000) {
    if (debuglevel < 5000)
      fprintf(summaryfp, "\t%g\t%g\n", sum_neglog_prob0,
	      sum_neglog_prob0/(log(2)*(sum_yieldweights)));
    else
      fprintf(summaryfp, "-logP = %g, bits/token = %g.\n", sum_neglog_prob0,
	      sum_neglog_prob0/(log(2)*(sum_yieldweights)));
    fflush(summaryfp);
  }

  if (tracefp && debuglevel >= 10000) {
    write_rule_values(tracefp, g, si, rule_counts, 0);
    fprintf(tracefp, "\n");
    fflush(tracefp);
  }

  if (summaryfp && debuglevel >= 5000 && debuglevel < 10000)
    write_grammar(summaryfp, g, si, minruleprob);      

  while (1) {
    ++iteration;

    add_bias(g, rule_counts);
    set_rule_weights(g, rule_counts, VariationalBayes);
    prune_grammar(g, si, minruleprob);
    if (jitter != 0) 
      jitter_weights(g, jitter);
    set_rule_weights(g, g->weights, 0);
    if (iteration < nanneal) {
      temperature = annealstart*pow(annealstop/annealstart, (iteration-1.0)/(nanneal-1.0));
      scale_weights(g, 1.0/temperature);
    }
    else
      temperature = 1.0;
    nrules0 = nrules;
    nrules = g->nrules;

    if (summaryfp && debuglevel >= 1000) {
      if (debuglevel < 5000)
	fprintf(summaryfp, "%d\t%g\t%d", iteration, temperature, (int) nrules);
      else
	fprintf(summaryfp, "# Iteration %d, temperature %g, %d rules, ",
		iteration, temperature, (int) nrules);
      fflush(summaryfp);
    }

    sum_neglog_prob = expected_rule_counts(g, si, yieldfp, tracefp, summaryfp, debuglevel,
					   maxsentlen, minruleprob, wordscale,
					   rule_counts, &sum_yieldweights, weighted_yields_flag);

    if (summaryfp && debuglevel >= 1000) {
      if (debuglevel < 5000)
	fprintf(summaryfp, "\t%g\t%g\n", sum_neglog_prob,
		sum_neglog_prob/(log(2)*(sum_yieldweights)));
      else
	fprintf(summaryfp, "-logP = %g, bits/token = %g.\n", sum_neglog_prob,
		sum_neglog_prob/(log(2)*(sum_yieldweights)));
      fflush(summaryfp);
    }

    if (tracefp && debuglevel >= 10000) {
      write_rule_values(tracefp, g, si, rule_counts, 0);
      fprintf(tracefp, "\n");
      fflush(tracefp);
    }
    
    if (summaryfp && debuglevel >= 5000 && debuglevel < 10000)
      write_grammar(summaryfp, g, si, minruleprob);      

    if (nrules==nrules0 &&
	iteration >= minits &&
	((maxits > 0 && iteration >= maxits)
	 || (sum_neglog_prob0-sum_neglog_prob)/fabs(sum_neglog_prob) < stoptol))
      break;

    sum_neglog_prob0 = sum_neglog_prob;
  }

  FREE(rule_counts);

  return(sum_neglog_prob/(log(2)*sum_yieldweights));
}

/*
FLOAT
inside_outside_H(grammar g, const si_t si, FILE *yieldfp, 
		 FILE *tracefp, FILE *summaryfp, int debuglevel,
		 int maxsentlen, int minits, int maxits,
		 FLOAT stoptol, FLOAT minruleprob,
		 FLOAT jitter, FLOAT beta, FLOAT Hfactor)
{
  FLOAT *rule_counts = CALLOC(g->nrules, sizeof(FLOAT));
  FLOAT sum_neglog_prob0;
  FLOAT sum_neglog_prob;
  int   iteration = 0;
  size_t nrules, nrules0;
  FLOAT sum_yieldweights;
  FLOAT H, Q, Q0;
  int   i;

  if (summaryfp && debuglevel >= 1000)
    fprintf(summaryfp, "Iteration %d, nrules = %ld, ", iteration, (long) g->nrules);

  nrules = g->nrules;
  sum_neglog_prob0 = expected_rule_counts(g, si, yieldfp, tracefp, 
					  summaryfp, debuglevel,
					  maxsentlen, minruleprob,
					  rule_counts, &sum_yieldweights);

  Q0 = sum_neglog_prob0;
  H = 0;
  for (i = 0; i < g->nrules; ++i) {
    FLOAT w = g->weights[i];
    H -= w * log(w);
  }
  Q0 += Hfactor * H;
    
  if (summaryfp && debuglevel >= 1000) 
    fprintf(summaryfp, "-log P = %g, entropy = %g bits per token, Q = %g, H = %g\n", 
	    sum_neglog_prob0,
	    sum_neglog_prob0/(log(2)*(sum_yieldweights)), Q0, H);

  if (tracefp && debuglevel > 10000)
    write_rule_values(tracefp, g, si, rule_counts, 0);

  while (1) {
    ++iteration;

    for (i = 0; i < g->nrules; ++i) {
      FLOAT w = g->weights[i];
      rule_counts[i] += Hfactor * w * (1 + log(w));
      if (rule_counts[i] < minruleprob)
	rule_counts[i] = minruleprob;
    }

    set_rule_weights(g, rule_counts);
    prune_grammar(g, si, minruleprob);
    if (jitter != 0)
      jitter_weights(g, jitter);
    set_rule_weights(g, g->weights);
    if (beta != 1)
      scale_weights(g, beta);
    nrules0 = nrules;
    nrules = g->nrules;

    if (summaryfp && debuglevel >= 1000)
      fprintf(summaryfp, "Iteration %d, nrules = %ld, ", iteration, (long) g->nrules);

    sum_neglog_prob = expected_rule_counts(g, si, yieldfp, tracefp, summaryfp, debuglevel,
					   maxsentlen, minruleprob, 
					   rule_counts, &sum_yieldweights);

    Q = sum_neglog_prob0;
    H = 0;
    for (i = 0; i < g->nrules; ++i) {
      FLOAT w = g->weights[i];
      H -= w * log(w);
    }
    Q += Hfactor * H;
    
    if (summaryfp && debuglevel >= 1000) 
      fprintf(summaryfp, "- log P = %g, entropy = %g bits per token, Q = %g, H = %g\n", 
	      sum_neglog_prob,
	      sum_neglog_prob/(log(2)*(sum_yieldweights)), Q, H);

    if (tracefp && debuglevel >= 10000)
      write_rule_values(tracefp, g, si, rule_counts, 0);

    if (nrules==nrules0 &&
	iteration >= minits &&
	((maxits > 0 && iteration >= maxits)
	 || (Q0-Q)/fabs(Q) < stoptol))
      break;

    sum_neglog_prob0 = sum_neglog_prob;
    Q0 = Q;
    add_bias(g, rule_counts);
  }

  FREE(rule_counts);

  return(sum_neglog_prob/(log(2)*sum_yieldweights));
}
*/
