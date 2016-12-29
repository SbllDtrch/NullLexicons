/* tree.c
 *
 *  Written by Mark Johnson, 27th May 1997
 */

#include "tree.h"

#include <assert.h>
#include <string.h>
#include <ctype.h>

#ifndef NDEBUG
size_t trees_allocated = 0;
size_t bintrees_allocated = 0;
#endif

tree
make_tree(si_index label, tree subtrees, tree sibling)
{
  tree t = NEW_TREE;
  t->label = label;
  t->subtrees = subtrees;
  t->sibling = sibling;
  return t;
}

void 
skipspaces(FILE *fp)
{
  int c;

  do c = getc(fp); while(isspace(c)); 	/* skip spaces */
  ungetc(c, fp);			/* push back non-space */
}

void 
skiplabel(FILE *fp)
{
  int c;

  do { c = getc(fp); assert(c!=EOF); }
  while (!isspace(c) && (c!='(') && (c!=')'));

  ungetc(c, fp);			/* push back final char */
}

si_index 
readlabel(FILE *fp, si_t si)
{
  int	 c, n=0, i;
  char	 s[MAXLABELLEN];

  while (((c=getc(fp))!=EOF) && !isspace(c) && (c!='(') && (c!=')') ) {
    s[n++] = c;
    assert(n<MAXLABELLEN-1);		/* leave space for '\0' */
  }  
  
  assert(n>0);				/* string should be non-empty */
  ungetc(c, fp);
  skipspaces(fp);
  s[n] = '\0';

  for (i=1; i<n-1; i++)			/* don't look at 1st or last char */
    if (strchr(CATSEP,s[i])) {		/* if s[i] is in CATSEP */
      s[i] = '\0';			/*  blast it away       */
      break;
    }

  return si_string_index(si, s);
}

/* readtree assumes that fp is pointing at first nonspace char */

tree 
readtree(FILE *fp, si_t si)
{
  int 	c;
  tree  t, p;

  c = getc(fp);
  assert(c!=EOF);		/* file ended w/out closing ')' */

  switch (c) {

  case ')': 
    skipspaces(fp);
    return(NULL); break;	/* empty tree */

  case '(':			/* nonterminal */
    skipspaces(fp);
    t = NEW_TREE;
    t->label = readlabel(fp, si);
    
    t->subtrees = p = readtree(fp, si); 
    while (p) {
      p->sibling = readtree(fp, si);
      p = p->sibling;
    }
    return(t); break;
  
  default:			/* skip terminal */
    skiplabel(fp);
    skipspaces(fp);
    c = getc(fp);		/* closing bracket */
    assert(c==')');
    skipspaces(fp);
    return(NULL);
  }}

/* readtree_root reads a tree from the treebank.  It inserts
 * a ROOT label that is not present in the treebank trees
 */

tree 
readtree_root(FILE *fp, si_t si)
{
  char c;
  skipspaces(fp);
  
  c = getc(fp);
  
  if (c == EOF) 
    return NULL;
  else { 
    tree p, t = NEW_TREE;
    assert(c == '(');
    skipspaces(fp);
    t->label = 1;  /* string id of root label */
    t->sibling = NULL;
    t->subtrees = p = readtree(fp, si);
    assert(t->subtrees);		/* tree should be nonempty */
    while (p) {
      p->sibling = readtree(fp, si);
      p = p->sibling;
    }
    return t;
  }}

void 
write_tree(FILE *fp, const tree t, si_t si)
{
  if (t->subtrees) {
    tree p;

    fprintf(fp, "(%s", si_index_string(si, t->label));
    
    for (p = t->subtrees; p; p = p->sibling) {
      fprintf(fp, " ");
      write_tree(fp, p, si);
    }

    fprintf(fp, ")");
  }
  else 
    fprintf(fp, "%s", si_index_string(si, t->label));
}

