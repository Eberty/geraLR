/*
*---------------------------------------------------------------------
*
*   File         : mymath.c
*   Created      : 1992-09-15
*   Last Modified: 2016-06-11
*
*   Mathematical constants, data types and functions.
*   Includes routines to access tables of trigonometric functions.
*   The tables should speed up any calculations that call sin and cos.
*   The user should make sure that the tables have been loaded prior
*   to any call to mySin or myCos. This is done by calling
*       load_trig_tables().
*
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "error.h"
#include "random.h"
#include "mymath.h"

/*
*---------------------------------------------------------------------
*
*   INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

/*                                 */
/* Seed for random number routines */
/*                                 */

long int
  random_seed;

/*                           */
/* Array of ordered integers */
/*                           */

t_predef_int_array
  my_ordered_ints;

/*
*---------------------------------------------------------------------
*   Function prototypes
*---------------------------------------------------------------------
*/

/*                                                                                                */
/* Floating point number comparison is a VERY tricky business! The following code is adapted from */
/* https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/    */
/*                                                                                                */

/* Extraction of components */

int32_t float2mantissa (float f);
int32_t float2exponent (float f);

/* Comparison */

Bool floatAlmostZero                 (float A, float maxDiff);
Bool floatsAlmostEqualUlps           (float A, float B, int maxUlpsDiff);
Bool floatsAlmostEqualUlpsAndAbs     (float A, float B, float maxDiff, int maxUlpsDiff);
Bool floatsAlmostEqualRelativeAndAbs (float A, float B, float maxDiff, float maxRelDiff);


/*                                                          */
/* Bitwise operations                                       */
/*                                                          */
/* IMPORTANT: Bit offsets are calculated from right to left */
/* ie. bit 0 is the least significant bit of a given byte   */
/*     <------  MEMORY ADDRESSES GROW THIS WAY  <------     */
/*                                                          */

Bool get_bit  (t_byte *addr, int offset);
void set_bit  (t_byte *addr, int offset, int value);
void flip_bit (t_byte *addr, int offset);

size_t bitStr_diffs (
 t_byte *addr1,
 int     offset1,
 t_byte *addr2,
 int     offset2,
 int     blockSize,
 int     totalSize );

Bool bitStr_equal (
 t_byte *addr1,
 int     offset1,
 t_byte *addr2,
 int     offset2,
 int     size );

unsigned int bits2uint (
 t_byte *addr,
 int     offset,
 int     size);

void copy_bits (
 t_byte *origin_addr,
 int     origin_offset,
 t_byte *dest_addr,
 int     dest_offset,
 int     size );

/*                           */
/* Random numbers and events */
/*                           */

Bool     flip_coin (float prob);
float    gaussian (float mean, float std_dev);
long int my_randomize (Bool b_really_random);
void     random_array (t_byte *addr, int size);
char     random_char (char from, char to);
int      random_int (int from, int to);
float    random_float (float from, float to);
double   random_double (double from, double to);
int     *random_list_of_int (int from, int to);
void     restore_random_seed (unsigned int seed);
void     shuffle_array (t_byte *array, int elements, int bytes_each);
void     shuffle_array_of_int (int *array, int elements);
float   *random_distribution (int total, float lower, float upper, t_random_distrib distrib_type);

/*                         */
/* Trigonometric functions */
/*                         */

#ifdef USE_TRIG_TABLES
  float myCos (float theta);
  float mySin (float theta);
  void  read_trig_tables (void);
#endif

/*                   */
/* Simple statistics */
/*                   */

void acc2avgDev (
  int     totSamples,
  double  acc,
  double  accSq,
  double *avg,
  double *stdDev );

/* Simple statistical records */

int  setup_statRec     (void);
void newData_statRec   (int index, double data);
void dummyData_statRec (int index, double data);
void clear_statRec     (int index);
void dismiss_statRec   (int index);
t_statRec read_statRec (int index);

/* Matrix statistical records */

int  setup_matStatRec        (int totRows, int totCols);
void newData_matStatRec      (int index, int row, int col, double data);
void dummyData_matStatRec    (int index, int row, int col, double data);
void clear_matStatRec        (int index);
void dismiss_matStatRec      (int index);
t_statRec read_matStatRec    (int index);
t_statRec read_matStatRecRow (int index, int row);
t_statRec read_matStatRecCol (int index, int col);

/*        */
/* Others */
/*        */

void  initialize_maths (void);
float normalize_angle  (float   theta);
void  bytes2ints       (t_byte *bytes,  int    *ints,    int total);
void  floats2bytes     (float  *floats, t_byte *bytes,   int total);
void  floats2doubles   (float  *floats, double *doubles, int total);
void  floats2ints      (float  *floats, int    *ints,    int total);

/*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION (not visible from other modules)
*
*---------------------------------------------------------------------
*/

#ifdef USE_TRIG_TABLES
  static float
    sinTable[tableSize],      /* Table of sines   */
    cosTable[tableSize];      /* Table of cosines */
#endif

/*
*---------------------------------------------------------------------
*   Function prototypes
*---------------------------------------------------------------------
*/

static float  ran3                 (long int *idum);
static void   check_statRec        (int index);
static int    get_free_statRec     (void);
static float *normal_distribution  (int total, float lower, float upper);
static float *uniform_distribution (int total, float lower, float upper);

/*                                                                                                */
/* Floating point number comparison is a VERY tricky business! The following code is adapted from */
/* https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/    */
/*                                                                                                */

typedef union {
  int32_t intRep;
  float floatRep;
}
  Float_t;

static Float_t makeFloat (float f) {
  Float_t newFloat;
  newFloat.floatRep = f;
  return (newFloat);
}

static Bool isNegative (Float_t unionFloat)  {
  return (unionFloat.intRep < 0);
}

/* Extraction of components */

extern int32_t float2mantissa (float f) {
  Float_t unionFloat;
  unionFloat.floatRep = f;
  return (unionFloat.intRep & ((1 << 23) - 1));
}

extern int32_t float2exponent (float f) {
  Float_t unionFloat;
  unionFloat.floatRep = f;
  return ((unionFloat.intRep >> 23) & 0xFF);
}

/* Comparison */

Bool floatAlmostZero (float A, float maxDiff) {
  return (fabs(A) <= maxDiff);
}

Bool floatsAlmostEqualUlps (float A, float B, int maxUlpsDiff) {
  Float_t
    uA, uB;
  int
    ulpsDiff;

  uA = makeFloat(A);
  uB = makeFloat(B);

  /* Different signs means they do not match */

  if (isNegative(uA) != isNegative(uB))

    /* Check for equality to make sure +0==-0 */

	return (A == B);

  /* Find the difference in ULPs */

  ulpsDiff = abs(uA.intRep - uB.intRep);
  return (ulpsDiff <= maxUlpsDiff);
}

