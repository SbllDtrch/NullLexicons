/* expected-counts.h
 *
 * (c) Mark Johnson, September 1998, last hacked 12th May 2008
 */

#ifndef EXPECTED_COUNTS_H
#define EXPECTED_COUNTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "grammar.h"
#include "local-trees.h"
#include <stdlib.h>

FLOAT
expected_rule_counts(const grammar g, const si_t si, FILE *yieldfp, 
		     FILE *tracefp, FILE *summaryfp, int debuglevel, int maxsentlen, 
		     FLOAT minruleprob, FLOAT wordscale,
		     FLOAT *rule_counts, FLOAT* sum_yieldweights,
		     int weighted_yields_flag);

void
set_rule_weights(grammar g, FLOAT *rule_counts, int VariationalBayes);

FLOAT
inside_outside(grammar g, const si_t si, FILE *yieldfp, 
	       FILE *tracefp, FILE *summaryfp, int debuglevel,
	       int maxsentlen, int minits, int maxits,
	       FLOAT stoptol, FLOAT minruleprob,
	       FLOAT jitter, int VariationalBayes, FLOAT wordscale,
	       FLOAT annealstart, FLOAT annealstop, int nanneal,
	       int weighted_yields_flag);

#ifdef __cplusplus
};
#endif

#endif  /* EXPECTED_COUNTS_H */