static void 
write_prolog_tree_(FILE *fp, const tree t, si_t si)
{
  char *s = si_index_string(si, t->label);
  
  fprintf(fp, "%s", s);
  if (t->subtrees) {
    tree p;
    
    if (s[0] != '\'')
      fprintf(fp, " ");
    fprintf(fp, "/[");
    
    for (p = t->subtrees; p; p = p->sibling) {
      write_prolog_tree_(fp, p, si);
      
      if (p->sibling)
	fprintf(fp, ",");
    }

    fprintf(fp, "]");
  }
}

void 
write_prolog_tree(FILE *fp, const tree t, si_t si)
{
  write_prolog_tree_(fp, t, si);
  /* fprintf(fp, ".\n"); */
}

void 
display_tree(FILE *fp, const tree t, si_t si, int indent)
{
  int  i;
  char *label = si_index_string(si, t->label);

  if (t->subtrees) {
    tree p;

    fprintf(fp, "(%s ", label);
    indent += strlen(label) + 2; 	/* for '(' + space */
    display_tree(fp, p = t->subtrees, si, indent);

    while ((p = p->sibling)) {
      printf("\n");
      for (i=0; i<indent; i++)
	fprintf(fp, " ");
      display_tree(fp, p, si, indent);
    }
    fprintf(fp, ")");
  }
  else {
    fprintf(fp, "%s", label);
  }}

void 
free_tree(tree t)
{
  if (t) {
    free_tree(t->sibling);
    free_tree(t->subtrees);
    FREE_TREE(t);
  }}

void 
write_bintree(FILE *fp, const bintree t, si_t si)
{
  if (t->left) {
    fprintf(fp, "(%s ", si_index_string(si, t->label));
    write_bintree(fp, t->left, si);
    if (t->right) {
      fprintf(fp, " ");
      write_bintree(fp, t->right, si);
    }
    fprintf(fp, ")");
  }
  else 
    fprintf(fp, "%s", si_index_string(si, t->label));
}

void 
display_bintree(FILE *fp, const bintree t, si_t si, int indent)
{
  int  i;
  char *label = si_index_string(si, t->label);

  if (t->left) {
    fprintf(fp, "(%s ", label);
    indent += strlen(label) + 2; 	/* for '(' + space */
    display_bintree(fp, t->left, si, indent);

    if (t->right) {
      printf("\n");
      for (i=0; i<indent; i++)
	fprintf(fp, " ");
      display_bintree(fp, t->right, si, indent);
    }
    fprintf(fp, ")");
  }
  else {
    fprintf(fp, "%s", label);
  }}

void
free_bintree(bintree t)
{
  if (t) {
    free_bintree(t->left);
    free_bintree(t->right);
    FREE_BINTREE(t);
  }}

tree 
collapse_identical_unary(const tree t)
{
  if (!t) 
    return NULL;
  else {
    tree p, c = NEW_TREE;
    c->label = t->label;
    c->sibling = collapse_identical_unary(t->sibling);
    p = t->subtrees;
    while (p && !p->sibling && p->label == t->label)
      p = p->subtrees;
    c->subtrees = collapse_identical_unary(p);
    return c;
  }}


static void right_binarize_label(char *s, const tree t, si_t si); 
	/* saves label for new binary node in *s */

static bintree right_binarize_helper(const tree t0, si_t si);
	/* binarizes non-left-edge nodes */

/* right_binarize_label loads *s with the concatenation of the labels
 * of t and its siblings 
 */

static void 
right_binarize_label(char *s, const tree t, si_t si)
{
  tree p;
  int  i = 0;
  char *l;

  for (l=si_index_string(si,t->label); *l; l++) {	/* copy first label */
    assert(i<MAXBLABELLEN); 
    s[i++] = *l;
  }

  for (p=t->sibling; p; p=p->sibling) {
    assert(i<MAXBLABELLEN);
    s[i++] = BINSEP;					/* copy category separator */

    for (l=si_index_string(si,p->label); *l; l++) {	/* copy next label */
      assert(i<MAXBLABELLEN); 
      s[i++] = *l;
    }}

  assert(i<MAXBLABELLEN);
  s[i++] = '\0';
}

/* right_binarize_helper builds a new bintree node corresponding to t0,
 * which is assumed to be a node not on the left edge of its local tree
 */

