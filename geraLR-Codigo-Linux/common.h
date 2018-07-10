/*
*---------------------------------------------------------------------
*
*   File         : common.h
*   Created      : 2012-05-28
*   Last Modified: 2012-05-28 (ROUND and ABS modifiqued 2018-03-30)
*
*   Commonly used definitions -- only typedefs and defines BUT
*   ABSOLUTELY NO FUNCTION PROTOTYPES OR FUNCTION CALLS
*
*---------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _COMMON_DOT_H_
#define _COMMON_DOT_H_

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <limits.h>
#include <float.h>
#include <stdbool.h>

/*
*---------------------------------------------------------------------
*   Definition of constants and macros
*---------------------------------------------------------------------
*/

#ifndef ABS
#define ABS(x)         ((x)<0 ? -(x) : (x))
#endif

#ifndef ROUND
//#define ROUND(x)      ((int) ((x) + 0.5))
#define ROUND(x)        ((x>=0)?(int)(x + .5):(int)(x - .5))
#endif

#define SIGN(x)        (((x)<0)?-1:(((x)==0)?0:1))
#define GREATEST(x,y)  ((x)>(y)? (x):(y))
#define SMALLEST(x,y)  ((x)<(y)? (x):(y))

/*                                                  */
/* MINIMUM and MAXIMUM values of numeric data types */
/*                                                  */

#define t_byte          unsigned char

/* Signed integer types */

#define min_CHAR        SCHAR_MIN
#define min_SHORT       SHRT_MIN
#define min_INT         INT_MIN
#define min_LONG        LONG_MIN

#define max_CHAR        SCHAR_MAX
#define max_SHORT       SHRT_MAX
#define max_INT         INT_MAX
#define max_LONG        LONG_MAX

/* Unigned integer types */

#define min_UCHAR       0
#define min_BYTE        0
#define min_USHORT      0
#define min_UINT        0
#define min_ULONG       0

#define max_UCHAR       UCHAR_MAX
#define max_BYTE        UCHAR_MAX
#define max_USHORT      USHRT_MAX
#define max_UINT        UINT_MAX
#define max_ULONG       ULONG_MAX

/* Floating point types  */

#define min_FLOAT       ((float)  FLT_MIN)
#define min_DOUBLE      ((double) DBL_MIN)
#define min_LONG_DOUBLE ((double) LDBL_MIN)
#define max_FLOAT       ((float)  FLT_MAX)
#define max_DOUBLE      ((double) DBL_MAX)
#define max_LONG_DOUBLE ((double) LDBL_MAX)

/*          */
/* OTHER CONSTANTS */
/*                 */

#endif /* ifndef _COMMON_DOT_H_ */
