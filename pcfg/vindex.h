/* vindex.h  -- Vectors of si_index
 *
 * Mark Johnson, 22nd May 1997
 */

#include "mmm.h"
#include "local-trees.h"
#include "hash.h"

#ifndef VINDEX_H
#define VINDEX_H

typedef struct struct_vindex {
  size_t    n; 		/* size of vector */
  si_index  *e;		/* vector of elements */
} *vindex;

vindex make_vindex(const size_t n);
void vindex_free(vindex v);

#define vindex_ref(vindex, i)		vindex->e[i]

#define vindex_resize(vindex, n) 	vindex->e = REALLOC(vindex->e, n*sizeof(*vindex->e))


size_t vindex_hash(const vindex v);
int vindex_cmp(const vindex v1, const vindex v2);
vindex vindex_copy(const vindex v);
void vindex_free(vindex v);


HASH_HEADER_ADD(vihashst, vindex, size_t)

#endif