Bool floatsAlmostEqualUlpsAndAbs (float A, float B, float maxDiff, int maxUlpsDiff) {
  Float_t
    uA, uB;
  float
    absDiff;
  int
    ulpsDiff;

  /* Check if the numbers are really close - */
  /* Needed when comparing numbers near zero */

  absDiff = fabs(A - B);
  if (absDiff <= maxDiff)
    return (true);

  uA = makeFloat(A);
  uB = makeFloat(B);

  /* Different signs means they do not match */

  if (isNegative(uA) != isNegative(uB))
    return (false);

  /* Find the difference in ULPs */

  ulpsDiff = abs(uA.intRep - uB.intRep);
  return (ulpsDiff <= maxUlpsDiff);
}

Bool floatsAlmostEqualRelativeAndAbs (float A, float B, float maxDiff, float maxRelDiff) {
	float largest;
  /* Check if the numbers are really close - */
  /* Needed when comparing numbers near zero */

  float diff = fabs(A - B);
  if (diff <= maxDiff)
    return (true);
  A = fabs(A);
  B = fabs(B);
  largest = (B > A) ? B : A;
  return (diff <= largest * maxRelDiff);
}

/*
---------------------------------------------------------------------
    Returns the bit at address "addr" offset "offset"
    NOTE: "offset" must be >= 0
---------------------------------------------------------------------
*/

