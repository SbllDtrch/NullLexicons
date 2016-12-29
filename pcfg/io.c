/* io.c
 *
 * (c) Mark Johnson, 2nd September 2007, 20th October 2012
 */

const char usage[] =
"io: The Inside-Outside algorithm for maximum likelihood PCFG estimation,\n"
"    now modified to optionally perform Variational Bayes.\n"
"\n"
"Written by (c) Mark.Johnson@MQ.edu.au in 1998, last modified 20th October 2012.\n"
"\n"
"Usage:\n"
"\n"
"io [-d debuglevel] [-R nruns] [-a alpha] [-c]\n"
"   [-g grammar] [-s stoptol] [-p prune] [-l maxlen] [-m minits] [-n maxits]\n"
"   [-S randseed] [-j jitter] [-J per-iteration-jitter] [-V] [-W wordscale]\n"
"   [-b annealstart] [-B annealstop] [-N nanneal] [-T tracefile] yieldfile\n"
"\n"
"where:\n"
"\n"
" yieldfile is a file containing the strings to be parsed, one per line, prefixed by a float count\n"
"      if the -c flag is specified\n"
"\n"
" grammar is a file containing the rules of the grammar to be re-estimated (default: stdin),\n"
"\n"
" re-estimation stops when the negative log probability of the strings changes less than stoptol\n"
"     (default: 1e-7)\n"
"\n"
" rules with lower probability than prune (default 0) are pruned from the grammar during training,\n"
"\n"
" sentences longer than maxlen are ignored during training (default: include all sentences),\n"
"\n"
" at least minits of EM training are performed (default 1),\n"
"\n"
" at most maxits of EM training are performed (default: run until convergence),\n"
"\n"
" a bias count of alpha is added to all rules without an explicit bias (i.e., alpha is the default\n"
"    Dirichlet prior; default = 0)\n"
"\n"
" the annealing temperature is adjusted from annealstart to annealstop in nanneal steps,\n"
"\n"
" positive values of debug cause increasingly more debugging information to be written (default: 0),\n"
"\n"
" after each EM re-estimation the current likelihood is written tracefile.\n"
"\n"
"The grammar should consist of rules, one per line, of the format\n"
"\n"
"   [bias [initial-weight]] Parent --> Child1 Child2 ...\n"
"\n"
"where initial-weight is the initial rule probability used to start EM, and\n"
"      bias is a pseudo-count added to this rule at each iteration\n"
"\n"
"The start category is the Parent of the first rule in the grammar.\n"
"\n"
"The -V flag forces it to perform Variational Bayes inference, i.e., updating the rule\n"
"probabilities by passing them through exp(Digamma(.)), as described in\n"
"Kenichi Kurihara and Taisuke Sato. 2006. \"Variational Bayesian grammar induction for natural language\".\n"
"In 8th International Colloquium on Grammatical Inference.\n"
"\n"
"If you perform Variational Bayes, you should ensure that all rules have a positive bias.\n"
"\n"
"WARNING: as of 2nd Sept 2007, I haven't updated the program to print out the Variational Posterior\n"
"likelihood (it still prints out the EM log likelihood).  So don't believe these values if you're using\n"
"Variational Bayes, and don't get worried if they go UP in later iterations.\n"
"\n"
"-W wordscale causes all lexical probabilities to be multiplied by wordscale, and for the final\n"
"tree probabilities to be divided by the corresponding factor (wordscale raised to the power of\n"
"the number of words in the sentence).  This is an easy way of avoiding underflow in very long sentences.\n"
"\n";

#include "local-trees.h"
#include "grammar.h"
#include "expected-counts.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static grammar g_global = NULL;
static si_t    si_global;

static FLOAT minruleprob = 0;     /* prune rules with less than this weight */

static void
write_grammar_(int interrupt)
{
  if (g_global)
    write_grammar(stdout, g_global, si_global, minruleprob);
  exit(EXIT_FAILURE);
}

