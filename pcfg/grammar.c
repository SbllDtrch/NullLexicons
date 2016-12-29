/* grammar.c
 *
 * Last modified (c) Mark Johnson, 6th April, 2013
 *
 * modified to read rule bias, followed by rule weight, when reading a rule
 */

#include "grammar.h"
#include "tree.h"
#include "hash-templates.h"
#include "mmm.h"
#include "vindex.h"
#include <assert.h>
#include <ctype.h>
#include <math.h> 
#include <stdio.h>
#include <stdlib.h>


#define NRULESGUESS 128     /* a guess at the number of rules in the grammar */


#define SWAP(a,b) {FLOAT temp=(a); (a)=(b); (b)=temp;} 
#define A(i,j)    a[i*n+j]

void matinv(FLOAT *a, int n) { 
  int *col_index, *row_index, *pivot; 
  int i, icol=0, irow=0, j, k, l, ll; 
  FLOAT pivinv; 
  col_index = MALLOC(n*sizeof(int));
  row_index = MALLOC(n*sizeof(int));
  pivot = CALLOC(n, sizeof(int));

  for (i=0; i<n; i++) { 
    FLOAT max_a = 0.0;
    for (j=0; j<n; j++) 
      if (pivot[j] != 1) 
	for (k=0; k<n; k++) { 
	  if (pivot[k] == 0) { 
	    if (fabs(A(j,k)) > max_a) { 
	      irow=j; 
	      icol=k; 
	      max_a = fabs(A(j,k)); 
	    }} 
	  else
	    assert(pivot[k] <= 1);  /* singular inverse matrix */
	} 
    ++(pivot[icol]);  
    if (irow != icol) { 
      for (l=0; l<n; l++) 
	SWAP(A(irow,l), A(icol,l))
	  } 
    row_index[i] = irow;
    col_index[i] = icol; 
    assert(A(icol,icol) != 0.0);
    pivinv = 1.0/A(icol,icol); 
    A(icol,icol) = 1.0; 
    for (l=0; l<n; l++) 
      A(icol,l) *= pivinv; 
    for (ll=0; ll<n; ll++) 
      if (ll != icol) {    
	FLOAT a_ll_icol = A(ll,icol); 
	A(ll, icol) = 0.0; 
	for (l=0; l<n; l++) 
	  A(ll, l) -= A(icol,l)*a_ll_icol; 
      }} 
  for (l=n-1; l>=0; l--) { 
    if (row_index[l] != col_index[l]) 
      for (k=0; k<n; k++) 
	SWAP(A(k,row_index[l]), A(k, col_index[l])); 
  } 
  FREE(pivot); 
  FREE(row_index); 
  FREE(col_index); 
}


#undef SWAP
#undef A

/* this code maintains the unary rule closure
 */

void
compute_unary_closure(const grammar g, FLOAT minruleprob)
{
  size_t  i, j, n = g->nnts;
  FLOAT   *unary_close = MALLOC(n*n*sizeof(FLOAT));

  /* check that previous closure lists were freed */
  assert(!g->child_parentprob);
  assert(!g->parent_childprob);

  g->child_parentprob = CALLOC(n+1, sizeof(catproblist));
  g->parent_childprob = CALLOC(n+1, sizeof(catproblist));

  /* initialize unary_close to unit matrix */

  for (i=0; i<n; i++)
    for (j=0; j<n; j++)
      unary_close[i*n+j] = (i==j ? 1.0 : 0.0);

  for (i=0; i<g->nrules; i++) {
    vindex v = g->rules[i];
    if (v->n==2 && v->e[1]<=n) {  /* possibly recursive unary rule */
      assert(v->e[0]>0); assert(v->e[1]>0); 
      assert(v->e[0]<=n); assert(v->e[1]<=n);
      assert(unary_close[(v->e[0]-1)*n+v->e[1]-1] == 0.0);
      unary_close[(v->e[0]-1)*n+v->e[1]-1] -= g->weights[i];
    }}

  matinv(unary_close, n);

  { 
    size_t parent, child;

    for (parent=1; parent<=n; parent++)
      for (child=1; child<=n; child++) {
	if (unary_close[(parent-1)*n+child-1] > minruleprob) {
	  catproblist cp = MALLOC(sizeof(struct struct_catproblist));
	  catproblist pp = MALLOC(sizeof(struct struct_catproblist));
	  cp->next = g->parent_childprob[parent];
	  cp->cat = child;
	  cp->prob = unary_close[(parent-1)*n+child-1];
	  g->parent_childprob[parent] = cp;

	  pp->next = g->child_parentprob[child];
	  pp->cat = parent;
	  pp->prob = unary_close[(parent-1)*n+child-1];
	  g->child_parentprob[child] = pp;
	}
      }
  }
  FREE(unary_close);
}


