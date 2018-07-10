/*
*---------------------------------------------------------------------
*
*   File         : mymath.h
*   Created      : 15.Sep.92
*   Last Modified: 14.Sep.95
*
*   Mathematical constants, data types and functions.
*
*---------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _MYMATH_DOT_H_
#define _MYMATH_DOT_H_

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <math.h>
#include <stdint.h>

#include "common.h"

/*
*---------------------------------------------------------------------
*   Definition of constants and macros
*---------------------------------------------------------------------
*/

#define my_ATAN(x,y)        (atan2 ((double)(x), (double)(y)))
#define SQR(x)              ((x)*(x))
#define SQRT(x)             (sqrt ((double)(x)))
#define iCATET(x,y)         (ROUND (SQRT(SQR(x)-SQR(y))))
#define dCATET(x,y)         ((double) (SQRT(SQR(x)-SQR(y))))
#define HIPOT(x,y)          (SQRT(SQR(x)+SQR(y)))
#define DIST(x1,y1,x2,y2)   (SQRT(SQR((x2)-(x1))+SQR((y2)-(y1))))

/*                  */
/* BIT MANIPULATION */
/*                  */

#define BITS_PER_BYTE  CHAR_BIT                      /* ...on this machine    */
#define UINT_BYTES     (sizeof (unsigned int))       /* Bytes in unsigned int */
#define UINT_BITS      (UINT_BYTES * BITS_PER_BYTE)  /* Bits  in unsigned int */

/* Bytes needed to store "b" bits */

#define BITS_2_BYTES(b)  ((int) (((b)-1)/BITS_PER_BYTE) + 1)

/* Offset of the bit in position [row][col] of a boolean array */

#define bit_OFFSET(totCols,row,col)  ((row)*(totCols) + (col))

/*          */
/* BOOLEANS */
/*          */

#define XOR(a,b)  (((a) && !(b)) || (!(a) && (b)))

/*                 */
/* OTHER CONSTANTS */
/*                 */

/*
#define my_zero             0.001        /  For floating point comparisons   /
#define nearly_ZERO         0.0001       /  For floating point comparisons   /
*/

#define half_pi             1.570796327  /* pi/2                            */
#define pi                  3.141592654  /* pi                              */
#define three_half_pi       4.712388980  /* 3*pi/2                          */
#define two_pi              6.283185307  /* 2*pi                            */
#define deg2rad             0.017453293  /* Radians = deg2rad * degrees     */
#define rad2deg            57.295779513  /* Degrees = rad2deg * radians     */
#define fullCircle      23040            /* 23040=360*64; used by XDrawArc  */

/* Size of pre-defined arrays (char, int, float & double) */

#define predef_array_SIZE  500

/*                                                 */
/* Uncomment the following line if you want to use */
/* the lookup tables for sin() & cos(). Otherwise  */
/* the standard library routines will be used.     */
/*
#define USE_TRIG_TABLES
*/

#ifdef USE_TRIG_TABLES
  #define tablePrecision      0.001      /* Precision of the angle theta    */
  #define tableFactor      1000          /* 1/tablePrecision                */
  #define tableSize        6284          /* (int) 1 + 2*pi*tableFactor      */
  #define tableName        "trig.tbl"    /* File containing the trig tables */
#else
  #define myCos(theta)  ((float) cos((double)(theta)))
  #define mySin(theta)  ((float) sin((double)(theta)))
#endif

/*
*---------------------------------------------------------------------
*   Type definitions
*---------------------------------------------------------------------
*/

/*                     */
/* Pre-defined arrays  */
/*                     */

typedef char
  t_predef_char_array  [predef_array_SIZE];

typedef int
  t_predef_int_array   [predef_array_SIZE];

typedef float
  t_predef_float_array [predef_array_SIZE];

typedef double
  t_predef_double_array[predef_array_SIZE];

/*                     */
/* Integer coordinates */
/*                     */

typedef struct {
  int x, y;
}
  t_iCoords;

/*                          */
/* Real coordinates (float) */
/*                          */

typedef struct {
  float x, y;
}
  t_fCoords;

/*                           */
/* Real coordinates (double) */
/*                           */

typedef struct {
  double x, y;
}
  t_dCoords;

/*                      */
/* Random distributions */
/*                      */

typedef enum {
  t_uniform_distrib,
  t_normal_distrib
}
  t_random_distrib;

/*                                 */
/* Seed for random number routines */
/*                                 */

extern long int
  random_seed;

/*                   */
/* Simple statistics */
/*                   */

typedef struct {
  unsigned int samples;
  double       min;
  double       max;
  double       acc;
  double       accSq;
  double       avg;
  double       dev;
}
  t_statRec;

/*
*---------------------------------------------------------------------
*   Variables
*---------------------------------------------------------------------
*/

