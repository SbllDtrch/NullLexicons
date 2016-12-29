/* hash-string.h
 *
 * Written by Mark Johnson, 25th May 1997;
 * modified so that si_index is of type index.
 */

#ifndef HASH_STRING
#define HASH_STRING

#include <stdio.h>
#include "hash.h"
#include "local-trees.h"

size_t strhash(const char *s);
char *mystrsave(const char *s);

/*
HASH_HEADER(strhashd, char *, double);
HASH_HEADER(strhashl, char *, long);
HASH_HEADER(strhashstr, char *, char *);
*/

/***************************************************************************
 *                                                                         *
 *                       string index stuff                                *
 *                                                                         *
 ***************************************************************************/

/* si_index is defined in local-trees.h
 */

HASH_HEADER(strhashsi, char *, si_index)

typedef struct si_table {       /* string_index table */
  strhashsi ht;          /* hash table */
  char      **strings;   /* array of strings */
  size_t    stringsize;  /* size of string table */
} *si_t;

/* make_si:  make a new si table */
si_t make_si(const size_t initial_size);        /* estimated size */
                          
/* returns the index associated with string 
 *  (a new number will be allocated if necessary) */
si_index si_string_index(si_t si, char *string);

/* returns the string associated with number; NULL if none exist */
char *si_index_string(const si_t si, const si_index index);

/* returns the number of strings indexed in si */
/* size_t si_nstrings(const si_t si); */
#define si_nstrings(si)                 (si)->ht->size

/* frees the memory used by si */
void si_free(si_t si);

/* displays the si to a file */
void si_display(si_t si, FILE *fp);

#endif