static void
free_catproblists(catproblist *v, size_t n)
{
  int i;
  catproblist p, nextp;
  
  for (i=0; i<=n; i++)
    for (p=v[i]; p; p=nextp) {
      nextp = p->next;
      FREE(p);
    }
  
  FREE(v);
}

void
free_unary_closure(const grammar g)
{
  free_catproblists(g->child_parentprob, g->nnts);
  g->child_parentprob = NULL;
  free_catproblists(g->parent_childprob, g->nnts);
  g->parent_childprob = NULL;
}

void 
write_rule_values(FILE *fp, grammar g, si_t si, FLOAT *values, FLOAT threshold) 
{
  size_t ruleid, child;

  for (ruleid=0; ruleid<g->nrules; ruleid++) {
    assert(g->rules[ruleid]->n>=2);
    if (values[ruleid] >= threshold) {
      fprintf(fp, "%g\t%s " REWRITES, (float) values[ruleid], 
	      si_index_string(si, g->rules[ruleid]->e[0]));
      for (child=1; child<g->rules[ruleid]->n; child++)
	fprintf(fp, " %s", si_index_string(si, g->rules[ruleid]->e[child]));
      fprintf(fp, "\n");
    }
  }
}

void
write_grammar(FILE *fp, grammar g, si_t si, FLOAT threshold)
{
  write_rule_values(fp, g, si, g->weights, threshold);
}

void 
dump_grammar(FILE *fp, grammar g, si_t si) 
{
  size_t ruleid, child;
  si_index right;
  rulelist rp;
  brule    br;

  for (ruleid=0; ruleid<g->nrules; ruleid++) {
    assert(g->rules[ruleid]->n>=2);
    fprintf(fp, "[%d]\t%g\t%s " REWRITES, (int) ruleid, (float) g->weights[ruleid], 
	    si_index_string(si, g->rules[ruleid]->e[0]));
    for (child=1; child<g->rules[ruleid]->n; child++)
      fprintf(fp, " %s", si_index_string(si, g->rules[ruleid]->e[child]));
    fprintf(fp, "\n");
  }

  for (right=1; right<=g->ncats; right++)
    for (rp=g->urules[right]; rp; rp=rp->next) {
      fprintf(fp, "[%d]\t%s " REWRITES " %s\n", (int) rp->ruleid, 
	      si_index_string(si, g->rules[rp->ruleid]->e[0]),
	      si_index_string(si, right) );
      assert(g->rules[rp->ruleid]->n==2);
      assert(g->rules[rp->ruleid]->e[1]==right);
    }

  for (right=1; right<=g->nnts; right++)   /* bug fix suggested by Tetsuo Kiso */
    for (br=g->brules[right]; br; br=br->next) {
      assert(br->active_parent||br->completes);
      if (br->active_parent)
	fprintf(fp, "%s " REWRITES " %s %s\n", si_index_string(si, br->active_parent),
		si_index_string(si, br->left), si_index_string(si, right));
      for (rp=br->completes; rp; rp=rp->next) {
	fprintf(fp, "[%d]\t%s " REWRITES " %s %s\n", (int) rp->ruleid, 
		si_index_string(si, g->rules[rp->ruleid]->e[0]),
		si_index_string(si, br->left),
		si_index_string(si, right) );
	assert(g->rules[rp->ruleid]->n>=3);
	assert(g->rules[rp->ruleid]->e[g->rules[rp->ruleid]->n-1]==right);
      }
    }
}

si_index 
read_cat(FILE *fp, si_t si)
{
  char string[MAXLABELLEN];
  int    c;
  size_t i;

  while ((c = fgetc(fp)) && isspace(c) && (c != '\n'))		/* skip spaces */
    ;

  if ((c == '\n') || (c == EOF)) return(0);			/* line ended, return 0 */

  for (i = 0; (c != EOF) && (!isspace(c)) && (i < MAXLABELLEN); c = fgetc(fp)) 
    string[i++] = c;

  ungetc(c, fp);

  if (i >= MAXLABELLEN) {
    string[MAXLABELLEN-1] = '\0';
    fprintf(stderr, "read_cat() in grammar.c: Category label longer than MAXLABELLEN: %s\n", string);
    exit(EXIT_FAILURE);
  }

  string[i] = '\0';
  return(si_string_index(si, string));
}