/*                           */
/* Array of ordered integers */
/*                           */

extern t_predef_int_array
  my_ordered_ints;

/*
*---------------------------------------------------------------------
*   Procedures defined in <math.h>
*---------------------------------------------------------------------
*/

extern double atan2   (double, double);
extern double cos     (double);
extern double exp     (double);
extern double log     (double);
extern double log10   (double);
extern double sin     (double);
extern double sqrt    (double);

/*
*---------------------------------------------------------------------
*   Procedures defined in "mymath.c"
*---------------------------------------------------------------------
*/

/*                                                                                                */
/* Floating point number comparison is a VERY tricky business! The following code is adapted from */
/* https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/    */
/*                                                                                                */

/* Extraction of components */

extern int32_t float2mantissa (float f);
extern int32_t float2exponent (float f);

/* Comparison */

extern Bool floatAlmostZero                 (float A, float maxDiff);
extern Bool floatsAlmostEqualUlps           (float A, float B, int maxUlpsDiff);
extern Bool floatsAlmostEqualUlpsAndAbs     (float A, float B, float maxDiff, int maxUlpsDiff);
extern Bool floatsAlmostEqualRelativeAndAbs (float A, float B, float maxDiff, float maxRelDiff);

/*                                                          */
/* Bitwise operations                                       */
/*                                                          */
/* IMPORTANT: Bit offsets are calculated from right to left */
/* ie. bit 0 is the least significant bit of a given byte   */
/*     <------  MEMORY ADDRESSES GROW THIS WAY  <------     */
/*                                                          */

extern Bool get_bit  (t_byte *addr, int offset);
extern void set_bit  (t_byte *addr, int offset, int value);
extern void flip_bit (t_byte *addr, int offset);

extern size_t bitStr_diffs (
 t_byte *addr1,
 int     offset1,
 t_byte *addr2,
 int     offset2,
 int     blockSize,
 int     totalSize );

extern Bool bitStr_equal (
 t_byte *addr1,
 int     offset1,
 t_byte *addr2,
 int     offset2,
 int     size );

extern unsigned int bits2uint (
 t_byte *addr,
 int     offset,
 int     size);

extern void copy_bits (
 t_byte *origin_addr,
 int     origin_offset,
 t_byte *dest_addr,
 int     dest_offset,
 int     size );

/*                           */
/* Random numbers and events */
/*                           */

extern Bool     flip_coin (float prob);
extern float    gaussian (float mean, float std_dev);
extern long int my_randomize (Bool b_really_random);
extern void     random_array (t_byte *addr, int size);
extern char     random_char (char from, char to);
extern int      random_int (int from, int to);
extern float    random_float (float from, float to);
extern double   random_double (double from, double to);
extern int     *random_list_of_int (int from, int to);
extern void     restore_random_seed (unsigned int seed);
extern void     shuffle_array (t_byte *array, int elements, int bytes_each);
extern void     shuffle_array_of_int (int *array, int elements);

extern float   *random_distribution (
  int              total,
  float            lower,
  float            upper,
  t_random_distrib distrib_type );

/*                         */
/* Trigonometric functions */
/*                         */

#ifdef USE_TRIG_TABLES
  extern float myCos (float theta);
  extern float mySin (float theta);
  extern void  read_trig_tables (void);
#endif

/*                   */
/* Simple statistics */
/*                   */

extern void acc2avgDev (
  int     totSamples,
  double  acc,
  double  accSq,
  double *avg,
  double *stdDev );

/* Simple statistical records */

extern int  setup_statRec     (void);
extern void newData_statRec   (int index, double data);
extern void dummyData_statRec (int index, double data);
extern void clear_statRec     (int index);
extern void dismiss_statRec   (int index);
extern t_statRec read_statRec (int index);

/* Matrix statistical records */

extern int  setup_matStatRec        (int totRows, int totCols);
extern void newData_matStatRec      (int index, int row, int col, double data);
extern void dummyData_matStatRec    (int index, int row, int col, double data);
extern void clear_matStatRec        (int index);
extern void dismiss_matStatRec      (int index);
extern t_statRec read_matStatRec    (int index);
extern t_statRec read_matStatRecRow (int index, int row);
extern t_statRec read_matStatRecCol (int index, int col);

/*        */
/* Others */
/*        */

extern void  initialize_maths (void);
extern float normalize_angle  (float theta);
extern void  bytes2ints       (t_byte *bytes,  int    *ints,    int total);
extern void  floats2bytes     (float  *floats, t_byte *bytes,   int total);
extern void  floats2doubles   (float  *floats, double *doubles, int total);
extern void  floats2ints      (float  *floats, int    *ints,    int total);

#endif /* ifndef _MYMATH_DOT_H_ */
