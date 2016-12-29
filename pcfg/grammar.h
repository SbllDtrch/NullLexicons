/* grammar.h
 *
 * last modified (c) Mark Johnson, 30th April 2006
 */

#ifndef GRAMMAR_H
#define GRAMMAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "local-trees.h"
#include "hash.h"
#include "hash-string.h"
#include "vindex.h"

typedef struct struct_rulelist {
  size_t                 ruleid;
  struct struct_rulelist *next;
} *rulelist;

typedef struct struct_brule {
  si_index            left;
  si_index            active_parent;
  rulelist            completes;
  struct struct_brule *next;
} *brule;

typedef struct struct_catproblist {
  si_index                  cat;
  FLOAT                     prob;
  struct struct_catproblist *next;
} *catproblist;

typedef struct struct_grammar {
  size_t        nrules;        /* number of rules in the grammar */
  size_t        nnts;          /* number of non-terminal categories */
  size_t        ncats;         /* number of nonbinarized categories */
  vindex	*rules;        /* array of rules */
  FLOAT		*weights;      /* weight array */
  FLOAT         *bias;         /* pseudo-count bias */
  rulelist      *urules;       /* array mapping child to list of unary rules */
  brule		*brules;       /* array mapping right child to list of binary rules */
  /* FLOAT		**unary_close; */ /* unary closure matrix */
  catproblist	*child_parentprob; /* vector mapping child to unary rule parents */
  catproblist   *parent_childprob; /* vector mapping parent to unary rule children */
  si_index      root_label;    /* start category */
} *grammar;

si_index read_cat(FILE *fp, si_t si);
grammar read_grammar(FILE *fp, si_t si, FLOAT rule_bias_default);
grammar copy_grammar(const grammar g0, si_t si);
void write_rule_values(FILE *fp, grammar g, si_t si, FLOAT *values, FLOAT threshold);
void write_grammar(FILE *fp, grammar g, si_t si, FLOAT threshold);
void write_prolog_format_grammar(FILE *fp, grammar g, si_t si, FLOAT threshold);
void dump_grammar(FILE *fp, grammar g, si_t si);
void free_grammar(grammar g);
void compute_unary_closure(const grammar g, FLOAT minruleprob);
void free_unary_closure(const grammar g);
grammar prune_grammar(const grammar g, const si_t si, FLOAT minruleprob);
void jitter_weights(grammar g, FLOAT jitter);

#ifdef __cplusplus
};
#endif

#endif
