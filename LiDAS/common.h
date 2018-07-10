/*
*---------------------------------------------------------------------
*
*   File         : common.h
*   Created      : 2012-05-28
*   Last Modified: 2012-05-28
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

/*
*---------------------------------------------------------------------
*   Definition of constants and macros
*---------------------------------------------------------------------
*/

#define ABS(x)         ((x)<0 ? -(x) : (x))
#define ROUND(x)       ((int) ((x) + 0.5))
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
/* BOOLEANS */
/*          */

#ifndef Bool
# define Bool   char
# define FALSE  0
# define TRUE   1
#endif

/*                 */
/* OTHER CONSTANTS */
/*                 */

#define float_ZERO       ((float)  0)
#define float_ONE        ((float)  1)
#define double_ZERO      ((double) 0)
#define double_ONE       ((double) 1)
#define long_double_ZERO ((long double) 0)
#define long_double_ONE  ((long double) 1)

#endif /* ifndef _COMMON_DOT_H_ */
