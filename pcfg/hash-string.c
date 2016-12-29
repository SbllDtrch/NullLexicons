/* hash-string.c
 */

#include "hash-string.h"
#include "hash-templates.h"
#include <assert.h>
#include <string.h>

/* strhash maps a character string to a hash index < 2^28
 * This is the fn hashpjw of Aho, Sethi and Ullman, p 436.
 */
 
size_t strhash(const char *s)
{
  const char *p;
  unsigned h = 0, g;
  for (p=s; *p != '\0'; p++) {
    h = (h << 4) + (*p);
    if ((g = h&0xf0000000)) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }}
  return h;
}

char *mystrsave(const char *s)    /* make a duplicate of s; see p 143 */
{
  char *p;
  
  p = (char *) MALLOC(strlen(s)+1);	/* +1 for '\0' */
  strcpy(p, s);
  return p;
}

/*
HASH_CODE_ADD(strhashl, char *, long, strhash, strcmp, mystrsave, FREE, 0, NO_OP)
HASH_CODE_ADD(strhashd, char *, double, strhash, strcmp, mystrsave, FREE, 0.0, NO_OP)
HASH_CODE(strhashstr, char *, char *, strhash, strcmp, mystrsave, FREE, NULL, FREE)
*/

/***************************************************************************
 *                                                                         *
 *                       string index stuff                                *
 *                                                                         *
 ***************************************************************************/

HASH_CODE(strhashsi, char *, si_index, strhash, strcmp, mystrsave, FREE, 0, NO_OP)

/* make_si:  make an si table */
si_t make_si(const size_t initial_size)
{
  size_t s;
  si_t si = MALLOC(sizeof(struct si_table));
  si->ht = make_strhashsi(initial_size);
  
  si->stringsize = 1;
  for (s=initial_size; s; s /= 2)
    si->stringsize *= 2;

  si->strings = (char **) MALLOC(si->stringsize*sizeof(char *));
  return si;
}

/* si_string_index: returns the number associated with string
 */
si_index si_string_index(si_t si, char *string)
{
  strhashsi_cell_ptr p = strhashsi_find(si->ht, string);

  if (p->value) return p->value;

  if (si->ht->size >= SI_INDEX_MAX) {
    fprintf(stderr, "si_string_index() in hash-string.c: index overflow\n");
    abort();
  }

  p->value = si->ht->size;

  if (si->ht->size > si->stringsize) {
    si->stringsize *= 2;
    si->strings = REALLOC(si->strings, si->stringsize*sizeof(char *));
  }

  si->strings[p->value-1] = p->key;
  return p->value;
}
 
char *si_index_string(const si_t si, const si_index index) 
{
  assert(index > 0);
  assert(index <= si->ht->size);
  return si->strings[index-1];
}

void si_free(si_t si)
{
  free_strhashsi(si->ht);
  FREE(si->strings);
  FREE(si);
}

void si_display(si_t si, FILE *fp)
{
  size_t i;

  for (i=1; i<=si_nstrings(si); i++) 
    fprintf(fp, "%ld: %s\n", (long) i, si_index_string(si, i));
}


 