static size_t
read_double(FILE *fp, double *f)
{
  int	c = EOF;

  while (!feof(fp) && (c = fgetc(fp)) != EOF && isspace(c))
    ;
  
  if (c != EOF) {
    ungetc(c, fp);
    if (isdigit(c) || c == '+' || c == '-') 
      return fscanf(fp, " %lg ", f);
  }
  return 0;
}

size_t
read_rule(FILE *fp, si_t si, FLOAT *weight, FLOAT* bias, vindex rule, FLOAT rule_bias_default)
{
  size_t n;
  double w = 1.0, b = rule_bias_default;
  si_index cat;

  if (feof(fp))
    return 0;

  read_double(fp, &b);
  read_double(fp, &w);

  if (feof(fp))
    return 0;

  *weight = w;
  *bias = b;

  rule->e[0] = read_cat(fp, si);
  assert(rule->e[0]);
  fscanf(fp, " " REWRITES);	    /* read the rewrites symbol */

  for (n=1; n<MAXRULELEN; n++) {    /* read the rhs, n is rule length */
    cat = read_cat(fp, si);
    if (!cat)
      break;
    rule->e[n] = cat;
  }

  if (n >= MAXRULELEN) {
    fprintf(stderr, "read_rule() in grammar.c: rule too long, "
	    "increase MAXRULELEN\n");
    exit(EXIT_FAILURE);
  }

  rule->n=n;

  return(n);
}


static rulelist
make_rulelist(size_t ruleid, rulelist next)
{
  rulelist r = MALLOC(sizeof(struct struct_rulelist));
  r->ruleid = ruleid;
  r->next = next;
  return(r);
}

static brule
make_brule(si_index left, size_t active_parent, rulelist completes, brule next)
{
  brule b = MALLOC(sizeof(struct struct_brule));
  b->left = left;
  b->active_parent = active_parent;
  b->completes = completes;
  b->next = next;
  return(b);
}

static void
add_active_parent(grammar g, si_index active_parent, 
		  si_index left, si_index right)
{
  brule b;
  assert(right>0);
  assert(right<=g->nnts);
  for (b=g->brules[right]; b&&(b->left!=left); b=b->next)
    ;
  if (b) {
    assert((b->active_parent==0)||(b->active_parent==active_parent));
    b->active_parent=active_parent;
  }
  else 
    g->brules[right] = make_brule(left, active_parent, NULL, g->brules[right]);
}


static void
add_complete_parent(grammar g, si_index left, si_index right, size_t ruleid)
{
  brule b;
  assert(right>0);
  if (right>g->nnts) {
    fprintf(stderr, 
	    "Error in add_complete_parent in grammar.c: terminal appears in non-unary rule\n");
    exit(EXIT_FAILURE);
  }

  for (b=g->brules[right]; b&&(b->left!=left); b=b->next)
    ;

  if (b)
    b->completes = make_rulelist(ruleid, b->completes);
  else 
    g->brules[right] = make_brule(left, 0, make_rulelist(ruleid, 0), 
				  g->brules[right]);
}
    