static bintree 
right_binarize_helper(const tree t0, si_t si)
{
  bintree t;

  if (t0->sibling) {
    char s[MAXBLABELLEN];
    t = NEW_BINTREE;
    right_binarize_label(s, t0, si);
    t->label = si_string_index(si, s);
    t->left = right_binarize(t0, si);
    t->right = right_binarize_helper(t0->sibling, si);
  }
  else
    t = right_binarize(t0, si);

  return t;
}
    
bintree 
right_binarize(const tree t0, si_t si)
{
  bintree t = NEW_BINTREE;

  t->label = t0->label;

  if (t0->subtrees) {
    t->left = right_binarize(t0->subtrees, si);
    t->right = t0->subtrees->sibling ? 
      right_binarize_helper(t0->subtrees->sibling, si) : NULL;
  }
  else {
    t->left = NULL;
    t->right = NULL;
  }

  return t;
}

static int
append_string(char s[], int i, const char *s0)
{
  while (*s0) {
    assert(i<MAXBLABELLEN-1);	/* leave space for '\0' */
    s[i++] = *s0++;
  }
  s[i] = '\0';
  return i;
}
  

/* right_binarize_helper builds a new bintree node corresponding to t0,
 * which is assumed to be a node not on the left edge of its local tree
 */

static bintree 
td_right_binarize_helper(const tree t0, si_t si, char *s, int i)
{
  bintree t;

  if (t0->sibling) {
    t = NEW_BINTREE;
    t->label = si_string_index(si, s);
    t->left = td_right_binarize(t0, si);
    s[i++] = BINSEP;
    i = append_string(s, i, si_index_string(si, t0->label));
    t->right = td_right_binarize_helper(t0->sibling, si, s, i);
  }
  else
    t = td_right_binarize(t0, si);

  return t;
}
    
bintree 
td_right_binarize(const tree t0, si_t si)
{
  bintree t = NEW_BINTREE;

  t->label = t0->label;

  if (t0->subtrees) {
    t->left = td_right_binarize(t0->subtrees, si);
    
    if (t0->subtrees->sibling) {
      char s[MAXBLABELLEN];
      int  i = append_string(s, 0, si_index_string(si, t0->label));
      s[i++] = BINSEP;
      i = append_string(s, i, si_index_string(si, t0->subtrees->label));
      t->right = td_right_binarize_helper(t0->subtrees->sibling, si, s, i);
    }
    else
      t->right = NULL;
  }
  else {
    t->left = NULL;
    t->right = NULL;
  }

  return t;
}

static tree
bintree_tree_(bintree bt, si_t si, tree sibling)
{
  if (!bt) {
    assert(!sibling);		/* should be no sibling to right of empty (left) node */
    return NULL;
  }

  if (strchr(si_index_string(si, bt->label), BINSEP))
    return bintree_tree_(bt->left, si, bintree_tree_(bt->right, si, sibling));
  else {
    tree t = bintree_tree(bt, si);
    t->sibling = sibling;
    return t;
  }}
    

tree
bintree_tree(bintree bt, si_t si)
{
  tree t = NEW_TREE;
  
  t->label = bt->label;
  t->sibling = NULL;
  t->subtrees = bintree_tree_(bt->left, si, bintree_tree_(bt->right, si, NULL));
  return t;
}

static size_t 
tree_terms_(tree t, vindex ts, size_t i) 
{
  for ( ; t; t=t->sibling)
    if (t->subtrees)
      i = tree_terms_(t->subtrees, ts, i);
    else {
      if (i >= ts->n) {
	ts->n *= 2;
	vindex_resize(ts, ts->n);
      }
      assert(ts->n > i);
      ts->e[i++] = t->label;
    }
  return i;
}

struct struct_vindex 
tree_terms(tree t)
{
  struct struct_vindex ts;

  ts.n = 8;
  ts.e = MALLOC(ts.n*sizeof(*ts.e));

  ts.n = tree_terms_(t, &ts, 0);
  ts.e = REALLOC(ts.e, ts.n*sizeof(*ts.e));

  return ts;
}
