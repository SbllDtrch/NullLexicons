/* mmm.h
 *
 * Mark's memory manager
 *
 * Defines CALLOC, MALLOC, REALLOC and FREE, which are just like the
 * standard versions, except that these do varying amounts
 * of sanity checking.
 */

#include <stdlib.h>

extern	long	mmm_blocks_allocated;

#define CALLOC(n,m)	mmm_calloc(n,m)
#define MALLOC(n)	mmm_malloc(n)
#define REALLOC(x,n)	mmm_realloc(x,n)
#define FREE(x)		mmm_free(x)

void *mmm_calloc(size_t count, size_t size);
void *mmm_malloc(size_t n);
void *mmm_realloc(void *ptr, size_t size);
void mmm_free(void *ptr);
