/* hash-templates.h
 *
 * Hash table code templates.
 *
 * This file defines two macros, which in turn define hash functions
 * of arbitrary objects.
 *
 * HASH_CODE(HASH, HASH_KEY, HASH_VALUE, KEY_HASH, KEY_NEQ, KEY_COPY,  \
 *		  KEY_FREE, NULL_VALUE, VALUE_FREE)
 *   defines a hash package whose function names have the prefix HASH,
 *   and where
 *       HASH_KEY is the type of hash keys
 *       HASH_VALUE is the type of hash values
 *       KEY_HASH is a hash function for the keys
 *       KEY_NEQ returns 0 iff its pair of key arguments are equal
 *       KEY_COPY returns a copy of a key
 *       KEY_FREE frees a key
 *       NULL_VALUE is value returned for an undefined key
 *       VALUE_FREE frees a value
 *
 * HASH_CODE_ADD has the same arguments as HASH_CODE, but defines
 * functions that assume that values can be added in addition to
 * those of HASH_CODE
 *
 * These functions can be `expanded' by macros if desired.
 * IDENTITY and NO_OP are provided as possible expansions.
 */
 
#include <assert.h>
#include "mmm.h"

/*
#define MALLOC		malloc
#define CALLOC	 	calloc
#define REALLOC 	realloc
#define FREE		free
*/

/* Preferred hash sizes - primes not close to a power of 2
 * size_t preferred_sizes[] = {7 43 277 1663 10007 60077 360497 2247673 13486051};
 */

#define preferred_size(size)		\
	(size < 21) ? 7 :		\
	(size < 129) ? 43 :		\
	(size < 831) ? 277 :		\
	(size < 4989) ? 1663 :		\
	(size < 30021) ? 10007 :	\
	(size < 180231) ? 60077 :	\
	(size < 1081491) ? 360497 :	\
	(size < 6743019) ? 2247673 :    \
	13486051

#define IDENTITY(x)	x
#define NO_OP(x)	
#define NEQ(x,y)	((x)!=(y))

#define HASH_EXPAND_LOAD_FACTOR 3
#define HASH_CONTRACT_LOAD_FACTOR 5

#define HASH_CODE(HASH, HASH_KEY, HASH_VALUE, KEY_HASH, KEY_NEQ, KEY_COPY,  \
		  KEY_FREE, NULL_VALUE, VALUE_FREE)			    \
	\
/* HASH_HEADER(HASH, HASH_KEY, HASH_VALUE); */ \
	\