int      
main(int argc, char **argv)
{
  si_t  si = make_si(1024);
  FILE  *grammarfp = stdin;
  FILE  *yieldfp;
  FILE	*tracefp = stderr;  	/* set this to NULL to stop trace output */
  FILE	*summaryfp = stdout;	/* set this to NULL to stop parse stats */
  grammar g0, g = NULL;
  int	maxlen = 0, minits = 0, maxits = 0;
  FLOAT stoptol = 1e-7;
  int nanneal = 0, randseed = 97;
  FLOAT rule_bias_default = 0;
  FLOAT annealstart = 1, annealstop = 1;
  FLOAT jitter0 = 0, jitter = 0;
  int debuglevel = 0, nruns = 1, irun = 0;
  FLOAT wordscale=1;
  int VariationalBayes=0, weighted_yields_flag=0;

  {
    int chr;
    
    while ((chr = getopt(argc, argv, "a:g:s:p:l:m:n:d:t:b:cB:N:j:J:S:d:R:T:VW:")) != -1) 
      switch (chr) {
      case 'a':
	rule_bias_default = atof(optarg);
	break;
      case 'g': 
	grammarfp = fopen(optarg, "r");
	if (grammarfp == NULL) {
	  fprintf(stderr, "Error: couldn't open grammarfile %s\n%s",
		  optarg, usage);
	  exit(EXIT_FAILURE);
	}
	break;
      case 'c':
	weighted_yields_flag=1;
	break;
      case 's':
	stoptol = atof(optarg);
	break;
      case 'p':
	minruleprob = atof(optarg);
	break;
      case 'l':
	maxlen = atoi(optarg);
	break;
      case 'm':
	minits = atoi(optarg);
	break;
      case 'n':
	maxits = atoi(optarg);
	break;
      case 'b':
	annealstart = atof(optarg);
	break;
      case 'B':
	annealstop = atof(optarg);
	break;
      case 'N':
	nanneal = atoi(optarg);
	break;
      case 'j':
	jitter0 = atof(optarg);
	break;
      case 'J':
	jitter = atof(optarg);
	break;
      case 'S': 
	randseed = atoi(optarg);
	break;
      case 'd':
	debuglevel = atoi(optarg);
	break;
      case 'R':
	nruns = atoi(optarg);
	break;
      case 'T':
	summaryfp = fopen(optarg, "w");
	break;
      case 'V':
	VariationalBayes = 1;
	break;
      case 'W':
	wordscale = atof(optarg);
	break;
      case '?':
      default:
	fprintf(stderr, "Error: unknown command line flag %c\n\n%s\n",
		chr, usage);
	exit(EXIT_FAILURE);
	break;
      }
  }
    
  if (optind + 1 != argc) {
    fprintf(stderr, "Error: expect a yieldfile\n\n%s\n", usage);
    exit(EXIT_FAILURE);
  }

  if ((yieldfp = fopen(argv[optind], "r")) == NULL) {
    fprintf(stderr, "Error: Couldn't open yieldfile %s\n%s", argv[optind], usage);
    exit(EXIT_FAILURE);
  }
   
  srand(randseed);
  if (summaryfp && debuglevel >= 100)
    fprintf(summaryfp, "# rule_bias_default (-a) = %g, stoptol (-s) = %g, minruleprob (-p) = %g, "
	    "maxlen (-l) = %d, minits (-m) = %d, maxits = (-n) = %d, annealstart (-b) = %g, "
	    "annealstop (-B) = %g, nanneal (-N) = %d, jitter0 (-j) = %g, jitter (-J) = %g, "
	    "VariationalBayes (-V) = %d, wordscale (-W) = %g, randseed (-S) = %d, "
	    "debuglevel (-d) = %d, nruns (-R) = %d\n",
	    rule_bias_default, stoptol, minruleprob, maxlen, minits, maxits, annealstart,
	    annealstop, nanneal, jitter0, jitter, VariationalBayes, wordscale, randseed, debuglevel, nruns);

  g0 = read_grammar(grammarfp, si, rule_bias_default);
  set_rule_weights(g0, g0->weights, VariationalBayes);      /* normalize rule counts */

  signal(SIGINT, write_grammar_);

  for (irun = 0; irun < nruns; ++irun) {
    FLOAT entropy;

    g = copy_grammar(g0, si);

    if (summaryfp && debuglevel >= 100)
      fprintf(summaryfp, "# Starting run %d\n", irun);
    
    g_global = g;
    si_global = si;
    
    if (jitter0 > 0)
      jitter_weights(g, jitter0);
    
    entropy = inside_outside(g, si, yieldfp, tracefp, summaryfp, debuglevel, maxlen,
			     minits, maxits, stoptol, minruleprob, jitter, VariationalBayes, wordscale,
			     annealstart, annealstop, nanneal, weighted_yields_flag); 

    if (summaryfp && debuglevel >= 0)
      fprintf(summaryfp, "# Run %d, entropy %g, %ld rules\n", irun, entropy, (long) g->nrules);

    if (debuglevel >= 1 && (debuglevel < 5000 || debuglevel >= 10000)) {
      write_grammar(stdout, g, si, minruleprob);
      fprintf(stdout, "\n");
      fflush(stdout);
    }

    free_grammar(g);
  }
  
  free_grammar(g0);
  si_free(si);
    
  if (mmm_blocks_allocated) 
    fprintf(stderr, "Error in mrf(): %ld memory block(s) not deallocated\n", 
	    mmm_blocks_allocated);
  
  /* check that everything has been deallocated */
  assert(mmm_blocks_allocated == 0);		
  exit(EXIT_SUCCESS);
}