static void
binarize_grammar(grammar g, si_t si)
{
  size_t ruleid;

  assert(!g->urules);
  assert(!g->brules);
  assert(g->nrules > 0);
  assert(g->rules[0]->n >= 2);

  g->root_label = g->rules[0]->e[0];

  g->urules = CALLOC(g->ncats+1, sizeof(rulelist)); /* adjust for fact that si_index start at 1 */
  g->brules = CALLOC(g->nnts+1, sizeof(brule));
  
  for (ruleid=0; ruleid<g->nrules; ruleid++) {
    vindex rule = g->rules[ruleid];

    switch (rule->n) {
    case 0: 
    case 1:           /* errors */
      fprintf(stderr, "binarize_grammar() in grammar.c: rule %d has length %d\n", 
	      (int) ruleid, (int) rule->n);
      exit(EXIT_FAILURE);
      break;
    case 2:           /* unary rule */
      assert(rule->e[1]>0);
      assert(rule->e[1]<=g->ncats);
      g->urules[rule->e[1]] = make_rulelist(ruleid,g->urules[rule->e[1]]);
      break;
    case 3: {         /* binary rule */
      assert(rule->e[1] <= g->nnts);
      assert(rule->e[2] <= g->nnts);
      add_complete_parent(g, rule->e[1], rule->e[2], ruleid);
      break;
    default:          /* ternary or longer rule */
      { int child, labelpos;
        char label[MAXBLABELLEN], *s;
	si_index parent, left, right;

	parent = rule->e[1];                 /* prime the new label */
	assert(parent <= g->nnts);
	for (labelpos=0, s=si_index_string(si,rule->e[1]); *s; label[labelpos++]=*s++)
	  assert(labelpos<MAXBLABELLEN);

	for (child=2; child<rule->n-1; child++) {  
	  left = parent;
	  right = rule->e[child];
	  if (right>g->nnts) 
	    fprintf(stderr, "## Error in io::grammar.c: terminal category %s"
		    " appears in non-unary rule", si_index_string(si, right));
	  assert(right <= g->nnts);
	  label[labelpos++] = BINSEP;
	  for (s=si_index_string(si,right); *s; label[labelpos++]=*s++)
	    assert(labelpos<MAXBLABELLEN);
	  assert(labelpos<MAXBLABELLEN);
	  label[labelpos] = '\0';
	  parent = si_string_index(si,label);
	  add_active_parent(g, parent, left, right);
	}
	
	left = parent;            /* make the final complete */
	assert(child==rule->n-1);
	right = rule->e[child];
	add_complete_parent(g, left, right, ruleid);
      }
    }}
  }
}

grammar
read_grammar(FILE *fp, si_t si, FLOAT rule_bias_default) 
{
  FLOAT     weight, bias;
  size_t    nrulesmax=NRULESGUESS;
  vindex    rule = make_vindex(MAXRULELEN);
  grammar   g;
  si_t      si0 = make_si(NLABELS);       /* local symbol table */
  vihashst  rule_index = make_vihashst(nrulesmax);
  size_t    i, j;
  size_t    *relabelling;

  assert(si_nstrings(si) == 0);           /* check symbol table is empty */

  g = MALLOC(sizeof(struct struct_grammar));
  g->nrules = 0;
  g->rules = MALLOC(nrulesmax*sizeof(vindex));
  g->weights = MALLOC(nrulesmax*sizeof(FLOAT));
  g->bias = MALLOC(nrulesmax*sizeof(FLOAT));
  g->urules = NULL;
  g->brules = NULL;
  g->child_parentprob = NULL;
  g->parent_childprob = NULL;
  g->root_label = 0;

  while (read_rule(fp, si0, &weight, &bias, rule, rule_bias_default)) {	/* read the rule */
    vihashst_cell_ptr p = vihashst_find(rule_index, rule);
    if (p->value) {
      fprintf(stderr, "%s\n", p->value);
      fprintf(stderr, "read_grammar() in grammar.c: duplicate rule\n");
      exit(EXIT_FAILURE);
    }

    p->value = g->nrules++;                       /* set rule number */
    
    if (g->nrules >= nrulesmax) {                 /* reallocate if necessary */
      assert(g->nrules == nrulesmax);
      nrulesmax *= 2;
      assert(nrulesmax > g->nrules);
      g->rules = REALLOC(g->rules, nrulesmax*sizeof(vindex));
      g->weights = REALLOC(g->weights, nrulesmax*sizeof(FLOAT));
      g->bias = REALLOC(g->bias, nrulesmax*sizeof(FLOAT));
    }
 
    g->weights[p->value] = weight;                 /* save the weight */
    g->bias[p->value] = bias;                      /*  and the bias */
    g->rules[p->value] = vindex_copy(rule);        /*  and the copied rule */
  }

  free_vihashst(rule_index);                       /* free the rule index */
  vindex_free(rule);

  if (g->nrules == 0) {
    fprintf(stderr, "## Error in grammar.c: failed to read any rules "
	    "(g->nrules == 0");
    exit(EXIT_FAILURE);
  }

  g->rules = REALLOC(g->rules, g->nrules*sizeof(vindex));  /* trim arrays */
  g->weights = REALLOC(g->weights, g->nrules*sizeof(FLOAT));
  g->bias = REALLOC(g->bias, g->nrules*sizeof(FLOAT));

  /* relabel the categories so that nonterms are given lower numbers */

  relabelling = CALLOC(si_nstrings(si0)+1, sizeof(size_t));

  /* load the nonterminals into the si symbol table */

  for (i=0; i<g->nrules; i++) {
    size_t parent = g->rules[i]->e[0];
    assert(parent<=si_nstrings(si0));
    if (!relabelling[parent])
      relabelling[parent] = si_string_index(si, si_index_string(si0, parent));
  }

  g->nnts = si_nstrings(si);    /* the si symbol table's size is # of nonterms */

  /* now relabel all rules */

  for (i=0; i<g->nrules; i++) {
    vindex rule = g->rules[i];
    assert(relabelling[rule->e[0]] > 0);
    for (j=0; j<rule->n; j++) {
      size_t cat = rule->e[j];
      assert(cat<=si_nstrings(si0));
      if (!relabelling[cat])
	relabelling[cat] = si_string_index(si, si_index_string(si0, cat));
      rule->e[j] = relabelling[cat];
    }
  }
   
  assert(si_nstrings(si) == si_nstrings(si0));
  FREE(relabelling);
  si_free(si0);

  g->ncats = si_nstrings(si);         /* number of categories so far */
  binarize_grammar(g, si);            /* binarize the grammar */
  return(g);
}

