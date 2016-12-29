/* hash.h
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "mmm.h"

/* 
 * typedef size_t 	(*KEY_HASH)(const HASH_KEY);
 * typedef int    	(*KEY_NEQ)(const HASH_KEY, const HASH_KEY);
 * typedef HASH_KEY	(*KEY_COPY)(const HASH_KEY);
 * typedef void		(*KEY_FREE)(HASH_KEY);
 * typedef void		(*VALUE_FREE)(HASH_VALUE);
 * typedef HASH_VALUE	NULL_VALUE
 */

#define HASH_HEADER(HASH, HASH_KEY, HASH_VALUE)	\
						\
typedef struct HASH ## _cell {	\
  HASH_KEY	   key;		\
  size_t	   hashedkey;	\
  struct HASH ## _cell *next;	\
  HASH_VALUE	   value;	\
} * HASH ## _cell_ptr;		\
				\
typedef struct HASH ## _table {	/* hash table of pointers */ 	\
  HASH ## _cell_ptr *table;					\
  size_t 	    tablesize;	/* size of table */		\
  size_t 	    size; 	/* no of elts in hash table */	\
} * HASH;							\
	\
HASH make_ ## HASH(size_t initial_size);	\
HASH_VALUE HASH ## _ref(const HASH ht, const HASH_KEY key);	\
HASH_VALUE HASH ## _set(HASH ht, HASH_KEY key, HASH_VALUE value);	\
HASH_VALUE HASH ## _delete(HASH ht, HASH_KEY key);	\
void free_ ## HASH(HASH ht);	\
size_t HASH ## _size(HASH ht);	\
HASH ## _cell_ptr HASH ## _find(HASH ht, const HASH_KEY key);	\
HASH_VALUE *HASH ## _valuep(HASH ht, const HASH_KEY key);      	\
	\
	\
typedef struct HASH ## it { 	\
  HASH_KEY          key;	\
  HASH_VALUE        value;	\
  HASH		    ht;		\
  HASH ## _cell_ptr next;	\
  size_t            index;	\
} HASH ## it;			\
				\
HASH ## it HASH ## it_init(HASH ht);		\
HASH ## it HASH ## it_next(HASH ## it hit);	\
int HASH ## it_ok(HASH ## it hit);  

#define HASH_HEADER_ADD(HASH, HASH_KEY, HASH_VALUE)	\
							\
HASH_HEADER(HASH, HASH_KEY, HASH_VALUE)			\
HASH_VALUE HASH ## _inc(HASH ht, const HASH_KEY key, HASH_VALUE inc);