Bool get_bit (t_byte *addr, int offset)
{
 if (offset < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid offset (%d)", offset);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 addr += offset / BITS_PER_BYTE;
 offset %= BITS_PER_BYTE;
 return ((Bool) ((*addr >> offset) & 1));
}

/*
---------------------------------------------------------------------
    Sets the bit at address "addr" offset "offset" to "value"
    NOTE: "offset" must be >= 0
---------------------------------------------------------------------
*/

void set_bit (t_byte *addr, int offset, int value)
{
 if (offset < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid offset (%d)", offset);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 addr += offset / BITS_PER_BYTE;
 offset %= BITS_PER_BYTE;
 if (value)
   *addr |=  (1 << offset);
 else
   *addr &= ~(1 << offset);
}

/*
---------------------------------------------------------------------
    Changes the bit at address "addr" offset "offset"
    NOTE: "offset" must be >= 0
---------------------------------------------------------------------
*/

void flip_bit (t_byte *addr, int offset)
{
 if (offset < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid offset (%d)", offset);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 addr += offset / BITS_PER_BYTE;
 offset %= BITS_PER_BYTE;
 *addr ^= (1 << offset);
}

/*
---------------------------------------------------------------------
    Interprets a bit string as an unsigned integer. The bit string
    starts at address "addr" offset "offset" and is "size" bits long.
    Here is an example of the sort of thing we want to do...
---------------------------------------------------------------------


                                                +-------- offset = 6
                                                |
                  +-- last_byte                 |     +-- addr
     ignore_bits  |                             |     |
     = 5   vvvvv  v                             v     v
          +--------+  +--------+  +--------+  +--------+
     ...  |     uts|  |rqponmlk|  |jihgfedc|  |ba      | ...
          +--------+  +--------+  +--------+  +--------+
                ^^^                            ^^
                 trailing_bits = 3              leading_bits = 2

                <--------(size = 21 bits)------->




         <------  MEMORY ADDRESSES GROW THIS WAY  <------



                                                      +-- result
                                                      |  (4 bytes)
                                                      v
          +--------+  +--------+  +--------+  +--------+
     ...  |00000000|  |000utsrq|  |ponmlkji|  |hgfedcba| ...
          +--------+  +--------+  +--------+  +--------+

                           <------(size = 21 bits)----->


---------------------------------------------------------------------
    NOTE:

    if (offset < 0) or (! (0 <= size <= UINT_BITS))
      print_error_and_exit;
---------------------------------------------------------------------
*/

unsigned int bits2uint (t_byte *addr, int offset, int size)
{
 t_byte
   *last_byte,
   *byte;
 unsigned int
   temp,
   result;
 int
   ignore_bits,
   leading_bits,
   trailing_bits;

 if (offset < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid offset (%d)", offset);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if ((size < 0) || (size > (int) UINT_BITS)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid size (%d)", size);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 if (size == 0)
   return (0);

 /* Skip as many trailing bytes as necessary */

 addr += offset / BITS_PER_BYTE;
 offset %= BITS_PER_BYTE;

 /* Work out where the bit string ends */

 last_byte = addr + (size + offset - 1) / BITS_PER_BYTE;

 /* If the entire bit string falls in a single */
 /* byte then treat this as a special case...  */

 if (addr == last_byte) {
   ignore_bits = (int) UINT_BITS - (size + offset);
   temp = *addr;
   temp <<= ignore_bits;
   temp >>= UINT_BITS - size;
   return (temp);
 }

 /* The bit string crosses a byte boundary...  */
 /* Get the appropriate bits in the last byte  */

 leading_bits = BITS_PER_BYTE - offset;
 trailing_bits = (size - leading_bits) % BITS_PER_BYTE;
 ignore_bits = (int) UINT_BITS - trailing_bits;
 temp = *last_byte;
 result = (temp << ignore_bits) >> ignore_bits;

 /* Get all bits in intermediate bytes */

 for (byte = last_byte - 1; byte > addr; byte--) {
   result <<= BITS_PER_BYTE;
   result += (*byte);
 }

 /* Get the appropriate bits in the first byte */

 result <<= leading_bits;
 temp = *addr;
 result += (temp >> offset);

 return (result);
}

/*
---------------------------------------------------------------------
    Copies "size" bits from address "origin_addr" offset
    "origin_offset" to adress "dest_addr" offset "dest_offset".
    Overlapping regions are treated properly.
---------------------------------------------------------------------
*/

void copy_bits (
 t_byte *origin_addr,
 int     origin_offset,
 t_byte *dest_addr,
 int     dest_offset,
 int     size )
{
 t_byte
   *last_origin_byte,
   *last_dest_byte,
   *copy = NULL,
   *real_origin;
 int
   c_bit;
 size_t
   region_BYTES;
 Bool
   b_overlap;

 if (origin_offset < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid origin offset (%d)", origin_offset);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (dest_offset < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid destination offset (%d)", dest_offset);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if ((size < 0) || (size > (int) UINT_BITS)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid size (%d)", size);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 if (size == 0)
   return;

 /* Skip as many trailing bytes as necessary */

 origin_addr += origin_offset / BITS_PER_BYTE;
 origin_offset %= BITS_PER_BYTE;

 dest_addr += dest_offset / BITS_PER_BYTE;
 dest_offset %= BITS_PER_BYTE;

 /* Work out where the bit strings end */

 last_origin_byte = origin_addr + (size + origin_offset - 1) / BITS_PER_BYTE;
 last_dest_byte   = dest_addr   + (size + dest_offset   - 1) / BITS_PER_BYTE;

 /* If bit strings overlap then allocate memory for working space */

 b_overlap = ! (
   ((origin_addr < dest_addr) && (last_origin_byte < dest_addr)) ||
   ((dest_addr < origin_addr) && (last_dest_byte < origin_addr))
 );

 if (b_overlap) {
   region_BYTES = (size_t) (size - 1) / BITS_PER_BYTE + 1;
   errno = 0;
   if ((copy = (t_byte *) malloc (region_BYTES)) == NULL)
     ERROR_no_memory (errno, __FILE__, __func__, "copy");
   memset (copy, 0, region_BYTES);
   real_origin = copy;
 }
 else
   real_origin = origin_addr;;

 /* Now do the copying */

 /* NOTE:                                                            */
 /*                                                                  */
 /* The following loop is *NOT* a very clever way of doing this job. */
 /* Instead of calling "get_bit()" and "set_bit()" it would be much  */
 /* better to implement the low level bitwise operations by hand -   */
 /* something like was done in "bits2uint()"...                      */

 for (c_bit = 0; c_bit < size; c_bit++)
   set_bit (
     dest_addr,
     dest_offset + c_bit,
     get_bit (real_origin, origin_offset + c_bit) );

 if (copy)
   free (copy);
}

/*
---------------------------------------------------------------------
    Compares two bit strings of size "totalSize" located at
    {addr1,offset1} and {addr2,offset2}. Comparisons are
    performed between blocks of "blockSize" bits.
    Returns the number of blocks that don't match.
---------------------------------------------------------------------
*/

size_t bitStr_diffs (
 t_byte *addr1,
 int     offset1,
 t_byte *addr2,
 int     offset2,
 int     blockSize,
 int     totalSize )
{
 int
   c_blkBit,
   c_strBit,
   block1,
   block2,
   bit1,
   bit2;
 size_t
   diffs;

 if (offset1 < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid offset1 (%d)", offset1);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (offset2 < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid offset2 (%d)", offset2);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (blockSize < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid blockSize (%d)", blockSize);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (totalSize < 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid totalSize (%d)", totalSize);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (blockSize > totalSize) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid sizes: block=%d, total=%d",
     blockSize, totalSize);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 if ((blockSize == 0) || (totalSize == 0))
   return (0);

 diffs = 0;
 if (blockSize == 1) {

   /* Faster algorithm for bitwise comparison */

   for (c_strBit = 0, block1 = offset1, block2 = offset2;
          c_strBit < totalSize;
            c_strBit++, block1++, block2++ )
     if (get_bit (addr1, block1) != get_bit (addr2, block2))
       diffs++;
 }
 else {
   for (c_strBit = 0, block1 = offset1, block2 = offset2;
          c_strBit < totalSize;
            c_strBit += blockSize, block1 += blockSize, block2 += blockSize )
     for (c_blkBit = 0, bit1 = block1, bit2 = block2;
            c_blkBit < blockSize;
              c_blkBit++, bit1++, bit2++ )
       if (get_bit (addr1, bit1) != get_bit (addr2, bit2)) {
         diffs++;
         break;
       }
 }
 return (diffs);
}

/*
---------------------------------------------------------------------
    Compares two bit strings of size "size" located at
    {addr1,offset1} and {addr2,offset2}.
    Returns a boolean saying whether the bit strings are identical.
---------------------------------------------------------------------
*/

Bool bitStr_equal (
 t_byte *addr1,
 int     offset1,
 t_byte *addr2,
 int     offset2,
 int     size )
{
 int
   c_bit;

 for (c_bit = 0; c_bit < size; c_bit++)
   if (get_bit (addr1, offset1) != get_bit (addr2, offset2))
     return (FALSE);
 return (TRUE);
}

/*
---------------------------------------------------------------------
    Flip a biased coin; true if heads
---------------------------------------------------------------------
*/

Bool flip_coin (float prob)
{
 return ((Bool) (random_float (float_ZERO, float_ONE) < prob));
}

#ifdef USE_TRIG_TABLES

/*
---------------------------------------------------------------------
    Returns sin(theta)
---------------------------------------------------------------------
*/

float mySin (float theta)
{
 static float
   round = tablePrecision/2;
 int
   index_in_table;

 /* Normalize theta, making sure 0 <= theta <= 2*pi */

 while (theta > two_pi)
   theta -= two_pi;
 while (theta < 0)
   theta += two_pi;

/*
 return (*(sinTable+(int)((theta+round)*tableFactor)));
*/

 /* If theta is smaller than the maximum precision the */
 /* lookup table can cope with, then return correct    */
 /* value by calling the built-in sin function         */

 if ((index_in_table = (int)((theta+round)*tableFactor)) == 0)
   return ((float) sin((double) theta));
 else
   return (*(sinTable + index_in_table));
}

/*
---------------------------------------------------------------------
    Returns cos(theta)
---------------------------------------------------------------------
*/

float myCos (float theta)
{
 static float
   round = tablePrecision/2;
 int
   index_in_table;

 /* Normalize theta, making sure 0 <= theta <= 2*pi */

 while (theta > two_pi)
   theta -= two_pi;
 while (theta < 0)
   theta += two_pi;

/*
 return (*(cosTable+(int)((theta+round)*tableFactor)));
*/

 /* If theta is smaller than the maximum precision the */
 /* lookup table can cope with, then return correct    */
 /* value by calling the built-in cos function         */

 if ((index_in_table = (int)((theta+round)*tableFactor)) == 0)
   return ((float) cos((double) theta));
 else
   return (*(cosTable + index_in_table));
}

#endif

/*
---------------------------------------------------------------------
    Normalize theta, making sure 0 <= theta <= 2*pi
---------------------------------------------------------------------
*/

float normalize_angle (float theta)
{
 while (theta > two_pi)
   theta -= two_pi;
 while (theta < 0)
   theta += two_pi;
 return (theta);
}

/*
---------------------------------------------------------------------
    Use system time as seed for the random number generator
---------------------------------------------------------------------
*/

long int my_randomize (Bool b_really_random)
{
 time_t
   ltime,
   stime,
   prev_ltime = time (NULL),
   prev_stime = prev_ltime / 2;

 if (b_really_random)
   do
     stime = (ltime = time (NULL)) / 2;
   while (stime == prev_stime);
 else
   stime = 1234567890;

 random_seed = (long int) stime;
 RANDOM_init_genrand ((unsigned long int) random_seed);
 return (random_seed);
}

/*
---------------------------------------------------------------------
    Returns a random number within the given range
---------------------------------------------------------------------

#define random_number (type, from, to) \
   ((type) (from + (type)(to - from) * drand48()))

*/

/*
---------------------------------------------------------------------
    Fills an array with random bytes in the interval [0,255]
---------------------------------------------------------------------
*/

void random_array (t_byte *addr, int size)
{
 int
   c_byte;
 t_byte
   *p_byte;

 for (c_byte = 0, p_byte = addr; c_byte < size; c_byte++, p_byte++)
   *p_byte = (t_byte) (256 * RANDOM_genrand_res53());
}

/*
---------------------------------------------------------------------
    Returns a random char in the interval [from, to]
---------------------------------------------------------------------
*/

char random_char (char from, char to)
{
 return ((char) (from + (++to - from) * RANDOM_genrand_res53()));
}

/*
---------------------------------------------------------------------
    Returns a random int in the interval [from, to)
---------------------------------------------------------------------
*/

int random_int (int from, int to)
{
 return ((int) (from + (to - from) * RANDOM_genrand_res53()));
}

/*
---------------------------------------------------------------------
    Returns a random float in the interval [from, to)
---------------------------------------------------------------------
*/

float random_float (float from, float to)
{
 return ((float) (from + (to - from) * RANDOM_genrand_res53()));
}

/*
---------------------------------------------------------------------
    Returns a random double in the interval [from, to)
---------------------------------------------------------------------
*/

double random_double (double from, double to)
{
 return ((double) (from + (to - from) * RANDOM_genrand_res53()));
}

/*
---------------------------------------------------------------------
    Reads the trigonometric tables
---------------------------------------------------------------------
*/

#ifdef USE_TRIG_TABLES

void read_trig_tables (void)
{
 FILE
   *filePt;

 errno = 0;
 if ((filePt = fopen(tableName, "rb")) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
     "File [%s] cannot be opened for reading", tableName);
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if (fread (sinTable,
            sizeof (sinTable[0]),
            tableSize,
            filePt) != tableSize) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
     "File read error [%s]: sine table", tableName);
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 errno = 0;
 if (fread (cosTable,
            sizeof (cosTable[0]),
            tableSize,
            filePt) != tableSize) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
     "File read error [%s]: cosine table", tableName);
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
}

#endif

/*
---------------------------------------------------------------------
    Restore the state of the random number generator
---------------------------------------------------------------------
*/

void restore_random_seed (unsigned int seed)
{
 srand (seed);
}

/*
---------------------------------------------------------------------
    Shuffles the contents of an array of any type
---------------------------------------------------------------------
*/

void shuffle_array (t_byte *array, int elements, int bytes_each)
{
 t_byte
   *element,
   *temp;
 int
   index_in_array,
   index_in_temp,
   local_index;
 size_t
   array_size = (size_t) elements * bytes_each;

 errno = 0;
 if ((temp = (t_byte *) malloc (array_size)) == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "temp");
 errno = 0;
 if ((element = (t_byte *) malloc ((size_t) bytes_each)) == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "element");

 /* Make sure the shuffling really works */
 /* (quite often it doesn't at first!!!) */

 do {
   memcpy (temp, array, array_size);
   index_in_temp = 0;
   local_index = elements;
   while (local_index) {

     /* Pick elements at random positions */
     /* in array and put them into temp   */

     index_in_array = random_int (0, local_index);
     memcpy ((t_byte *) element,
             (t_byte *) array + (index_in_array * bytes_each),
             (unsigned int) bytes_each );
     if (index_in_array != (local_index - 1))
       memcpy ((t_byte *) array + (index_in_array * bytes_each),
               (t_byte *) array + ((local_index - 1) * bytes_each),
               (unsigned int) bytes_each );
     memcpy ((t_byte *) temp + ((index_in_temp++) * bytes_each),
             (t_byte *) element,
             (unsigned int) bytes_each );
     local_index--;
   }
 } while ((! memcmp (temp, array, array_size)) && (elements > 1));

 memcpy ((t_byte *) array, (t_byte *) temp, (unsigned int) array_size);
 free (element);
 free (temp);
}

/*
---------------------------------------------------------------------
    Shuffles the contents of an array of integers
---------------------------------------------------------------------
*/

void shuffle_array_of_int (int *array, int elements)
{
 int
   index_in_array,
   index_in_temp,
   number,
   remaining;
 size_t
   size_in_bytes = elements * sizeof (int);
 static int
   *temp = NULL;

 errno = 0;
 if ((temp = (int *) realloc (temp, size_in_bytes)) == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "temp");

 /* Make sure the shuffling really works */
 /* (quite often it doesn't at first!!!) */

 do {
   memcpy (temp, array, size_in_bytes);
   index_in_temp = 0;
   remaining = elements;
   while (remaining) {

     /* Pick numbers at random positions */
     /* in array and put them into temp  */

     number = array [index_in_array = random_int (0, remaining)];
     if (index_in_array != (remaining - 1))
       array [index_in_array] = array [remaining - 1];
     temp [index_in_temp++] = number;
     remaining--;
   }
 } while ((! memcmp (temp, array, size_in_bytes)) && (elements > 1));

 memcpy (array, temp, size_in_bytes);
}

/*
---------------------------------------------------------------------
    Set-up a list of random integers in specified range
    (no repetitions)
---------------------------------------------------------------------
*/

int *random_list_of_int (int from, int to)
{
 size_t
   bytes;
 int
   c_number,
   numbers;
 int
   *list = NULL;

 /* Make sure from <= to */

 if (from > to) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid range: [%d..%d]", from, to);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Allocate memory for list */

 numbers = to - from + 1;
 bytes = numbers * sizeof (int);
 errno = 0;
 if ((list = (int *) malloc (bytes)) == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "list");

 /* Fill it up */

 for (c_number = 0; c_number < numbers; c_number++)
   list [c_number] = from + c_number;

 /* Shuffle list */

 shuffle_array_of_int (list, numbers);

 /* Return pointer to the list */

 return (list);
}

/*
---------------------------------------------------------------------
    ???
    From Numerical Recipes in C;
    slightly modified to make sure it never returns 1.
---------------------------------------------------------------------
*/

#define MBIG 1000000000
#define MSEED 161803398
#define FAC (1.0/MBIG)

static float ran3 (long int *idum)
{
 static int
   inext,
   inextp,
   iff=0;
 static long int
   ma[56];
 long int
   mj,
   mk;
 int
   i,
   ii,
   k;
 float
   t;

 if (*idum<0 || iff==0) {
   iff = 1;
   mj = MSEED - ((*idum)<0 ? -(*idum) : (*idum));
   mj %= MBIG;
   ma[55] = mj;
   mk = 1;
   for (i=1; i<=54; i++) {
     ii = (21*i) % 55;
     ma[ii] = mk;
     mk = mj-mk;
     if (mk<0)
       mk += MBIG;
     mj = ma[ii];
   }
   for (k=1; k<=4; k++)
     for (i=1; i<=55; i++) {
       ma[i] -= ma[1+(i+30)%55];
       if (ma[i]<0)
         ma[i] += MBIG;
     }
   inext = 0;
   inextp = 31;
   *idum = 1;
 }
 if (++inext==56)
   inext = 1;
 if (++inextp==56)
   inextp = 1;
 mj = ma[inext] - ma[inextp];
 if (mj<0)
   mj+=MBIG;
 ma[inext] = mj;
 *idum = mj;

 /* return (mj*FAC); */

 t = (float) mj*FAC;

 if (! floatsAlmostEqualUlpsAndAbs (t, 1.0f, FLT_EPSILON, 5))
   return (t);
 else
   return (ran3(idum));
}

#undef MBIG
#undef MSEED
#undef FAC

/*
---------------------------------------------------------------------
    Gaussian distribution (from Numerical Recipes in C)
    Slightly modified to allow any mean or standard deviation.
    Returns a random number as a float.
---------------------------------------------------------------------
*/

float gaussian (float mean, float std_dev)
{
 static int
   iset = 0;
 static float
   next;
 float
   curr,
   fac,
   rsq,
   v1,
   v2;

 if (iset == 0) {
   do {
     v1  = 2.0*ran3(&random_seed) - 1.0;
     v2  = 2.0*ran3(&random_seed) - 1.0;
     rsq = v1*v1 + v2*v2;
   }
     while (rsq >= 1.0 || floatAlmostZero (rsq, FLT_MIN));
   fac  = sqrt (-2.0*log(rsq)/rsq);
   curr = v1*fac;
   next = v2*fac;
   iset = 1;
   return (mean + std_dev*curr);
 }
 else {
   iset=0;
   return (mean + std_dev*next);
 }
}

/*
---------------------------------------------------------------------
    Normal distribution.
    Returns a pointer to an array of "total" random floats in the
    interval [lower..upper] drawn from a normal distribution.
---------------------------------------------------------------------
*/

static float *normal_distribution (int total, float lower, float upper)
{
 int
   c_number;
 float
   factor,
   mean,
   minNumber,
   maxNumber,
   range,
   *p_number;
 static float
   *array = NULL;

 /* Re-allocate memory for array of random numbers */

 errno = 0;
 array = (float *) realloc (array, total * sizeof (float));
 if (array == NULL)
   ERROR_no_memory (errno, "", "normal_distribution", "array");

 /* Generate random numbers */

 minNumber = max_FLOAT;
 maxNumber = min_FLOAT;
 mean = (lower + upper) / 2;
 for (c_number = 0; c_number < total; c_number++) {
   p_number = (float *) array + c_number;
   (*p_number) = (float) gaussian ((float) mean, (float) 1.0);
   minNumber = SMALLEST (*p_number, minNumber);
   maxNumber = GREATEST (*p_number, maxNumber);
 }

 /* Normalize the distribution */

 range = maxNumber - minNumber;
 factor = (upper - lower) / range;
 for (c_number = 0; c_number < total; c_number++) {
   p_number = (float *) array + c_number;
   (*p_number) = (float) lower + factor * (*p_number - minNumber);
 }

 return (array);
}

/*
---------------------------------------------------------------------
    Uniform distribution.
    Returns a pointer to an array of "total" random floats in the
    interval [lower..upper) drawn from a linear distribution.
---------------------------------------------------------------------
*/

static float *uniform_distribution (int total, float lower, float upper)
{
 int
   c_number;
 float
   range,
   *p_number;
 static float
   *array = NULL;

 /* Free memory allocated by a previous call to this procedure */

 if (array)
   free (array);

 /* Allocate memory for array of random numbers */

 errno = 0;
 array = (float *) malloc (total * sizeof (float));
 if (array == NULL)
   ERROR_no_memory (errno, "", "uniform_distribution", "array");

 /* Generate random numbers */

 range = upper - lower;
 for (c_number = 0; c_number < total; c_number++) {
   p_number = (float *) array + c_number;
   (*p_number) = lower + range * ran3(&random_seed);
 }

 return (array);
}

/*
---------------------------------------------------------------------
    Random distribution.
    Returns a pointer to an array of "total" random floats in the
    interval [lower..upper] drawn from the specified distribution.
---------------------------------------------------------------------
*/

float *random_distribution (
  int              total,
  float            lower,
  float            upper,
  t_random_distrib distrib_type )
{
 switch (distrib_type) {
   case (t_uniform_distrib):
     return (uniform_distribution (total, lower, upper));
   case (t_normal_distrib):
     return (normal_distribution (total, lower, upper));
   default: {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid distribution type (%d)", distrib_type);
     ERROR_fatal_error(0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }
}

/*
---------------------------------------------------------------------
    Convert an array of unsigned chars into an array of integers.
    Both arrays must already exist.
---------------------------------------------------------------------
*/

void bytes2ints (t_byte *bytes, int *ints, int total)
{
 int
   c_number;
 t_byte
   *p_byte;
 int
   *p_int;

 for (c_number = 0, p_byte = bytes, p_int = ints;
        c_number < total;
          c_number++, p_byte++, p_int++)
   (*p_int) = (int) ROUND (*p_byte);
}

/*
---------------------------------------------------------------------
    Convert an array of floats into an array of unsigned chars.
    Both arrays must already exist.
---------------------------------------------------------------------
*/

void floats2bytes (float *floats, t_byte *bytes, int total)
{
 int
   c_number;
 float
   *p_float;
 t_byte
   *p_byte;

 for (c_number = 0, p_float = floats, p_byte = bytes;
        c_number < total;
          c_number++, p_float++, p_byte++)
   (*p_byte) = (t_byte) ROUND (*p_float);
}

/*
---------------------------------------------------------------------
    Convert an array of floats into an array of integers.
    Both arrays must already exist.
---------------------------------------------------------------------
*/

void floats2ints (float *floats, int *ints, int total)
{
 int
   c_number;
 float
   *p_float;
 int
   *p_int;

 for (c_number = 0, p_float = floats, p_int = ints;
        c_number < total;
          c_number++, p_float++, p_int++)
   (*p_int) = ROUND (*p_float);
}

/*
---------------------------------------------------------------------
    Convert an array of floats into an array of doubles.
    Both arrays must already exist.
---------------------------------------------------------------------
*/

void floats2doubles (float *floats, double *doubles, int total)
{
 int
   c_number;
 float
   *p_float;
 double
   *p_double;

 for (c_number = 0, p_float = floats, p_double = doubles;
        c_number < total;
          c_number++, p_float++, p_double++)
   (*p_double) = (double) (*p_float);
}

/*
---------------------------------------------------------------------
    Calculate the mean and standard deviation of a set of samples.
    "totSamples", "acc" and "accSq" must be given.
---------------------------------------------------------------------
*/

void acc2avgDev (
  int     totSamples,
  double  acc,
  double  accSq,
  double *avg,
  double *stdDev )
{
 double
   Avg,
   Var,
   Dev;

 if (totSamples < 1) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid number of samples (%d)", totSamples);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 Avg = acc / totSamples;
 if (totSamples > 1)
   Var = (accSq - acc*acc / totSamples) / (totSamples - 1);
 else
   Var = (double) 0;
 Var = ABS (Var);
 Dev = (double) SQRT (Var);
 *avg = Avg;
 *stdDev = Dev;
}

/*
 *---------------------------------------------------------------------
 *
 *  Simple statistical records
 *
 *---------------------------------------------------------------------
 *
 */

#define STAT_REC_BLK_SIZE  20

typedef struct {
  Bool         b_engaged;
  unsigned int samples;
  double       min;
  double       max;
  double       acc;
  double       accSq;
}
  t_statRecInfo;

/* ------------------------------ */
/* Simple statRec's info vector   */
/* ------------------------------ */
/* Name      : statRecVec         */
/* Type      : t_statRecInfo      */
/* Dimensions: [0..totStatRecs-1] */
/* ------------------------------ */

static t_statRecInfo
  *statRecVec = NULL;

static int
  totStatRecs = 0;

/*
---------------------------------------------------------------------
    Find a free statistical record and return its index.
    If all records are engaged then create some more.
---------------------------------------------------------------------
*/

static int get_free_statRec (void)
{
 int
   index,
   newTotal;
 size_t
   newSize,
   oldSize;

 /* If there is a free record, then use it */

 for (index = 0; index < totStatRecs; index++)
   if (! statRecVec[index].b_engaged)
     return (index);

 /* All records are engaged, so allocate space for more records */

 newTotal = totStatRecs + STAT_REC_BLK_SIZE;
 oldSize = totStatRecs * sizeof (t_statRecInfo);
 newSize = newTotal * sizeof (t_statRecInfo);
 errno = 0;
 statRecVec = (t_statRecInfo *) realloc ((void *) statRecVec, newSize);
 if (statRecVec == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "statRecVec");
 memset (&(statRecVec[totStatRecs]), 0, newSize - oldSize);
 index = totStatRecs;
 totStatRecs = newTotal;
 return (index);
}

/*
---------------------------------------------------------------------
    Check if a given statistical record is in use
---------------------------------------------------------------------
*/

static void check_statRec (int index)
{
 if (index >= totStatRecs)  {
  snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Statistical record %d does not exist", index);
  ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (! statRecVec[index].b_engaged) {
  snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Statistical record %d is not in use", index);
  ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
}

/*
---------------------------------------------------------------------
    Set up a simple statistical record and return its index
---------------------------------------------------------------------
*/

int setup_statRec (void)
{
 int
   index;

 index = get_free_statRec();
 statRecVec[index].b_engaged = TRUE;
 clear_statRec (index);
 return (index);
}

/*
---------------------------------------------------------------------
    Store data in a simple statistical record
---------------------------------------------------------------------
*/

void newData_statRec (int index, double data)
{
 check_statRec (index);
 statRecVec[index].samples++;
 if (data > statRecVec[index].max)
   statRecVec[index].max = data;
 if (data < statRecVec[index].min)
   statRecVec[index].min = data;
 statRecVec[index].acc += data;
 statRecVec[index].accSq += data * data;
}

/*
---------------------------------------------------------------------
    Use data to update "min" and "max" fields of a simple statistical
    record but do NOT store the data
---------------------------------------------------------------------
*/

void dummyData_statRec (int index, double data)
{
 check_statRec (index);
 if (data > statRecVec[index].max)
   statRecVec[index].max = data;
 if (data < statRecVec[index].min)
   statRecVec[index].min = data;
}

/*
---------------------------------------------------------------------
    Clear a simple statistical record.
---------------------------------------------------------------------
*/

void clear_statRec (int index)
{
 check_statRec (index);
 statRecVec[index].samples = (unsigned int) 0;
 statRecVec[index].min     = (double) max_DOUBLE;
 statRecVec[index].max     = (double) min_DOUBLE;
 statRecVec[index].acc     = (double) 0;
 statRecVec[index].accSq   = (double) 0;
}

/*
---------------------------------------------------------------------
    Dismiss (free) a simple statistical record.
---------------------------------------------------------------------
*/

void dismiss_statRec (int index)
{
 clear_statRec (index);
 statRecVec[index].b_engaged = FALSE;
}

/*
---------------------------------------------------------------------
    Return data stored in a simple statistical record.
---------------------------------------------------------------------
*/

t_statRec read_statRec (int index)
{
 double
   Var,
   Dev;
 t_statRec
   statRec;

 check_statRec (index);
 statRec.samples = statRecVec[index].samples;
 statRec.acc = statRecVec[index].acc;
 statRec.accSq = statRecVec[index].accSq;
 if (statRec.samples == 0)
   statRec.min = statRec.max = statRec.avg = statRec.dev = (double) 0;
 else {
   statRec.min = statRecVec[index].min;
   statRec.max = statRecVec[index].max;
   statRec.avg = statRec.acc / statRec.samples;
   if (statRec.samples > 1)
     Var = (statRec.accSq - statRec.acc*statRec.acc / statRec.samples) /
           (statRec.samples - 1);
   else
     Var = (double) 0;
   Var = ABS (Var);
   Dev = (double) SQRT (Var);
   statRec.dev = Dev;
 }
 return (statRec);
}

/*
 *---------------------------------------------------------------------
 *
 *  Matrix statistical records (or "matrix statRec"s for short)
 *
 *---------------------------------------------------------------------
 *
 *  Useful for simple statistical analysis of data arranged in
 *  2D-matrices. Consider the matrix M below:
 *
 *
 *                                 accRowsVec
 *           +-----------------+   +--+
 *           |  |  |  |  |  |  |   |  |
 *           |--|--|--|--|--|--|   +--+
 *           |  |  |  |  |  |  |   |  |
 *       M = |--|--|--|--|--|--|   +--+
 *           |  |  |  |  |  |  |   |  |
 *           |--|--|--|--|--|--|   +--+
 *           |  |  |  |  |  |  |   |  |
 *           +-----------------+   +--+
 *
 *           +--+--+--+--+--+--+   +--+
 *           |  |  |  |  |  |  |   |  |
 *           +--+--+--+--+--+--+   +--+
 *           accColsVec            accMat
 *
 *
 *  Suppose that rows represent network units, columns represent tasks,
 *  and matrix elements represent performance indices. For example,
 *    M(3,2) = 0.777
 *  means that the performance index of unit 3 in task 2 is 0.777 .
 *  A matrix statRec can be used to analyse the data in the array by
 *  rows and by columns independently. Note that the data need not be
 *  actually stored in an array; a matrix statRec can be used with any
 *  data having a tabular nature. When a program requests a matrix statRec,
 *  three "entities" are created in association with the matrix statRec:
 *
 *    - "accRowsVec":
 *      array of "totRows" statRecs (or rather statRecIds) where the
 *      data submitted to the matrix statRec is grouped by rows.
 *
 *    - "accColsVec":
 *      array of "totCols" statRecs (or rather statRecIds) where the
 *      data submitted to the matrix statRec is grouped by columns.
 *
 *    - "accMat":
 *      a statRec (or rather statRecId) where all data submitted
 *      to the matrix statRec is grouped together.
 *
 *  For the example above a matrix statRec is created with
 *
 *    statRecId = setup_matStatRec (4,6);
 *
 *  Data is stored/entered in the statRec with something like
 *    newData_matStatRec (statRecId, 3, 2, 0.777);
 *
 *  This call has the same effect as the following operations:
 *    newData_statRec (accRowsVec[3], 0.777);
 *    newData_statRec (accColsVec[2], 0.777);
 *    newData_statRec (accMat, 0.777);
 *
 *  Individual statRecs in "accRowsVec" and "accColsVec", as well
 *  as "accMat", can be accessed with something like
 *    statRec = read_matStatRecRow (statRecId, 3);
 *    statRec = read_matStatRecCol (statRecId, 2);
 *    statRec = read_matStatRec (statRecId);
 *
 *  A matrix statRec can be released (ie. terminated) with
 *    dismiss_matStatRecRow (statRecId);
 *  causing all memory dinamically allocated for the matrix statRec
 *  to be freed.
 *
 *  NOTE:
 *  A matrix statRec creates only "totRows+totCols+1" simple statRecs.
 *  It does *NOT* create statRecs for individual matrix positions.
 *  If that is required, they should be explicitly created with
 *  as many calls to setup_statRec() as necessary.
 *---------------------------------------------------------------------
*/

#define MAT_STAT_REC_BLK_SIZE  20

typedef struct {
  Bool b_engaged;
  int  totRows;
  int  totCols;
  int *accRowsVec;
  int *accColsVec;
  int  accMat;
}
  t_matStatRecInfo;

/* ------------------------------ */
/* Matrix statRec's info vector   */
/* ------------------------------ */
/* Name      : matStatRecVec      */
/* Type      : t_matStatRecInfo   */
/* Dimensions: [0..totStatRecs-1] */
/* ------------------------------ */

static t_matStatRecInfo
  *matStatRecVec = NULL;

static int
  totMatStatRecs = 0;

/* ------------------------------- */
/* Pointer to matStatRecVec[index] */
/* ------------------------------- */

#define p_MAT_STAT_REC(index)  \
  ((t_matStatRecInfo *) matStatRecVec + (index))

/* ----------------------------------------------- */
/* Matrix statRec's accRows vector                 */
/* ----------------------------------------------- */
/* Name      : matStatRecVec[].accRowsVec          */
/* Type      : (int *)                             */
/* Dimensions: [0..totRows-1]                      */
/* ----------------------------------------------- */
/* Matrix statRec's accCols vector                 */
/* ----------------------------------------------- */
/* Name      : matStatRecVec[].accColsVec          */
/* Type      : (int *)                             */
/* Dimensions: [0..totCols-1]                      */
/* ----------------------------------------------- */

/*
---------------------------------------------------------------------
    Find a free matrix statistical record and return its index.
    If all records are engaged then create some more.
---------------------------------------------------------------------
*/

static int get_free_matStatRec (void)
{
 int
   index,
   newTotal;
 size_t
   newSize,
   oldSize;

 /* If there is a free record, then use it */

 for (index = 0; index < totMatStatRecs; index++)
   if (! matStatRecVec[index].b_engaged)
     return (index);

 /* All records are engaged, so allocate space for more records */

 newTotal = totMatStatRecs + MAT_STAT_REC_BLK_SIZE;
 oldSize = totMatStatRecs * sizeof (t_matStatRecInfo);
 newSize = newTotal * sizeof (t_matStatRecInfo);
 errno = 0;
 matStatRecVec =
   (t_matStatRecInfo *) realloc ((void *) matStatRecVec, newSize);
 if (matStatRecVec == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "matStatRecVec");
 memset (&(matStatRecVec[totMatStatRecs]), 0, newSize - oldSize);
 index = totMatStatRecs;
 totMatStatRecs = newTotal;
 return (index);
}

/*
---------------------------------------------------------------------
    Check if a given matrix statistical record is in use
---------------------------------------------------------------------
*/

static void check_matStatRec (int index)
{
 if (index >= totMatStatRecs)  {
  snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Matrix statistical record %d does not exist", index);
  ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (! matStatRecVec[index].b_engaged) {
  snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Matrix statistical record %d is not in use", index);
  ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
}

/*
---------------------------------------------------------------------
    Set up a matrix statistical record and return its index
---------------------------------------------------------------------
*/


int setup_matStatRec (int totRows, int totCols)
{
 int
   c_row,
   c_col,
   index,
   *vector = NULL;
 size_t
   vector_BYTES;
 t_matStatRecInfo
   *p_info;

 /* Get a free position in array of matrix statRec's */

 index = get_free_matStatRec();
 p_info = p_MAT_STAT_REC (index);

 /* Allocate and initialize "accRowsVec" */

 vector_BYTES = totRows * sizeof (int *);
 errno = 0;
 vector = (int *) malloc (vector_BYTES);
 if (vector == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "matStatRecVec[%d].accRowsVec", index);
   ERROR_no_memory (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 for (c_row = 0; c_row < totRows; c_row++)
   vector[c_row] = setup_statRec();
 p_info->accRowsVec = vector;

 /* Allocate and initialize "accColsVec" */

 vector_BYTES = totCols * sizeof (int *);
 errno = 0;
 vector = (int *) malloc (vector_BYTES);
 if (vector == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "matStatRecVec[%d].accColsVec", index);
   ERROR_no_memory (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 for (c_col = 0; c_col < totCols; c_col++)
   vector[c_col] = setup_statRec();
 p_info->accColsVec = vector;

 /* Final initializations */

 p_info->accMat    = setup_statRec();
 p_info->totRows   = totRows;
 p_info->totCols   = totCols;
 p_info->b_engaged = TRUE;
 clear_matStatRec (index);
 return (index);
}

/*
---------------------------------------------------------------------
    Store data in a matrix statistical record
---------------------------------------------------------------------
*/

void newData_matStatRec (int index, int row, int col, double data)
{
 t_matStatRecInfo
   *p_info;

 check_matStatRec (index);
 p_info = p_MAT_STAT_REC (index);

 /* Make sure "row" and "col" are valid */

 ERROR_check_range_int (
   (int)    row,
   (int)    0,
   (int)    p_info->totRows - 1,
   (char *) "row",
   (char *) __func__,
   (char *) __FILE__ );
 ERROR_check_range_int (
   (int)    col,
   (int)    0,
   (int)    p_info->totCols - 1,
   (char *) "col",
   (char *) __func__,
   (char *) __FILE__ );

 /* Update appropriate statistical records */

 newData_statRec (p_info->accRowsVec[row], data);
 newData_statRec (p_info->accColsVec[col], data);
 newData_statRec (p_info->accMat, data);
}

/*
---------------------------------------------------------------------
    Use data to update "min" and "max" fields of a matrix statistical
    record but do NOT store the data
---------------------------------------------------------------------
*/

void dummyData_matStatRec (int index, int row, int col, double data)
{
 t_matStatRecInfo
   *p_info;

 check_matStatRec (index);
 p_info = p_MAT_STAT_REC (index);

 /* Make sure "row" and "col" are valid */

 ERROR_check_range_int (
   (int)    row,
   (int)    0,
   (int)    p_info->totRows - 1,
   (char *) "row",
   (char *) __func__,
   (char *) __FILE__ );
 ERROR_check_range_int (
   (int)    col,
   (int)    0,
   (int)    p_info->totCols - 1,
   (char *) "col",
   (char *) __func__,
   (char *) __FILE__ );

 /* Update appropriate statistical records */

 dummyData_statRec (p_info->accRowsVec[row], data);
 dummyData_statRec (p_info->accColsVec[col], data);
 dummyData_statRec (p_info->accMat, data);
}

/*
---------------------------------------------------------------------
    Clear a matrix statistical record, or rather the simple
    statRec's associated with it -- DO NOT dismiss them!
---------------------------------------------------------------------
*/

void clear_matStatRec (int index)
{
 int
   c_row,
   c_col;
 t_matStatRecInfo
   *p_info;

 check_matStatRec (index);
 p_info = p_MAT_STAT_REC (index);

 /* Clear all simple statRec's associated with this matrix statRec */

 for (c_row = 0; c_row < p_info->totRows; c_row++)
   clear_statRec (p_info->accRowsVec[c_row]);

 for (c_col = 0; c_col < p_info->totCols; c_col++)
   clear_statRec (p_info->accColsVec[c_col]);

 clear_statRec (p_info->accMat);
}

/*
---------------------------------------------------------------------
    Dismiss (free) a matrix statistical record and all simple
    statRec's associated with it
---------------------------------------------------------------------
*/

void dismiss_matStatRec (int index)
{
 int
   c_row,
   c_col;
 t_matStatRecInfo
   *p_info;

 check_matStatRec (index);
 p_info = p_MAT_STAT_REC (index);

 /* Dismiss all simple statRec's associated with this matrix statRec */

 for (c_row = 0; c_row < p_info->totRows; c_row++)
   dismiss_statRec (p_info->accRowsVec[c_row]);

 for (c_col = 0; c_col < p_info->totCols; c_col++)
   dismiss_statRec (p_info->accColsVec[c_col]);

 dismiss_statRec (p_info->accMat);

 /* Free dynamically allocated memory */

 free (p_info->accRowsVec);
 free (p_info->accColsVec);

 /* Finally... */

 p_info->accMat    = -1;
 p_info->totRows   = 0;
 p_info->totCols   = 0;
 p_info->b_engaged = FALSE;
}

/*
---------------------------------------------------------------------
    Return accumulated data stored in a matrix statRec
---------------------------------------------------------------------
*/

t_statRec read_matStatRec (int index)
{
 t_statRec
   statRec;

 check_matStatRec (index);
 statRec = read_statRec (matStatRecVec[index].accMat);
 return (statRec);
}

/*
---------------------------------------------------------------------
    Return accumulated data stored in a row of a matrix statRec
---------------------------------------------------------------------
*/

t_statRec read_matStatRecRow (int index, int row)
{
 t_matStatRecInfo
   *p_info;
 t_statRec
   statRec;

 check_matStatRec (index);
 p_info = p_MAT_STAT_REC (index);

 /* Make sure "row" is valid */

 ERROR_check_range_int (
   (int)    row,
   (int)    0,
   (int)    p_info->totRows - 1,
   (char *) "row",
   (char *) __func__,
   (char *) __FILE__ );

 /* Access appropriate statistical records */

 statRec = read_statRec (p_info->accRowsVec[row]);
 return (statRec);
}

/*
---------------------------------------------------------------------
    Return accumulated data stored in a col of a matrix statRec
---------------------------------------------------------------------
*/

t_statRec read_matStatRecCol (int index, int col)
{
 t_matStatRecInfo
   *p_info;
 t_statRec
   statRec;

 check_matStatRec (index);
 p_info = p_MAT_STAT_REC (index);

 /* Make sure "col" is valid */

 ERROR_check_range_int (
   (int)    col,
   (int)    0,
   (int)    p_info->totCols - 1,
   (char *) "col",
   (char *) __func__,
   (char *) __FILE__ );

 /* Access appropriate statistical records */

 statRec = read_statRec (p_info->accColsVec[col]);
 return (statRec);
}

/*
---------------------------------------------------------------------
    Math library exception handling
---------------------------------------------------------------------
*/

/****

int matherr (register struct exception *err)
{

 switch (err->type) {
   case DOMAIN:
     printf ("DOMAIN");
     if (!strcmp (err->name, "sqrt"))
       err->retval = sqrt (- err->arg1);
     break;
   case SING:
     printf ("SINGULARITY");
     break;
   case OVERFLOW:
     printf ("OVERFLOW");
     break;
   case UNDERFLOW:
     printf ("UNDERFLOW");
     break;
 }
 printf (" ERROR in %s [Args:%f %f][Res:%f]\n",
         err->name, err->arg1, err->arg2, err->retval);
}

****/

/*
---------------------------------------------------------------------
    Initializations
---------------------------------------------------------------------
*/

void initialize_maths (void)
{
 int
   index;

 (void) my_randomize (TRUE);

#ifdef USE_TRIG_TABLES
 read_trig_tables();
#endif

 for (index = 0; index < predef_array_SIZE; index++)
   my_ordered_ints [index] = index;
}