static void
free_rulelist(rulelist r)
{
  while (r) {
    rulelist p=r;
    r=r->next;
    FREE(p);
  }}

static void
free_grammar_data(const grammar g)
{
  size_t i;

  for (i=0; i<g->nrules; i++)
    vindex_free(g->rules[i]);

  FREE(g->rules);
  FREE(g->weights);
  FREE(g->bias);

  for (i=1; i<=g->ncats; i++)            /* free unary rules */
    free_rulelist(g->urules[i]);

  FREE(g->urules);
  g->urules = NULL;

  { brule p, q;
    for (i=1; i<=g->nnts; i++) 
      for (p=g->brules[i]; p; (q=p, p=p->next, FREE(q)))
	free_rulelist(p->completes);
  }

  FREE(g->brules);
  g->brules = NULL;
}

void
free_grammar(grammar g)
{
  free_grammar_data(g);
  FREE(g); 
}

grammar
prune_grammar(const grammar g, const si_t si, FLOAT minruleprob)
{
  size_t  nrulesmax = g->nrules;
  size_t  oldi;
  size_t  nrules = 0;
  vindex  *rules = MALLOC(nrulesmax*sizeof(vindex));
  FLOAT   *weights = MALLOC(nrulesmax*sizeof(FLOAT));
  FLOAT   *bias = MALLOC(nrulesmax*sizeof(FLOAT));

  for (oldi=0; oldi<g->nrules; oldi++)
    if (g->weights[oldi] >= minruleprob) {
      size_t i = nrules++;
      assert(i < g->nrules);
      rules[i] = vindex_copy(g->rules[oldi]);
      weights[i] = g->weights[oldi];
      bias[i] = g->bias[oldi];
    }
  rules = REALLOC(rules, nrules*sizeof(vindex));  /* trim arrays */
  weights = REALLOC(weights, nrules*sizeof(FLOAT));
  bias = REALLOC(bias, nrules*sizeof(FLOAT));


  /* free old grammar stuff */

  free_grammar_data(g);

  /* now move the newly constructed structures into the grammar */
  
  g->nrules = nrules;
  g->rules = rules;
  g->weights = weights;
  g->bias = bias;

  binarize_grammar(g, si);            /* binarize the grammar */
  return(g);
}

grammar
copy_grammar(const grammar g0, si_t si)
{
  grammar g =  MALLOC(sizeof(struct struct_grammar));
  size_t  nrules = g0->nrules;
  size_t  i;

  g->nrules = nrules;
  g->nnts = g0->nnts;
  g->ncats = g0->ncats;
  g->rules = MALLOC(nrules*sizeof(vindex));
  g->weights = MALLOC(nrules*sizeof(FLOAT));
  g->bias = MALLOC(nrules*sizeof(FLOAT));
  g->urules = NULL;
  g->brules = NULL;
  g->child_parentprob = NULL;
  g->parent_childprob = NULL;

  for (i = 0; i < nrules; i++) {
    g->rules[i] = vindex_copy(g0->rules[i]);
    g->weights[i] = g0->weights[i];
    g->bias[i] = g0->bias[i];
  }

  binarize_grammar(g, si);            /* binarize the grammar */
  return(g);
}

void
jitter_weights(grammar g, FLOAT jitter)
{
  int i;
  for (i=0; i<g->nrules; i++) 
    g->weights[i] += g->weights[i]*(jitter*rand());//RAND_MAX;
}
