/* local-trees.h
 *
 * (c) Mark Johnson, 21st May 1997
 * modified (c) Mark Johnson, 12th July 2004
 *
 * Identifiers for local tree counts
 */

#ifndef LOCAL_TREES_H
#define LOCAL_TREES_H

#define REWRITES 	"-->"
#define CATSEP		"-=|"		/* separates categories */
#define BINSEP		'_'		/* joins new (binarized) categories */

typedef unsigned long 	si_index;	/* type of category indices */
#define SI_INDEX_MAX	ULONG_MAX

typedef double	FLOAT;			/* type of floating point calculations */

#define MAXRHS		63		/* max length of prodn rhs */
#define MAXRULELEN	(MAXRHS+1)	/* max rule length */
#define MAXLABELLEN	64		/* max length of a label */
#define MAXBLABELLEN	512		/* max length of a binarized label */ 
#define NLABELS 	64		/* guesstimate of no of category labels */
#define CHART_CELLS	128		/* initial size of chart cells */
#define COMPLETE_CELLS  20		/* initial size of parent completion table */

#endif