HASH make_ ## HASH(size_t initial_size)	\
{	\
  HASH ht;	\
  ht = (HASH) MALLOC(sizeof(struct HASH ## _table));	\
  ht->tablesize = preferred_size(initial_size);	\
  ht->size = 0; 	\
  ht->table = (HASH ## _cell_ptr *)	\
    CALLOC(ht->tablesize, (size_t) sizeof(HASH ## _cell_ptr));	\
  return ht;	\
}   	\
        \
size_t HASH ## _size(HASH ht) \
{                             \
  return ht->size;            \
}                             \
	\
  	\
static void HASH ## _resize(HASH ht)	\
{	\
  HASH ## _cell_ptr *oldtable = ht->table;	\
  size_t oldtablesize = ht->tablesize;	\
  size_t tablesize = preferred_size(ht->size);	\
  size_t i;	\
  HASH ## _cell_ptr	p, nextp;	\
  	\
  ht->tablesize = tablesize;	\
  ht->table = (HASH ## _cell_ptr *) 	\
    CALLOC(tablesize, (size_t) sizeof(HASH ## _cell_ptr));	\
  	\
  for (i=0; i<oldtablesize; i++)	\
    for (p = oldtable[i]; p; p = nextp) {	\
      nextp = p->next;	\
      p->next = ht->table[p->hashedkey%tablesize];	\
      ht->table[p->hashedkey%tablesize] = p;	\
    }	\
  	\
  FREE(oldtable);	\
}	\
	\
HASH ## _cell_ptr HASH ## _find(HASH ht, const HASH_KEY key) 	\
{ 	\
  size_t hashedkey = KEY_HASH(key);	\
  size_t hashedkeymod = hashedkey%ht->tablesize;	\
  HASH ## _cell_ptr p = ht->table[hashedkeymod];	\
  	\
  while (p && (p->hashedkey != hashedkey || KEY_NEQ(key, p->key)))	\
    p = p->next;	\
	\
  if (p) 	\
    return p;	\
  else {	\
    if (ht->size++ >= HASH_EXPAND_LOAD_FACTOR*ht->tablesize) {	\
      HASH ## _resize(ht);	\
      hashedkeymod = hashedkey%ht->tablesize;	\
    }	\
    p = MALLOC(sizeof(struct HASH ## _cell));	\
    p->hashedkey = hashedkey;	\
    p->key = KEY_COPY(key);	\
    p->value = NULL_VALUE;	\
    p->next = ht->table[hashedkeymod];	\
    ht->table[hashedkeymod] = p;	\
    return p;			\
}}	\
  	\
HASH_VALUE *HASH ## _valuep(HASH ht, const HASH_KEY key)      	\
{	\
  return(&(HASH ## _find(ht, key)->value));	\
}	\
	\
HASH_VALUE HASH ## _ref(const HASH ht, const HASH_KEY key)	\
{	\
  size_t hashedkey = KEY_HASH(key);	\
  HASH ## _cell_ptr p = ht->table[hashedkey%ht->tablesize];	\
	\
  while (p && (p->hashedkey != hashedkey || KEY_NEQ(key, p->key)))	\
    p = p->next;	\
	\
  return p ? p->value : NULL_VALUE;	\
}	\
	\
HASH_VALUE HASH ## _set(HASH ht, HASH_KEY key, HASH_VALUE value)	\
{	\
  HASH ## _cell_ptr p = HASH ## _find(ht, key);	\
  HASH_VALUE oldvalue = p->value;		\
  p->value = value;				\
  return oldvalue;				\
}	\
	\
HASH_VALUE HASH ## _delete(HASH ht, HASH_KEY key)       \
{       \
  HASH_VALUE oldvalue;  \
  size_t hashedkey = KEY_HASH(key);     \
  HASH ## _cell_ptr     *p  = ht->table + hashedkey%ht->tablesize;      \
        \
  while ( *p &&         \
          ((*p)->hashedkey != hashedkey || KEY_NEQ(key, (*p)->key)) )  \
    p = &((*p)->next); \
        \
  if (*p) {     \
    oldvalue = (*p)->value;     \
    *p = (*p)->next;    \
    KEY_FREE((*p)->key);        \
    FREE(*p);   \
        \
    if (--ht->size < ht->tablesize/HASH_CONTRACT_LOAD_FACTOR)   \
      HASH ## _resize(ht);      \
    return oldvalue;		\
  }			\
  else		       \
    return NULL_VALUE; \
}		       \
  		       \
  						\
void free_ ## HASH(HASH ht)			\
{						\
  HASH ## _cell_ptr p, q;			\
  size_t  i;					\
  						\
  for (i = 0; i < ht->tablesize; i++)					\
    for (p = ht->table[i]; p; p = q) {					\
      q = p->next;							\
      KEY_FREE(p->key);							\
      VALUE_FREE(p->value);						\
      FREE(p);								\
    }									\
  FREE(ht->table);							\
  FREE(ht);								\
}									\
  									\
/*************************************************************************** \
 *                                                                         * \
 *                           hash iteration                                * \
 *                                                                         * \
 ***************************************************************************/ \
  									\
  									\
HASH ## it HASH ## it_init(HASH ht)					\
{									\
  struct HASH ## it hit = {0, NULL_VALUE, ht, NULL, 0};                 \
  return HASH ## it_next(hit);						\
}									\
									\
HASH ## it HASH ## it_next(HASH ## it hit0)				\
{									\
  if (hit0.next) {							\
    hit0.key = hit0.next->key;						\
    hit0.value = hit0.next->value;					\
    hit0.next = hit0.next->next;					\
    return hit0;							\
  }									\
  else {								\
    size_t i = hit0.index;						\
    size_t tablesize = hit0.ht->tablesize;				\
    HASH ## _cell_ptr *table = hit0.ht->table;				\
    while (i < tablesize && !table[i])					\
      i++;								\
    if (i==tablesize) {							\
      hit0.ht = NULL;							\
      return hit0;							\
    }									\
    else {								\
      hit0.key = table[i]->key;						\
      hit0.value = table[i]->value;					\
      hit0.next = table[i]->next;					\
      hit0.index = i+1;							\
      return hit0;							\
    }}}									\
  									\
int HASH ## it_ok(HASH ## it hit)					\
{									\
  return hit.ht!=NULL;							\
} 

#define HASH_CODE_ADD(HASH, HASH_KEY, HASH_VALUE, KEY_HASH, KEY_NEQ,    \
		      KEY_COPY, KEY_FREE, NULL_VALUE, VALUE_FREE)       \
								        \
HASH_CODE(HASH, HASH_KEY, HASH_VALUE, KEY_HASH, KEY_NEQ,                \
		KEY_COPY, KEY_FREE, NULL_VALUE, VALUE_FREE)             \
                                                                        \
HASH_VALUE HASH ## _inc(HASH ht, const HASH_KEY key, HASH_VALUE inc)	\
{	                                                                \
/*  size_t hashedkey = KEY_HASH(key); */	\
  HASH ## _cell_ptr p = HASH ## _find(ht, key);	\
  	\
  p->value += inc;	\
  return p->value;	\
} 
