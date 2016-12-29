/* tree.h
 *
 * Written by Mark Johnson, 27th May 1997
 */

#ifndef TREE_H
#define TREE_H

#include "local-trees.h"
#include "hash-string.h"
#include "vindex.h"
#include <stdio.h>

#ifndef NDEBUG
extern size_t trees_allocated;
extern size_t bintrees_allocated;
#endif

typedef struct tree {
  si_index     label;
  struct tree  *subtrees;
  struct tree  *sibling;
} *tree;

typedef struct bintree {
  si_index	label;
  struct bintree	*left;
  struct bintree	*right;
} *bintree;

#ifdef NDEBUG
#define NEW_TREE	(tree) MALLOC(sizeof(struct tree))
#define FREE_TREE(t)	FREE(t)
#define NEW_BINTREE	(bintree) MALLOC(sizeof(struct bintree))
#define FREE_BINTREE(t)	FREE(t)
#else
#define NEW_TREE	(trees_allocated++, (tree) MALLOC(sizeof(struct tree)))
#define FREE_TREE(t)	{assert(trees_allocated--); FREE(t); }
#define NEW_BINTREE	(bintrees_allocated++, (bintree) MALLOC(sizeof(struct bintree)))
#define FREE_BINTREE(t)	{assert(bintrees_allocated--); FREE(t); }
#endif

void skipspaces(FILE *fp);		/* skips spaces in fp */
void skiplabel(FILE *fp);		/* skips (remainder of) label in fp */
si_index readlabel(FILE *fp, si_t si);	/* returns index of next label */
tree readtree(FILE *fp, si_t si);	/* reads & returns next tree from fp */
tree readtree_root(FILE *fp, si_t si); 	/* like readtree, but hallucinates a root label */

void write_tree(FILE *fp, const tree t, si_t si);	/* writes tree t onto stdout */
void write_prolog_tree(FILE *fp, const tree t, si_t si);	/* writes tree t onto stdout in Prolog format */
void display_tree(FILE *fp, const tree t, si_t si, int indent);	/* prettyprints tree t to stdout */
void free_tree(tree t);			/* frees the tree t */

void write_bintree(FILE *fp, const bintree t, si_t si);	/* writes tree t onto stdout */
void display_bintree(FILE *fp, const bintree t, si_t si, int indent);	/* prettyprints tree t to stdout */
void free_bintree(bintree t);		/* frees the binary tree t */

tree collapse_identical_unary(const tree t);	/* collapses non-branching unary chains */

bintree right_binarize(const tree t0, si_t si);	/* returns a right-binarized tree */
tree bintree_tree(const bintree t, const si_t si);	/* returns the original tree for t */

bintree td_right_binarize(const tree t0, si_t si);	/* returns a right-binarized tree for td parsing */

struct struct_vindex tree_terms(tree t);		/* returns a list of terminals */

#endif




