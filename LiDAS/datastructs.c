/*
*---------------------------------------------------------------------
*
*   File         : datastructs.c
*   Created      : 2006-11-02
*   Last Modified: 2012-05-27
*
*   Type-safe implementation of several dynamically allocated data
*   structures and access methods - or as type safe as we can
*   make them in C
*
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
*   TO DO LIST
*---------------------------------------------------------------------

[X] In all top-level data structure methods: once the data structure
    is created, there is no need pass the data size as a parameter to
    the manipulation methods, so remove that parameter

[X] In all top-level data structure creation methods:
     - rename blockSize to initialBlockSize
     - take out the resizeLimit and resizeFactor, but keep them in
       new_genericArray() and the other implementation methods
     - create new maxDataItems, beyond which the generic array will not grow

[X] Write dataType2dataTypeStr() method for improved error reporting

[X] Rewrite all error messages to make them more informative

[X] Change position_in_genericArray() method to take the occurrence of the
    element seeked as a parameter

[X] Implement DATA_is_empty_() methods:
     [X] DATA_is_empty_set()
     [X] DATA_is_empty_stack()
     [X] DATA_is_empty_queue()

[X] Implement a few useful data structure methods:
     [X] DATA_set_union()
     [X] DATA_set_intersection()
     [X] DATA_set_difference()
     [X] DATA_set_symmetric_difference()
     [X] DATA_are_equal_sets()
     [X] DATA_is_subset()

[X] In linux: check for memory leaks with valgrind

[X] In DATA_initialize_module() pass a Bool parameter saying
    whether run-time errors in data structure methods should quietly
    return FALSE, or whether they should cause the program to abort.
    In all void methods (interface and implementation):
     - return Bool saying whether the method was successful, rather
       than aborting the program with an error message

[X] Implement DATA_multiple_store_in_array() method that takes a pointer to a
    data item and fills specified portions of the array with copies of it

[X] Implement new data lookup methods:
     [X] DATA_is_in_array()
     [X] DATA_is_in_set()
     [X] DATA_is_on_stack()
     [X] DATA_is_in_queue()
     [X] DATA_where_in_array()
     [X] DATA_where_in_set()
     [X] DATA_where_on_stack()
     [X] DATA_where_in_queue()
     [X] DATA_count_in_set()
     [X] DATA_count_in_array()
     [X] DATA_count_on_stack()
     [X] DATA_count_in_queue()

[X] Data duplication, lookup methods and supporting routines: consider data 
    subsets (ranges of bytes in each data item) when looking for a match:
     [X] position_in_genericArray()
     [X] occurrences_in_genericArray()
     [X] DATA_copy_array()
     [X] DATA_is_in_array()
     [X] DATA_where_in_array()
     [X] DATA_count_in_array()
     [X] DATA_new_set()
     [X] DATA_new_set2()
     [X] DATA_copy_set()
     [X] DATA_remove_from_set()
     [X] DATA_is_in_set()
     [X] DATA_where_in_set()
     [X] DATA_count_in_set()
     [X] DATA_copy_stack()
     [X] DATA_is_on_stack()
     [X] DATA_where_on_stack()
     [X] DATA_count_on_stack()
     [X] DATA_copy_queue()
     [X] DATA_is_in_queue()
     [X] DATA_where_in_queue()
     [X] DATA_count_in_queue()

[X] Data lookup methods and supporting routines: allow the specification
    of how data subsets should be combined when looking for a match:
    NOT, AND, OR, XOR

[ ] Implement further data structure methods:
     [ ] merge_stacks()
     [ ] merge_queues()

[ ] Implement new types of data structure:
     [ ] multi-dimensional matrix
     [ ] multiset (one that allows element duplication)
     [ ] priorityQueue

[ ] Implement matrix operations:
     [ ] addition
     [ ] subtraction
     [ ] scalar product
     [ ] vector product
     [ ] etc.

[ ] Implement compact_data_structure() method, which sizes down the
    data structure thus reducing its memory usage

[ ] Rewrite the code that sets up genericArrays_infoVector: treat
    this array as any other dinamically allocated array and use the
    same methods for creating arrays, sets etc

[ ] Write a test program that reads instructions from the command line
    (such as create set, add elements to set, etc) and displays the
    results interactively

[ ] Add #define DEBUG_COMPILE 1
    and methods for accessing internal fields of generic array info records

*/

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "datastructs.h"

/*
*---------------------------------------------------------------------
*
*   INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

/*
*-----------------------------------------------------------------------
*   Data types and global variables
*-----------------------------------------------------------------------
*/

static Bool
  is_data_structures_initialized = FALSE,
  has_data_structure_failed      = FALSE;

/*
*---------------------------------------------------------------------
*   Function prototypes
*---------------------------------------------------------------------
*/

Bool  DATA_is_module_initialized     (void);
Bool  DATA_has_data_structure_failed (void);

void  DATA_initialize_module (Bool abort_on_error);
char *DATA_get_error         (void);
void  DATA_destroy_all       (void);

/*--------------------*/
/* Methods for ARRAYS */
/*--------------------*/

/* Set up an empty array and return its numeric code */

DATA_t_array_code DATA_new_array (size_t dataSize, unsigned int maxDataItems);

DATA_t_array_code DATA_new_array2 (
 size_t       dataSize,
 unsigned int maxDataItems,
 unsigned int initialBlockSize,
 unsigned int resizeLimit,
 unsigned int resizeFactor );

/* Copy part of an existing array into a new one and return the new array's numeric code */

DATA_t_array_code DATA_copy_array (
 DATA_t_array_code  arrayCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Store a data item in a array */

void DATA_store_in_array (
 DATA_t_array_code arrayCode,
 unsigned int elementPosition,
 void        *p_dataItem );

/* Store multiple copies of a data item in specified positions in a array */

void DATA_multiple_store_in_array (
 DATA_t_array_code  arrayCode,
 void         *p_dataItem,
 unsigned int  arrayBlocks,
 ... );
 
/* Retrieve a data item stored in a array */

void DATA_read_from_array (
 DATA_t_array_code arrayCode,
 unsigned int elementPosition,
 void        *p_dataItem );

/* Find out if a data item is in a array */

Bool DATA_is_in_array (
 DATA_t_array_code  arrayCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Find the position of a data item in a array */

unsigned int DATA_where_in_array (
 DATA_t_array_code   arrayCode,                           
 unsigned int   occurrence,                                   
 void          *p_dataPattern,                                  
 unsigned int   patternSubsets,
 ... );

/* Find out how many data items in a array share the same portions of data */

unsigned int DATA_count_in_array (
 DATA_t_array_code  arrayCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Is the array empty? */

Bool DATA_is_empty_array (DATA_t_array_code arrayCode);

/* Obtain the largest array index in use (1 onwards) */

unsigned int DATA_get_array_size (DATA_t_array_code arrayCode);

/* Clear all data stored in a array */

void DATA_clear_array (DATA_t_array_code arrayCode);

/* Delete an array and its data */

void DATA_destroy_array (DATA_t_array_code arrayCode);

/*------------------*/
/* Methods for SETS */
/*------------------*/

/* Set up an empty set, define set membership and return its numeric code */

DATA_t_set_code DATA_new_set (
 size_t        dataSize, 
 unsigned int  maxDataItems,
 unsigned int  patternSubsets,
 ... );

DATA_t_set_code DATA_new_set2 (
 size_t        dataSize,
 unsigned int  maxDataItems,
 unsigned int  initialBlockSize,
 unsigned int  resizeLimit,
 unsigned int  resizeFactor,
 unsigned int  patternSubsets,
 ... );

/* Copy part of an existing set into a new one and return its numeric code */

DATA_t_set_code DATA_copy_set (
 DATA_t_set_code    setCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

 /* Add a data item to a set */

void DATA_add_to_set (DATA_t_set_code setCode, void *p_dataItem, Bool b_overwrite);

/* Retrieve a data item in a set */

void DATA_read_from_set (
 DATA_t_set_code    setCode,
 unsigned int  elementPosition,
 void         *p_dataItem );

/* Remove certain data items from a set */

void DATA_remove_from_set (
 DATA_t_set_code    setCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

/* Find out if a data item is in a set */

Bool DATA_is_in_set (
 DATA_t_set_code    setCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

/* Find the position of a data item in a set */

unsigned int DATA_where_in_set (
 DATA_t_set_code    setCode,
 unsigned int  occurrence,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Find out how many data items in a set share the same portions of data */

unsigned int DATA_count_in_set (
 DATA_t_set_code    setCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

/* Is the set empty? */

Bool DATA_is_empty_set (DATA_t_set_code setCode);

/* Obtain the number of data items in a set */

unsigned int DATA_get_set_size (DATA_t_set_code setCode);

/* Perform the union of set 1 and set 2 and return the result in a third set */

DATA_t_set_code DATA_set_union (DATA_t_set_code set1, DATA_t_set_code set2);

/* Perform the intersection of set1 and set2 and return the result in a third set */

DATA_t_set_code DATA_set_intersection (DATA_t_set_code set1, DATA_t_set_code set2);

/* Work out s1-s2 and return the result in a third set */

DATA_t_set_code DATA_set_difference (DATA_t_set_code set1, DATA_t_set_code set2);

/* Work out s1 XOR s2 and return the result in a third set */

DATA_t_set_code DATA_set_symmetric_difference (DATA_t_set_code set1, DATA_t_set_code set2);

/* Find out if two sets are equal */

Bool DATA_are_equal_sets (DATA_t_set_code set1, DATA_t_set_code set2);

/* Find out if set2 is a subset of set2 */

Bool DATA_is_subset (DATA_t_set_code set1, DATA_t_set_code set2);

/* Remove all data from a set */

void DATA_clear_set (DATA_t_set_code setCode);

/* Delete a set and its data */

void DATA_destroy_set (DATA_t_set_code setCode);

/*--------------------*/
/* Methods for STACKS */
/*--------------------*/

/* Set up an empty stack and return its numeric code */

DATA_t_stack_code DATA_new_stack (size_t dataSize, unsigned int maxDataItems);

DATA_t_stack_code DATA_new_stack2 (
 size_t       dataSize,
 unsigned int maxDataItems,
 unsigned int initialBlockSize,
 unsigned int resizeLimit,
 unsigned int resizeFactor );

/* Copy part of an existing stack into a new one and return its numeric code */

DATA_t_stack_code DATA_copy_stack (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Push a data item onto the top of the stack */

void DATA_push_onto_stack (DATA_t_stack_code stackCode, void *p_dataItem);

/* Read the data item at the top of the stack without removing it */

void DATA_top_of_stack (DATA_t_stack_code stackCode, void *p_dataItem);

/* Pop the data item from the top of the stack and return the data */

void DATA_pop_from_stack (DATA_t_stack_code stackCode, void *p_dataItem);

/* Find out if a stack contains at least one data item that meets the specified criteria */

Bool DATA_is_on_stack (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Return the position of the Nth occurrence of a stack element that matches the specified criteria */

unsigned int DATA_where_in_stack (
 DATA_t_stack_code   stackCode, 
 unsigned int   occurrence,
 void          *p_dataPattern, 
 unsigned int   patternSubsets,
 ... );

/* Find out how many data items on a stack meet the specified criteria */

unsigned int DATA_count_on_stack (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Is the stack empty? */

Bool DATA_is_empty_stack (DATA_t_stack_code stackCode);

/* Obtain the number of data items in a stack */

unsigned int DATA_get_stack_height (DATA_t_stack_code stackCode);

/* Remove all data items in a stack */

void DATA_clear_stack (DATA_t_stack_code stackCode);

/* Delete a stack and its data */

void DATA_destroy_stack (DATA_t_stack_code stackCode);

/*--------------------*/
/* Methods for QUEUES */
/*--------------------*/

/* Set up an empty queue and return its numeric code */

DATA_t_queue_code DATA_new_queue (size_t dataSize, unsigned int maxDataItems);

DATA_t_queue_code DATA_new_queue2 (
 size_t       dataSize,
 unsigned int maxDataItems,
 unsigned int initialBlockSize,
 unsigned int resizeLimit,
 unsigned int resizeFactor );

/* Copy part of an existing queue into a new one and return its numeric code */

DATA_t_queue_code DATA_copy_queue (
 DATA_t_queue_code  queueCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

/* Add a data item to the rear of the queue */

void DATA_join_queue (DATA_t_queue_code queueCode, void *p_dataItem);

/* Read the data item at the front of the queue without removing it */

void DATA_front_of_queue (DATA_t_queue_code queueCode, void *p_dataItem);

/* Remove the data item from the front of the queue and return the data */

void DATA_leave_queue (DATA_t_queue_code queueCode, void *p_dataItem);

/* Find out if a queue contains at least one data item that meets the specified criteria */

Bool DATA_is_in_queue (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Return the position of the Nth occurrence of a queue element that matches the specified criteria */

unsigned int DATA_where_in_queue (
 DATA_t_queue_code   queueCode, 
 unsigned int   occurrence,
 void          *p_dataPattern, 
 unsigned int   patternSubsets,
 ... );

/* Find out how many data items in a queue meet the specified criteria */

unsigned int DATA_count_in_queue (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Is the queue empty? */

Bool DATA_is_empty_queue (DATA_t_queue_code queueCode);

/* Obtain the number of data items in a queue */

unsigned int DATA_get_queue_length (DATA_t_queue_code queueCode);

/* Remove all data items in a queue */

void DATA_clear_queue (DATA_t_queue_code queueCode);

/* Delete a queue and its data */

void DATA_destroy_queue (DATA_t_queue_code queueCode);

/*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION (not visible from other modules)
*
*---------------------------------------------------------------------
*/

/*
*-----------------------------------------------------------------------
* Error management
*-----------------------------------------------------------------------
*/

static Bool
  b_abort_on_error;

#define MAX_ERROR_MESSAGE_LENGTH  1000

static char
  dataErrorStr [MAX_ERROR_MESSAGE_LENGTH],
  auxErrorStr  [MAX_ERROR_MESSAGE_LENGTH];

/*
*-----------------------------------------------------------------------
* The various types of data strutures implemented
*-----------------------------------------------------------------------
*/

typedef enum {
 t_dataStructure_array,
 t_dataStructure_set,
 t_dataStructure_stack,
 t_dataStructure_queue
}
 t_dataStructure_type;

/* Strings with names of data structures, for error messaging */

static char
  *dataStructureName[] = {
    "array",
    "set",
    "stack",
    "queue" };

#define dataType2dataTypeStr(x)   (dataStructureName[x])

/* Default generic arrays parameters for the various dynamic data structures */

#define DEFAULT_ARRAY_BLOCKSIZE      10
#define DEFAULT_SET_BLOCKSIZE        10
#define DEFAULT_STACK_BLOCKSIZE      10
#define DEFAULT_QUEUE_BLOCKSIZE      10

#define DEFAULT_ARRAY_RESIZE_LIMIT   5
#define DEFAULT_SET_RESIZE_LIMIT     5
#define DEFAULT_STACK_RESIZE_LIMIT   5
#define DEFAULT_QUEUE_RESIZE_LIMIT   5

#define DEFAULT_ARRAY_RESIZE_FACTOR  4
#define DEFAULT_SET_RESIZE_FACTOR    4
#define DEFAULT_STACK_RESIZE_FACTOR  4
#define DEFAULT_QUEUE_RESIZE_FACTOR  4

static unsigned int
  default_blockSize[] = {
    DEFAULT_ARRAY_BLOCKSIZE,
    DEFAULT_SET_BLOCKSIZE,
    DEFAULT_STACK_BLOCKSIZE,
    DEFAULT_QUEUE_BLOCKSIZE },
  default_resizeLimit[] = {
    DEFAULT_ARRAY_RESIZE_LIMIT,
    DEFAULT_SET_RESIZE_LIMIT,
    DEFAULT_STACK_RESIZE_LIMIT,
    DEFAULT_QUEUE_RESIZE_LIMIT },
  default_resizeFactor[] = {
    DEFAULT_ARRAY_RESIZE_FACTOR,
    DEFAULT_SET_RESIZE_FACTOR,
    DEFAULT_STACK_RESIZE_FACTOR,
    DEFAULT_QUEUE_RESIZE_FACTOR };

/* Special positions in various data structures */

#define END_OF_SET       -1
#define TOP_OF_STACK     -2
#define BACK_OF_QUEUE    -3
#define FRONT_OF_QUEUE   -4

/*
*-----------------------------------------------------------------------
* Definitions for generic unidimensional arrays, used as the basis for
* all exported data structures. Everything is dinamically (re)allocated
*-----------------------------------------------------------------------
*/

/* Values for generic arrays info vector */

#define GENERIC_ARRAYS_INFO_VECTOR_INITIAL_BLOCK_SIZE   20
#define GENERIC_ARRAYS_INFO_VECTOR_BLOCK_RESIZE_LIMIT   10
#define GENERIC_ARRAYS_INFO_VECTOR_BLOCK_RESIZE_FACTOR  10

/* Default values for individual generic arrays; can be */
/* overridden at initialization of each generic array   */

#define GENERIC_ARRAY_INITIAL_BLOCK_SIZE   20
#define GENERIC_ARRAY_BLOCK_RESIZE_LIMIT   10
#define GENERIC_ARRAY_BLOCK_RESIZE_FACTOR  10

#define GENERIC_ARRAY_START_CODE            1
#define UNKNOWN_GENERIC_ARRAY_CODE          0

#define UNKNOWN_DATA_STRUCTURE_TYPE       255

#define UNKNOWN_PATTERN_MATCH_TYPE          0

typedef int
  t_genericArray_code;

typedef struct {
  Bool                 b_engaged;                  /* Is this generic array in use?                                                  */
  Bool                 b_extensible;               /* Is this generic array extensible or fixed size?                                */
  t_dataStructure_type dataType;                   /* Type of data structure the array is used for                                   */
  size_t               dataSize;                   /* Size in bytes of data items                                                    */
  size_t               currArraySize;              /* Memory currently allocated to the array                                        */
  unsigned int         totDataItems;               /* Sets, stacks & queues: number of data items currently stored in the array      */
                                                   /* Arrays: largest array index (dynamic array position) in use (1 onwards)        */
  unsigned int         maxDataItems;               /* Number of data items that fit in the array given its current memory allocation */
  unsigned int         absMaxDataItems;            /* Absolute maximum number of data items we are allowed to store in the array     */
  unsigned int         initialBlockSize;           /* Initial value of block size; needed for restoring it when clearing the array   */
  unsigned int         currBlockSize;              /* Current value of block size; may change as the array gets repeatedly extended  */
  unsigned int         totResizes_currBlockSize;   /* Number of times the array has been extended with current block size            */
  unsigned int         totResizes;                 /* Number of times the array has been extended since it was created               */
  unsigned int         resizeLimit;                /* Number of times the array can be resized before currBlockSize is increased     */
  unsigned int         resizeFactor;               /* Whenever resizeLimit is reached, currBlockSize *= resizeFactor                 */
  void                *pArray;                     /* Pointer to the first data item in the array                                    */
  unsigned int         patternSubsets;             /* Number of data pattern subsets that define set membership and pointer to list  */
  void                *pPatternSubsetParams;       /*   of pattern subset parameters; not used by ARRAYS, STACKS or QUEUES           */
}
  t_genericArray_info;

/* Memory currently allocated to genericArrays_infoVector */

static size_t
  genericArrays_infoVector_currSize;

/* Number of generic arrays currently in use */

static unsigned int
  tot_genericArrays;

/* Number of generic arrays that fit in genericArrays_infoVector */

static unsigned int
  max_genericArrays;

/* Extra memory is allocated to genericArrays_infoVector for this many new arrays */

static unsigned int
  genericArrays_infoVector_currBlockSize;

/* Number of times genericArrays_infoVector has been extended with current block size */

static unsigned int
  genericArrays_infoVector_totResizes_currBlockSize;

/* Number of times genericArrays_infoVector has been extended since it was created */

static unsigned int
  genericArrays_infoVector_totResizes;

/* Number of times the genericArrays_infoVector can be resized */
/* before genericArrays_infoVector_currBlockSize is increased  */

static unsigned int
  genericArrays_infoVector_resizeLimit;

/* Whenever resizeLimit is reached,                                                */
/* genericArrays_infoVector_currBlockSize *= genericArrays_infoVector_resizeFactor */

static unsigned int
  genericArrays_infoVector_resizeFactor;

/* Pointer to the first generic array info record */

static t_genericArray_info
  *genericArrays_infoVector;

/* Return a pointer to a generic array's info. Use of this macro */
/* must be preceded by a call to validate_genericArray_type()    */

#define pointer_to_GA_info(arrayCode) ((t_genericArray_info *) &(genericArrays_infoVector [arrayCode - GENERIC_ARRAY_START_CODE]))

/* Return a pointer to a data item in a generic array */

#define pointer_to_dataItem(arrayCode,dataItemPosition)  ((void *) ((char *) pointer_to_GA_info(arrayCode)->pArray + (dataItemPosition - 1) * pointer_to_GA_info(arrayCode)->dataSize))

/*
*---------------------------------------------------------------------
*   Function prototypes
*---------------------------------------------------------------------
*/

/* Several methods take variable length argument lists. These are implemented as wrappers whose only job is to  */
/* convert the variable arguments into lists of arguments and pass them on to their counterpart methods, who    */
/* then do all the actual work. The names of these counterpart methods end with _FAL for "Fixed Argument List". */

/* Array methods */
 
static DATA_t_array_code DATA_copy_array_FAL (
 DATA_t_array_code  arrayCode,
 void         *p_dataItem,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );
 
static void DATA_multiple_store_in_array_FAL (
 DATA_t_array_code  arrayCode,
 void         *p_dataItem,
 unsigned int  arrayBlocks,
 unsigned int *p_arrayBlockParams );
 
static Bool DATA_is_in_array_FAL (
 DATA_t_array_code  arrayCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static unsigned int DATA_where_in_array_FAL (
 DATA_t_array_code  arrayCode, 
 unsigned int  occurrence,
 void         *p_dataItem, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static unsigned int DATA_count_in_array_FAL (
 DATA_t_array_code  arrayCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

/* Set methods */

static void check_DATA_set_compatibility (
 DATA_t_set_code  set1, 
 DATA_t_set_code  set2, 
 char       *setOperation );
 
static void validate_two_DATA_set_operation (
 DATA_t_set_code  set1, 
 DATA_t_set_code  set2, 
 char       *setOperation ); 
 
static DATA_t_set_code DATA_copy_DATA_set_FAL (
 DATA_t_set_code    setCode,
 void         *p_dataItem,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );
 
static void DATA_remove_from_DATA_set_FAL (
 DATA_t_set_code    setCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static Bool DATA_is_in_DATA_set_FAL (
 DATA_t_set_code  setCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static unsigned int DATA_where_in_DATA_set_FAL (
 DATA_t_set_code    setCode, 
 unsigned int  occurrence,
 void         *p_dataItem, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static unsigned int DATA_count_in_DATA_set_FAL (
 DATA_t_set_code  setCode, 
 void         *p_dataItem,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

/*Stack methods */
 
static DATA_t_stack_code DATA_copy_stack_FAL (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static Bool is_on_stack_FAL (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static unsigned int where_on_stack_FAL (
 DATA_t_stack_code  stackCode, 
 unsigned int  occurrence,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static unsigned int DATA_count_on_stack_FAL (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

/* Queue methods */
 
static DATA_t_queue_code DATA_copy_queue_FAL (
 DATA_t_queue_code  queueCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );
 
static Bool DATA_is_in_queue_FAL (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static unsigned int DATA_where_in_queue_FAL (
 DATA_t_queue_code  queueCode, 
 unsigned int  occurrence,
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static unsigned int DATA_count_in_queue_FAL (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

/*                      */
/* Other static methods */
/*                      */

static int *convert_dataPattern_varArgs (
 t_genericArray_code  arrayCode,
 unsigned int         patternSubsets,
 va_list              varArgs,
 const char          *procName,
 char                *moduleName );
 
static unsigned int *convert_arrayPos_varArgs (
 t_genericArray_code  arrayCode,
 unsigned int         totPairs, 
 va_list              varArgs,
 const char          *procName,
 char                *moduleName );

static void DATA_clear_error_condition (void);

static void data_error (
 int         errorCode,
 char       *moduleName,
 const char *procName,
 char       *ERROR_auxErrorMsg );

static void check_range_int (
 int   value,
 int   min,
 int   max,
 char *varName,
 char *procName,
 char *moduleName );

static t_genericArray_code new_genericArray (
 t_dataStructure_type dataType,
 size_t               dataSize,
 unsigned int         absMaxDataItems,
 unsigned int         initialBlockSize,
 unsigned int         resizeLimit,
 unsigned int         resizeFactor );

static void store_in_genericArray (
 t_dataStructure_type dataType,
 t_genericArray_code  arrayCode,
 int                  dataItemPosition,
 void                *p_dataItem );

static void read_from_genericArray (
 t_dataStructure_type dataType,
 t_genericArray_code  arrayCode,
 int                  dataItemPosition,
 void                *p_dataItem );

static void remove_from_genericArray (
 t_dataStructure_type dataType,
 t_genericArray_code  arrayCode,
 int                  dataItemPosition );

static Bool data_items_match (
 void         *p_dataItem1,
 void         *p_dataItem2,
 size_t        dataItemSize,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams );

static unsigned int position_in_genericArray (
 t_genericArray_code arrayCode,
 unsigned int        occurrence,
 void               *p_dataPattern,
 unsigned int        patternSubsets,
 int                *p_patternSubsetParams );

static unsigned int occurrences_in_genericArray (
 t_genericArray_code arrayCode,
 void               *p_dataPattern,
 unsigned int        patternSubsets,
 int                *p_patternSubsetParams );

static void               *new_dummy_data_item      (t_genericArray_code arrayCode);
static unsigned int        elements_in_genericArray (t_genericArray_code arrayCode);
static t_genericArray_code copy_genericArray        (t_genericArray_code arrayCode);
static void                clear_genericArray       (t_genericArray_code arrayCode);
static void                destroy_genericArray     (t_genericArray_code arrayCode);

static unsigned int find_free_genericArray (void);

static t_dataStructure_type genericArrayCode2Type (t_genericArray_code arrayCode);

static void validate_genericArray_type (
 t_dataStructure_type expectedType,
 t_genericArray_code  arrayCode );

static void validate_existing_genericArray_position (
 t_dataStructure_type expectedType,
 t_genericArray_code  arrayCode,
 int                 *dataItemPosition );

static void validate_new_genericArray_position (
 t_dataStructure_type expectedType,
 t_genericArray_code  arrayCode,
 int                 *dataItemPosition );

 
/*
*-------------------------------------------------------------------------------------------------------
*   Take a variable length argument list specifying data pattern subsets and validate everything.
*   Return NULL as soon as an error is found. If there are no errors, then put the arguments on a
*   dynamically allocated list of integers and return a pointer to this list.
*   The caller is responsible for checking whether an error has occurred and also for freeing up the 
*   dynamically allocated memory.
*-------------------------------------------------------------------------------------------------------
*/

static int *convert_dataPattern_varArgs (
 t_genericArray_code  arrayCode,
 unsigned int         patternSubsets,
 va_list              varArgs,
 const char          *procName,
 char                *moduleName )
{
 unsigned int 
   iPatternSubset;
 int 
   patternSubsetStart,
   patternSubsetSize,
   totParams,
   *list,
   *p_param;
 DATA_t_patternMatchType
   patternMatchType;
 t_genericArray_info
   *p_GA_info;

 if (patternSubsets == 0)
   return (NULL);
 p_GA_info = pointer_to_GA_info (arrayCode);
 check_range_int (
   (int)    patternSubsets,
   (int)    1,
   (int)    p_GA_info->dataSize,
   (char *) "patternSubsets",
   (char *) procName,
   (char *) moduleName );
 if (has_data_structure_failed)
   return (NULL);
 if (patternSubsets == 1)
   totParams = (int) (2 * patternSubsets);
 else
   totParams = (int) (2 * patternSubsets) + 1;
 list = (int *) malloc (totParams * sizeof (int));
 if (list == NULL) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "malloc failed; unable to create list of %d variable parameters", totParams);
   data_error (0, moduleName, procName, auxErrorStr);
   return (NULL);
 }
 memset (list, 0, totParams * sizeof (int));
 p_param = list; 
 if (patternSubsets > 1) {
   patternMatchType = (DATA_t_patternMatchType) va_arg (varArgs, DATA_t_patternMatchType);
   check_range_int (
     (int)    patternMatchType,
     (int)    DATA_t_AND,
     (int)    DATA_t_XOR,
     (char *) "patternMatchType",
     (char *) procName,
     (char *) moduleName );
   if (has_data_structure_failed) {
     free (list);
     return (NULL);
   }
   *(p_param++) = (int) patternMatchType;
 }
 for (iPatternSubset = 0; iPatternSubset < patternSubsets; iPatternSubset++) {
   *(p_param++) = patternSubsetStart = (int) va_arg (varArgs, int);
   *(p_param++) = patternSubsetSize  = (int) va_arg (varArgs, int);
   if (patternSubsetStart < 0)
     patternSubsetStart = -patternSubsetStart;
   if (patternSubsetSize < 0)
     patternSubsetSize = -patternSubsetSize;
   check_range_int (
     (int)    patternSubsetStart,
     (int)    0,
     (int)    p_GA_info->dataSize,
     (char *) "patternSubsetStart",
     (char *) __func__,
     (char *) __FILE__ );
   if (has_data_structure_failed) {
     free (list);
     return (NULL);
   }
   check_range_int (
     (int)    patternSubsetSize,
     (int)    0,
     (int)    p_GA_info->dataSize,
     (char *) "patternSubsetSize",
     (char *) __func__,
     (char *) __FILE__ );
   if (has_data_structure_failed) {
     free (list);
     return (NULL);
   }
 }
 return (list);
}

/*
*-------------------------------------------------------------------------------------------------------
*   Take a variable length argument list specifying array positions in the form of pairs (from, to) and
*   validate everything. Return NULL as soon as an error is found. If there are no errors, then
*   put the arguments into a dynamically allocated list of integers and return a pointer to this list.
*   The caller is responsible for checking whether an error has occurred and also for freeing up the 
*   dynamically allocated memory.
*-------------------------------------------------------------------------------------------------------
*/

static unsigned int *convert_arrayPos_varArgs (
 t_genericArray_code  arrayCode,
 unsigned int         totPairs, 
 va_list              varArgs,
 const char          *procName,
 char                *moduleName )
{
 unsigned int 
   iPair,
   fromArrayPos,
   toArrayPos,
   totPos,
   *list,
   *p_pos;
 t_genericArray_info
   *p_GA_info;

 p_GA_info = pointer_to_GA_info (arrayCode);

 /* Nothing to do if the array position parameters are invalid */
 
 check_range_int (
   (int)    totPairs,
   (int)    1,
   (int)    p_GA_info->totDataItems,
   (char *) "totPairs",
   (char *) procName,
   (char *) moduleName );
 if (has_data_structure_failed)
   return (NULL);
 totPos = (unsigned int) 2 * totPairs;
 list = (unsigned int *) malloc (totPos * sizeof (unsigned int));
 if (list == NULL) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Unable to create list of %ud variable parameters (malloc failed)", totPos);
   data_error (0, moduleName, procName, auxErrorStr);
   return (NULL);
 }
 memset (list, 0, totPos * sizeof (int));
 for (iPair = 0, p_pos = list; iPair < totPairs; iPair++) {
   *(p_pos++) = fromArrayPos = (unsigned int) va_arg (varArgs, int);
   *(p_pos++) = toArrayPos   = (unsigned int) va_arg (varArgs, int);
   check_range_int (
     (int)    fromArrayPos,
     (int)    1,
     (int)    p_GA_info->totDataItems,
     (char *) "fromArrayPos",
     (char *) procName,
     (char *) moduleName );
   if (has_data_structure_failed) {
     free (list);
     return (NULL);
   }
   check_range_int (
     (int)    toArrayPos,
     (int)    fromArrayPos,
     (int)    p_GA_info->totDataItems,
     (char *) "toArrayPos",
     (char *) procName,
     (char *) moduleName );
   if (has_data_structure_failed) {
     free (list);
     return (NULL);
   }
 }
 return (list);
}

/*
*---------------------------------------------------------------------
*   Clear the error condition.
*   Must be called at the start of every interface method.
*---------------------------------------------------------------------
*/

void DATA_clear_error_condition (void)
{
 strcpy (dataErrorStr, "");
 strcpy (auxErrorStr, "");
 has_data_structure_failed = 0;
}

/*
*---------------------------------------------------------------------
*   Set up a detailed error message. If the global boolean
*   b_abort_on_error is TRUE then abort the program.
*---------------------------------------------------------------------
*/

void data_error (
 int         errorCode,
 char       *moduleName,
 const char *procName,
 char       *ERROR_auxErrorMsg )
{
 char
   auxStr [MAX_ERROR_MESSAGE_LENGTH];

 strcpy (auxStr, "");
 if (errorCode) {
   snprintf (auxStr, MAX_ERROR_MESSAGE_LENGTH, " %d", errorCode);
 }
 snprintf (dataErrorStr, MAX_ERROR_MESSAGE_LENGTH, "\n  Error%s in %s.c::%s()\n", auxStr, moduleName, procName);
 if (strcmp (ERROR_auxErrorMsg, "")) {
   snprintf (auxStr, MAX_ERROR_MESSAGE_LENGTH, "  %s\n", ERROR_auxErrorMsg);
   strcat (dataErrorStr, auxStr);
 }
 if (errorCode) {
   snprintf (auxStr, MAX_ERROR_MESSAGE_LENGTH, "  %s\n", strerror (errorCode));
   strcat (dataErrorStr, auxStr);
 }
 strcat (dataErrorStr, "\n");
 if (b_abort_on_error) {
   fprintf (stderr, "%s", dataErrorStr);
   exit (EXIT_FAILURE);
 }
 has_data_structure_failed = TRUE;
}

/*
-------------------------------------------------------------------------------
    Return a pointer to a string containing an error description
-------------------------------------------------------------------------------
*/

char *DATA_get_error (void)
{
 return ((char *) dataErrorStr);
}

/*
-------------------------------------------------------------------------------
    Ensure that an integer is within a valid range
-------------------------------------------------------------------------------
*/

void check_range_int (
 int   value,
 int   min,
 int   max,
 char *varName,
 char *procName,
 char *moduleName )
{
 if (value < min) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid %s (%d); must be >= %d", varName, value, min);
   data_error (0, moduleName, procName, auxErrorStr);
   return;
 }
 if (value > max) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid %s (%d); must be <= %d", varName, value, max);
   data_error (0, moduleName, procName, auxErrorStr);
   return;
 }
}

/*
-------------------------------------------------------------------------------
    Ensure that two sets are compatible for some set operation, e.g. union
-------------------------------------------------------------------------------
*/

static void check_DATA_set_compatibility (DATA_t_set_code set1, DATA_t_set_code set2, char *setOperation)
{
 t_genericArray_info
   *p_GA_info1,
   *p_GA_info2;
 unsigned int
   iPatternSubset;
 int
   patternSubsetStart1,
   patternSubsetStart2,
   patternSubsetSize1,
   patternSubsetSize2,
   *p_param1,
   *p_param2;
 DATA_t_patternMatchType
   patternMatchType1,
   patternMatchType2;

 /* Make sure the two sets have identical data pattern subset parameters */
 
 p_GA_info1 = pointer_to_GA_info (set1);
 p_GA_info2 = pointer_to_GA_info (set2);
 if (p_GA_info1->patternSubsets != p_GA_info2->patternSubsets) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "%s of sets %d and %d failed: number of data pattern subsets differ (%ud/%ud)", 
     setOperation, set1, set2, p_GA_info1->patternSubsets, p_GA_info2->patternSubsets );
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
 p_param1 = p_GA_info1->pPatternSubsetParams;
 p_param2 = p_GA_info2->pPatternSubsetParams;
 if (p_GA_info1->patternSubsets > 1) {
   patternMatchType1 = (DATA_t_patternMatchType) *(p_param1++);
   patternMatchType2 = (DATA_t_patternMatchType) *(p_param2++);
   if (patternMatchType1 != patternMatchType2) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "%s of sets %d and %d failed: pattern match type differ (%d/%d)", 
       setOperation, set1, set2, patternMatchType1, patternMatchType2 );
     data_error (0, __FILE__, __func__, auxErrorStr);
     return;
   }
 }
 for (iPatternSubset = 0; iPatternSubset < p_GA_info1->patternSubsets; iPatternSubset++) {
   patternSubsetStart1 = (int) *(p_param1++);
   patternSubsetStart2 = (int) *(p_param2++);
   if (patternSubsetStart1 != patternSubsetStart2) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "%s of sets %d and %d failed: start of pattern subset %ud differ (%d/%d)", 
       setOperation, set1, set2, iPatternSubset, patternSubsetStart1, patternSubsetStart2 );
     data_error (0, __FILE__, __func__, auxErrorStr);
     return;
   }
   patternSubsetSize1 = (int) *(p_param1++);
   patternSubsetSize2 = (int) *(p_param2++);
   if (patternSubsetSize1 != patternSubsetSize2) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "%s of sets %d and %d failed: size of pattern subset %ud differ (%d/%d)", 
       setOperation, set1, set2, iPatternSubset, patternSubsetSize1, patternSubsetSize2 );
     data_error (0, __FILE__, __func__, auxErrorStr);
     return;
   }
 }
}

/*
-------------------------------------------------------------------------------
    Ensure that two sets are compatible for some set operation, e.g. union
-------------------------------------------------------------------------------
*/

static void validate_two_DATA_set_operation (DATA_t_set_code set1, DATA_t_set_code set2, char *setOperation)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  set1 );
 if (has_data_structure_failed)
   return;
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  set2 );
 if (has_data_structure_failed)
   return;
 check_DATA_set_compatibility (set1, set2, setOperation);
}

/*
-------------------------------------------------------------------------------
    Find a free generic array and return its index in genericArrays_infoVector.
    If all arrays are engaged then create space for some more.
-------------------------------------------------------------------------------
*/

static unsigned int find_free_genericArray (void)
{
 size_t
   oldSize,
   newSize;
 unsigned int
   newMaximum,
   iArray,
   freeArrayIndex;

 /* If there is a free array then return its code */

 for (iArray = 0; iArray < max_genericArrays; iArray++)
   if (! genericArrays_infoVector[iArray].b_engaged)
     return ((unsigned int) iArray);

 /* All arrays are engaged, so allocate space for more arrays. */
 /* But first, check if the current block size is ok; we may   */
 /* need to increase it                                        */

 if (genericArrays_infoVector_totResizes_currBlockSize == genericArrays_infoVector_resizeLimit) {
   genericArrays_infoVector_currBlockSize *= genericArrays_infoVector_resizeFactor;
   genericArrays_infoVector_totResizes_currBlockSize = 0;
 }
 genericArrays_infoVector_totResizes++;
 genericArrays_infoVector_totResizes_currBlockSize++;

 newMaximum = max_genericArrays + genericArrays_infoVector_currBlockSize;
 oldSize = genericArrays_infoVector_currSize;
 newSize = newMaximum * sizeof (t_genericArray_info);
 genericArrays_infoVector = (t_genericArray_info *) realloc ((void *) genericArrays_infoVector, newSize);
 if (genericArrays_infoVector == NULL) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Unable to realloc genericArrays_infoVector");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((unsigned int) 0);
 }
 memset ((void *) &(genericArrays_infoVector[tot_genericArrays]), 0, newSize - oldSize);
 freeArrayIndex = max_genericArrays;
 max_genericArrays = newMaximum;
 for (iArray = freeArrayIndex; iArray < max_genericArrays; iArray++) {
   memset ((void *) &(genericArrays_infoVector[iArray]), 0, sizeof (t_genericArray_info));
   genericArrays_infoVector[iArray].pArray = NULL;
 }
 return (freeArrayIndex);
}

/*
---------------------------------------------------------------------
    Given a generic array code, return its type (set, stack etc)
---------------------------------------------------------------------
*/

static t_dataStructure_type genericArrayCode2Type (t_genericArray_code arrayCode)
{
 int
   arrayIndex;

 if (arrayCode < GENERIC_ARRAY_START_CODE) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array code (%d) must be >= %d", arrayCode, GENERIC_ARRAY_START_CODE);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((t_dataStructure_type) UNKNOWN_DATA_STRUCTURE_TYPE);
 }
 arrayIndex = arrayCode - GENERIC_ARRAY_START_CODE;
 if (arrayIndex >= (int) tot_genericArrays)  {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d does not exist", arrayCode);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((t_dataStructure_type) UNKNOWN_DATA_STRUCTURE_TYPE);
 }
 if (! genericArrays_infoVector[arrayIndex].b_engaged) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d is not in use", arrayCode);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((t_dataStructure_type) UNKNOWN_DATA_STRUCTURE_TYPE);
 }
 return (genericArrays_infoVector[arrayIndex].dataType);
}

/*
---------------------------------------------------------------------
    Make sure a given generic array is valid and in use
---------------------------------------------------------------------
*/

static void validate_genericArray_type (
 t_dataStructure_type expectedType,
 t_genericArray_code  arrayCode )
{
 t_dataStructure_type
   actualType;

 actualType = genericArrayCode2Type (arrayCode);
 if (actualType != expectedType) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d is of type %s; expected type %s", arrayCode, dataType2dataTypeStr (actualType), dataType2dataTypeStr (expectedType));
   data_error (0, __FILE__, __func__, auxErrorStr);
 }
}

/*
-------------------------------------------------------------------------
    Make sure data can be read from a given position in a generic array
     - Arrays and sets:
         1 <= element position <= totDataItems
     - Stacks:
         element position must be TOP_OF_STACK
     - Queues:
         element positiom must be either BACK_OF_QUEUE or FRONT_OF_QUEUE

    If dataItemPosition is a relative position then convert it to an
    absolute position and return it by reference
-------------------------------------------------------------------------
*/

static void validate_existing_genericArray_position (
 t_dataStructure_type expectedType,
 t_genericArray_code  arrayCode,
 int                 *dataItemPosition )
{
 int
   arrayIndex;
 int
   dataItemsInArray;
 t_dataStructure_type
   arrayType;
 t_genericArray_info
   *p_GA_info;

 if (arrayCode < GENERIC_ARRAY_START_CODE) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array code (%d) must be >= %d", arrayCode, GENERIC_ARRAY_START_CODE);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
 arrayIndex = arrayCode - GENERIC_ARRAY_START_CODE;
 if (arrayIndex >= (int) tot_genericArrays)  {
  snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d does not exist", arrayCode);
  data_error (0, __FILE__, __func__, auxErrorStr);
  return;
 }
 if (! genericArrays_infoVector[arrayIndex].b_engaged) {
  snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d is not in use", arrayCode);
  data_error (0, __FILE__, __func__, auxErrorStr);
  return;
 }
 arrayType = genericArrays_infoVector[arrayIndex].dataType;
 if (arrayType != expectedType) {
  snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d is of type %d; expected type %d", arrayCode, arrayType, expectedType);
  data_error (0, __FILE__, __func__, auxErrorStr);
  return;
 }
 p_GA_info = &(genericArrays_infoVector [arrayIndex]);
 dataItemsInArray = (int) p_GA_info->totDataItems;
 switch (arrayType) {
   case (t_dataStructure_array):
   case (t_dataStructure_set):
     if (*dataItemPosition < 1) {
       snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in %s %d: generic array underflow", *dataItemPosition, dataType2dataTypeStr (arrayType), arrayCode);
       data_error (0, __FILE__, __func__, auxErrorStr);
       return;
     }
     if (*dataItemPosition > dataItemsInArray) {
       snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in %s %d: generic array overflow (currently %d data items)", *dataItemPosition, dataType2dataTypeStr (arrayType), arrayCode, dataItemsInArray);
       data_error (0, __FILE__, __func__, auxErrorStr);
       return;
     }
     return;
   case (t_dataStructure_stack):
     if (*dataItemPosition != TOP_OF_STACK) {
       snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in stack %d: can only read the top of the stack", *dataItemPosition, arrayCode);
       data_error (0, __FILE__, __func__, auxErrorStr);
       return;
     }
     *dataItemPosition = dataItemsInArray;
     return;
   case (t_dataStructure_queue):
     if (*dataItemPosition == BACK_OF_QUEUE)
       *dataItemPosition = dataItemsInArray;
     else if (*dataItemPosition == FRONT_OF_QUEUE)
       *dataItemPosition = 1;
     else {
       snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in queue %d: can only read the front or back of the queue", *dataItemPosition, arrayCode);
       data_error (0, __FILE__, __func__, auxErrorStr);
       return;
     }
     return;
   default:
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d is of unknown type %d", arrayCode, arrayType);
     data_error (0, __FILE__, __func__, auxErrorStr);
     return;
 }
}

/*
-------------------------------------------------------------------------
    Make sure data can be stored at a given position in a generic array
     - Arrays:
         1 <= element position
         element position <= absMaxDataItems (if this has been provided)
     - Sets:
         element position must be END_OF_SET
     - Stacks:
         element position must be TOP_OF_STACK
     - Queues:
         element positiom must be BACK_OF_QUEUE

    If dataItemPosition is a relative position (as in BACK_OF_QUEUE)
    then convert it to an absolute position and return it by reference.
    Look out for these:
     - Sets: the new data should go right after the last set element
     - Stacks: the bottom of the stack is at array position 0
     - Queues: the front of the queue is at array position 0

    In all these cases, the new data must be stored in the first empty position
    at the far end of the generic array.
-------------------------------------------------------------------------
*/

static void validate_new_genericArray_position (
 t_dataStructure_type expectedType,
 t_genericArray_code  arrayCode,
 int                  *dataItemPosition )
{
 int
   arrayIndex;
 unsigned int
   maxDataItems,
   absMaxDataItems;
 t_dataStructure_type
   arrayType;
 t_genericArray_info
   *p_GA_info;

 if (arrayCode < GENERIC_ARRAY_START_CODE) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array code (%d) must be >= %d", arrayCode, GENERIC_ARRAY_START_CODE);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
 arrayIndex = (int) arrayCode - GENERIC_ARRAY_START_CODE;
 if (arrayIndex >= (int) tot_genericArrays)  {
  snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d does not exist", arrayCode);
  data_error (0, __FILE__, __func__, auxErrorStr);
  return;
 }
 if (! genericArrays_infoVector[arrayIndex].b_engaged) {
  snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d is not in use", arrayCode);
  data_error (0, __FILE__, __func__, auxErrorStr);
  return;
 }
 arrayType = genericArrays_infoVector[arrayIndex].dataType;
 if (arrayType != expectedType) {
  snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Generic array %d is a %s; expected a %s", arrayCode, dataType2dataTypeStr (arrayType), dataType2dataTypeStr (expectedType));
  data_error (0, __FILE__, __func__, auxErrorStr);
  return;
 }
 p_GA_info = &(genericArrays_infoVector [arrayIndex]);
 maxDataItems = p_GA_info->maxDataItems;
 if (arrayType == t_dataStructure_array) {
   if (*dataItemPosition < 1) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in array %d: generic array underflow", *dataItemPosition, arrayCode);
     data_error (0, __FILE__, __func__, auxErrorStr);
     return;
   }
   /* Is dataItemPosition within the array's current size? */
   if (*dataItemPosition <= (int) p_GA_info->maxDataItems)
     return;
   /* No, it's not, so can it be extended? Or maybe it's just empty and hasn't been allocated any memory yet */
   absMaxDataItems = p_GA_info->absMaxDataItems;
   if ((p_GA_info->totDataItems != 0) && (! p_GA_info->b_extensible)) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in fixed size array %d: generic array overflow (max %ud data items)", *dataItemPosition, arrayCode, absMaxDataItems);
     data_error (0, __FILE__, __func__, auxErrorStr);
     return;
   }
   /* Either it's empty or it has data in it but can be extended, so can it be extended indefinitely? */
   if (absMaxDataItems == 0)
     return;
   /* No, it cannot be extended indefinitely, so is the absolute maximum size large enough? */
   if (*dataItemPosition <= (int) absMaxDataItems)
     return;
   /* No luck; the array just can't get big enough */
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in array %d: generic array overflow (max %ud data items)", *dataItemPosition, arrayCode, absMaxDataItems);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
/*
 else if ((arrayType == t_dataStructure_set) && (*dataItemPosition != END_OF_SET)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in set %d: can only write to the end of the set", *dataItemPosition, arrayCode);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
*/
 else if ((arrayType == t_dataStructure_set) && (*dataItemPosition != END_OF_SET)) {
   if  (*dataItemPosition > (int) p_GA_info->totDataItems) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in set %d", *dataItemPosition, arrayCode);
     data_error (0, __FILE__, __func__, auxErrorStr);
   }
   return;
 }
 else if ((arrayType == t_dataStructure_stack) && (*dataItemPosition != TOP_OF_STACK)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in stack %d: can only write to the top of the stack", *dataItemPosition, arrayCode);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
 else if ((arrayType == t_dataStructure_queue) && (*dataItemPosition != BACK_OF_QUEUE)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in queue %d: can only write to the back of the queue", *dataItemPosition, arrayCode);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
 *dataItemPosition = (int) p_GA_info->totDataItems + 1;
 if ((p_GA_info->absMaxDataItems > 0) && (*dataItemPosition > (int) p_GA_info->absMaxDataItems)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid position %d in %s %d: generic array overflow (max %ud data items)", *dataItemPosition, dataType2dataTypeStr (arrayType), arrayCode, maxDataItems);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
}

/*
----------------------------------------------------------------------------
    Set up a generic array and return its index in genericArrays_infoVector
----------------------------------------------------------------------------
*/

static t_genericArray_code new_genericArray (
 t_dataStructure_type dataType,
 size_t               dataSize,
 unsigned int         absMaxDataItems,
 unsigned int         initialBlockSize,
 unsigned int         resizeLimit,
 unsigned int         resizeFactor )
{
 Bool
   b_extensible;
 unsigned int
   arrayIndex,
   totZeros;
 t_genericArray_code
   arrayCode;
 t_genericArray_info
   *p_GA_info;

 arrayIndex = find_free_genericArray();
 arrayCode = (t_genericArray_code) arrayIndex + GENERIC_ARRAY_START_CODE;

 /* Is this generic array meant to be extensible or fixed size?   */
 /* If all of initialBlockSize, resizeLimit and resizeFactor == 0 */
 /* then it will be flagged as fixed size. However, if only one   */
 /* or two of those three arguments are 0 then signal an error    */

 totZeros = 0;
 if (initialBlockSize == 0) totZeros ++;
 if (resizeLimit      == 0) totZeros ++;
 if (resizeFactor     == 0) totZeros ++;
 if ((totZeros == 1) || (totZeros == 2)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Invalid resizing parameters in %s %d: use either 3 or no zeros, not %ud", dataType2dataTypeStr (dataType), arrayCode, totZeros);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return (UNKNOWN_GENERIC_ARRAY_CODE);
 }
 b_extensible = (Bool) (absMaxDataItems == 0);
 if (b_extensible && (totZeros == 3)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Unable to decide whether %s %d is extensible of fixed size", dataType2dataTypeStr (dataType), arrayCode);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((t_genericArray_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }

 /* If the array is NOT extensible then we need to adjust initialBlockSize and resizeFactor, */
 /* otherwise the array will not be allocated any memory in store_in_genericArray()          */

 if ((! b_extensible) && (totZeros == 3)) {
   initialBlockSize = absMaxDataItems;
   resizeLimit = 1;
   resizeFactor = 1;
 }
 
 /* We may need to adjust initialBlockSize as we may have been */
 /* given a default value that clashes with absMaxDataItems    */

 if ((absMaxDataItems > 0) && (initialBlockSize > absMaxDataItems))
   initialBlockSize = absMaxDataItems;

 /* Now we know that the array may or may not be extensible, but if */
 /* it is NOT extensible then it must have an absolute maximum size */

 p_GA_info = &(genericArrays_infoVector [arrayIndex]);
 p_GA_info->b_engaged                = (Bool)                 TRUE;
 p_GA_info->b_extensible             = (Bool)                 b_extensible;
 p_GA_info->dataType                 = (t_dataStructure_type) dataType;
 p_GA_info->dataSize                 = (size_t)               dataSize;
 p_GA_info->currArraySize            = (size_t)               0;
 p_GA_info->totDataItems             = (unsigned int)         0;
 p_GA_info->maxDataItems             = (unsigned int)         0;
 p_GA_info->absMaxDataItems          = (unsigned int)         absMaxDataItems;
 p_GA_info->initialBlockSize         = (unsigned int)         initialBlockSize;
 p_GA_info->currBlockSize            = (unsigned int)         initialBlockSize;
 p_GA_info->totResizes_currBlockSize = (unsigned int)         0;
 p_GA_info->totResizes               = (unsigned int)         0;
 p_GA_info->resizeLimit              = (unsigned int)         resizeLimit;
 p_GA_info->resizeFactor             = (unsigned int)         resizeFactor;
 p_GA_info->patternSubsets           = (unsigned int)         0;
 if (p_GA_info->pArray != NULL) {
   free ((void *) p_GA_info->pArray);
   p_GA_info->pArray = NULL;
 }
 if (p_GA_info->pPatternSubsetParams != NULL) {
   free ((void *) p_GA_info->pPatternSubsetParams);
   p_GA_info->pPatternSubsetParams = NULL;
 }
 tot_genericArrays++;
 return (arrayCode);
}

/*
---------------------------------------------------------------------
    Create a duplicate of a generic array
---------------------------------------------------------------------
*/

static t_genericArray_code copy_genericArray (t_genericArray_code arrayCode)
{
 unsigned int
   copyIndex;
 t_genericArray_code
   copyCode;
 t_genericArray_info
   *p_GA_infoOrig,
   *p_GA_infoCopy;

 /* Find a free generic array */

 copyIndex = find_free_genericArray();
 copyCode = (t_genericArray_code) copyIndex + GENERIC_ARRAY_START_CODE;

 /* Set up pointers to info fields of both original and copy arrays */

 p_GA_infoOrig = pointer_to_GA_info (arrayCode);
 p_GA_infoCopy = pointer_to_GA_info (copyCode);

 /* Copy generic array info fields */

 memcpy ((void *) p_GA_infoCopy, (void *) p_GA_infoOrig, sizeof (t_genericArray_info));

 /* Allocate memory for generic array data and copy the data across */

 if (p_GA_infoOrig->pArray != NULL) {
   p_GA_infoCopy->pArray = (void *) malloc (p_GA_infoOrig->currArraySize);
   if (p_GA_infoCopy->pArray == NULL) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Unable to copy %s %d to %d: error allocating %d bytes to generic array",
       dataType2dataTypeStr (p_GA_infoOrig->dataType),
       arrayCode, copyCode, (int) p_GA_infoOrig->currArraySize );
     data_error (0, __FILE__, __func__, auxErrorStr);
     return ((t_genericArray_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
   memcpy ((void *) p_GA_infoCopy->pArray, (void *) p_GA_infoOrig->pArray, p_GA_infoOrig->currArraySize);
 }
 
 /* Allocate memory for generic array data data pattern subsets and copy the data across */

 if (p_GA_infoOrig->pPatternSubsetParams != NULL) {
   p_GA_infoCopy->pPatternSubsetParams = (void *) malloc (p_GA_infoOrig->currArraySize);
   if (p_GA_infoCopy->pPatternSubsetParams == NULL) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Unable to copy %s %d to %d: error allocating %d bytes to generic array",
       dataType2dataTypeStr (p_GA_infoOrig->dataType),
       arrayCode, copyCode, (int) p_GA_infoOrig->currArraySize );
     data_error (0, __FILE__, __func__, auxErrorStr);
     return ((t_genericArray_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
   memcpy ((void *) p_GA_infoCopy->pPatternSubsetParams, (void *) p_GA_infoOrig->pPatternSubsetParams, p_GA_infoOrig->currArraySize);
 }
 
 tot_genericArrays++;
 return (copyCode);
}

/*
---------------------------------------------------------------------
    Clear the contents of a generic array
---------------------------------------------------------------------
*/

static void clear_genericArray (t_genericArray_code arrayCode)
{
 t_genericArray_info
   *p_GA_info;

 p_GA_info = pointer_to_GA_info (arrayCode);

 /* Reset generic array */

 p_GA_info->currArraySize            = (size_t)       0;
 p_GA_info->totDataItems             = (unsigned int) 0;
 p_GA_info->maxDataItems             = (unsigned int) 0;
 p_GA_info->currBlockSize            = (unsigned int) p_GA_info->initialBlockSize;
 p_GA_info->totResizes_currBlockSize = (unsigned int) 0;
 p_GA_info->totResizes               = (unsigned int) 0;

 if (p_GA_info->pArray != NULL) {
   free ((void *) p_GA_info->pArray);
   p_GA_info->pArray = NULL;
 }
}

/*
---------------------------------------------------------------------
    Dismiss (free) a generic array and make it available for re-use
---------------------------------------------------------------------
*/

static void destroy_genericArray (t_genericArray_code arrayCode)
{
 t_genericArray_info
   *p_GA_info;

 p_GA_info = pointer_to_GA_info (arrayCode);
 p_GA_info->b_engaged                = (Bool)                 FALSE;
 p_GA_info->b_extensible             = (Bool)                 FALSE;
 p_GA_info->dataType                 = (t_dataStructure_type) UNKNOWN_DATA_STRUCTURE_TYPE;
 p_GA_info->dataSize                 = (size_t)               0;
 p_GA_info->currArraySize            = (size_t)               0;
 p_GA_info->totDataItems             = (unsigned int)         0;
 p_GA_info->maxDataItems             = (unsigned int)         0;
 p_GA_info->absMaxDataItems          = (unsigned int)         0;
 p_GA_info->initialBlockSize         = (unsigned int)         0;
 p_GA_info->currBlockSize            = (unsigned int)         0;
 p_GA_info->totResizes_currBlockSize = (unsigned int)         0;
 p_GA_info->totResizes               = (unsigned int)         0;
 p_GA_info->resizeLimit              = (unsigned int)         0;
 p_GA_info->resizeFactor             = (unsigned int)         0;
 p_GA_info->patternSubsets           = (unsigned int)         0;
 if (p_GA_info->pArray != NULL) {
   free ((void *) p_GA_info->pArray);
   p_GA_info->pArray = NULL;
 }
 if (p_GA_info->pPatternSubsetParams != NULL) {
   free ((void *) p_GA_info->pPatternSubsetParams);
   p_GA_info->pPatternSubsetParams = NULL;
 }
}

/*
---------------------------------------------------------------------
    Return a pointer to a newly created blank data item
---------------------------------------------------------------------
*/

static void *new_dummy_data_item (t_genericArray_code arrayCode)
{
 t_genericArray_info
   *p_GA_info;
 void
   *p_dataItem;

 p_GA_info = pointer_to_GA_info (arrayCode);
 p_dataItem = malloc (p_GA_info->dataSize);
 if (p_dataItem != NULL)
   memset (p_dataItem, 0, p_GA_info->dataSize);
 return (p_dataItem);
}

/*
---------------------------------------------------------------------
    Return the number of elements stored in a generic array
---------------------------------------------------------------------
*/

static unsigned int elements_in_genericArray (t_genericArray_code arrayCode)
{
 t_genericArray_info
   *p_GA_info;

 p_GA_info = pointer_to_GA_info (arrayCode);
 return (p_GA_info->totDataItems);
}

/*
---------------------------------------------------------------------
    Return a boolean indicating whether two data items match,
    according with the specified criteria
---------------------------------------------------------------------
*/

static Bool data_items_match (
 void         *p_dataItem1,
 void         *p_dataItem2,
 size_t        dataItemSize,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   iPatternSubset,
   partial_matches;
 int
   patternSubsetStart,
   patternSubsetSize,
   *p_param;
 DATA_t_patternMatchType
   patternMatchType = UNKNOWN_PATTERN_MATCH_TYPE;
 Bool
   b_match,
   b_want_negative_match;

 if ((patternSubsets == 0) && (p_patternSubsetParams != NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Did not expect list of data pattern parameters as patternSubsets = 0");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((Bool) FALSE);
 }
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Expected list of data pattern parameters as patternSubsets > 0 (%ud)", patternSubsets);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((Bool) FALSE);
 }
 b_match = FALSE;
 partial_matches = 0;
 if (patternSubsets == 0)
   b_match = (Bool) (! memcmp (p_dataItem1, p_dataItem2, dataItemSize));
 else {
   p_param = p_patternSubsetParams;
   if (patternSubsets > 1)
     patternMatchType = (DATA_t_patternMatchType) *(p_param++);
   for (iPatternSubset = 0; iPatternSubset < patternSubsets; iPatternSubset++) {
     patternSubsetStart  = (int) *(p_param++);
     patternSubsetSize = (int) *(p_param++);
     b_want_negative_match = FALSE;
     if (patternSubsetStart < 0) {
       patternSubsetStart = -patternSubsetStart;
       b_want_negative_match = TRUE;
     }
     if (patternSubsetSize < 0) {
       patternSubsetSize = -patternSubsetSize;
       b_want_negative_match = TRUE;
     }
     b_match = (Bool) (! memcmp ((char *) p_dataItem1 + patternSubsetStart, (char *) p_dataItem2 + patternSubsetStart, (size_t) patternSubsetSize));
     if (b_want_negative_match)   /* Implicit evaluation of */
       b_match = (Bool) (! b_match);       /* NOT boolean operator   */
     if (b_match)
       partial_matches++;
     if ((patternSubsets > 1) && (patternMatchType == DATA_t_AND) && (! b_match))             /* Short-circuit boolean evaluation:  */
         break;                                                                          /* it may be pointless to try further */
     if ((patternSubsets > 1) && (patternMatchType == DATA_t_XOR) && (partial_matches > 1))   /* partial matches on this particular */
         break;                                                                          /* data item                          */
   }
 }
 if (! b_match)
   return ((Bool) FALSE);
 if ((patternSubsets > 1) && (patternMatchType == DATA_t_AND) && (partial_matches != patternSubsets))
   return ((Bool) FALSE);
 if ((patternSubsets > 1) && (patternMatchType == DATA_t_OR) && (partial_matches == 0))
   return ((Bool) FALSE);
 if ((patternSubsets > 1) && (patternMatchType == DATA_t_XOR) && (partial_matches != 1))
   return ((Bool) FALSE);
 return ((Bool) TRUE);
}

/*
-------------------------------------------------------------------------------------------------------------
    Look for the Nth occurrence of a data item in a generic array and return its position in the array.
    A positive match will depend on the criteria provided in list of data pattern parameters.
    Data item position (the value returned by this funcion) starts from 1.
    Return 0 if the data looked for is not stored in the generic array.
-------------------------------------------------------------------------------------------------------------
*/

static unsigned int position_in_genericArray (
 t_genericArray_code arrayCode,
 unsigned int        occurrence,
 void               *p_dataPattern,
 unsigned int        patternSubsets,
 int                *p_patternSubsetParams )
{
 unsigned int
   iArrayPos,
   lower_iArrayPos,
   heigher_iArrayPos,
   first_iArrayPos,
   totFound;
 t_genericArray_info
   *p_GA_info;
 char
   *p_arrayPos,
   *lower_p_arrayPos,
   *heigher_p_arrayPos,
   *first_p_arrayPos;
 int
   incr_p_arrayPos;
 Bool
   b_is_a_stack,
   b_match;
 
 /* Nothing to do if the generic array is empty */
 
 p_GA_info = pointer_to_GA_info (arrayCode);
 if (p_GA_info->totDataItems == 0)
   return ((unsigned int) 0);

 /* Nothing to do if the occurrence seeked is invalid */

 check_range_int (
   (int)    occurrence,
   (int)    1,
   (int)    p_GA_info->totDataItems,
   (char *) "occurrence",
   (char *) __func__,
   (char *) __FILE__ );
 if (has_data_structure_failed)
   return ((unsigned int) 0);

 /* The consistency of data pattern parameters has already */
 /* been verified so just start looking for the data       */

 /* If we are looking for data on a stack, the search must be    */
 /* done backwards, i.e. from the top to the bottom of the stack */
 
 totFound           = 0;
 lower_iArrayPos    = 0;
 heigher_iArrayPos  = p_GA_info->totDataItems - 1;
 lower_p_arrayPos   = (char *) p_GA_info->pArray;
 heigher_p_arrayPos = (char *) ((char *) p_GA_info->pArray + (p_GA_info->totDataItems - 1) * p_GA_info->dataSize);
 incr_p_arrayPos    = (int) p_GA_info->dataSize;
 b_is_a_stack = (Bool) (genericArrayCode2Type (arrayCode) == t_dataStructure_stack);
 if (b_is_a_stack) {
   first_iArrayPos  = heigher_iArrayPos;
   first_p_arrayPos = heigher_p_arrayPos;
 }
 else {
   first_iArrayPos  = lower_iArrayPos;
   first_p_arrayPos = lower_p_arrayPos;
 }
 for (iArrayPos = first_iArrayPos, p_arrayPos = first_p_arrayPos; (lower_iArrayPos <= iArrayPos) && (iArrayPos <= heigher_iArrayPos); ) {
    b_match = data_items_match (
      (void *)       p_dataPattern,
      (void *)       p_arrayPos,
      (size_t)       p_GA_info->dataSize,
      (unsigned int) patternSubsets, 
      (int *)        p_patternSubsetParams );
   if (b_is_a_stack) {
     iArrayPos--;
     p_arrayPos -= incr_p_arrayPos;
   }
   else {
     iArrayPos++;
     p_arrayPos += incr_p_arrayPos;
   }
   if (! b_match)
     continue;
     
   /* At this point we have a positive match, but we don't want just the first match! */
   
   if (++totFound == occurrence)                     
     return (b_is_a_stack ? p_GA_info->totDataItems - iArrayPos - 1 : iArrayPos);   
 }
 
 /* Could not find the Nth occurrence of the data item in the array */

 return ((unsigned int) 0);
}

/*
-------------------------------------------------------------------------------------------------------------
    Count the items in a generic array that meet the criteria provided in list of data pattern parameters.
-------------------------------------------------------------------------------------------------------------
*/

static unsigned int occurrences_in_genericArray (
 t_genericArray_code arrayCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   iArrayPos,
   totFound;
 t_genericArray_info
   *p_GA_info;
 char
   *p_arrayPos;
 Bool
   b_match;
 
 /* Nothing to do if the generic array is empty */
 
 p_GA_info = pointer_to_GA_info (arrayCode);
 if (p_GA_info->totDataItems == 0)
   return ((unsigned int) 0);

 /* The consistency of data pattern parameters has already */
 /* been verified so just start looking for the data       */

 for (iArrayPos = totFound = 0, p_arrayPos = (char *) p_GA_info->pArray;
        iArrayPos < p_GA_info->totDataItems;
          iArrayPos++, p_arrayPos += p_GA_info->dataSize) {
    b_match = data_items_match (
      (void *)       p_dataPattern,
      (void *)       p_arrayPos,
      (size_t)       p_GA_info->dataSize,
      (unsigned int) patternSubsets, 
      (int *)        p_patternSubsetParams );
   if (! b_match)
     continue;
   totFound++;     /* At this point we have a positive match */
 }
 return (totFound);
}

/*
------------------------------------------------------------------------
    Return by reference the data in a given position of a generic array
------------------------------------------------------------------------
*/

static void read_from_genericArray (
 t_dataStructure_type dataType,
 t_genericArray_code  arrayCode,
 int                  dataItemPosition,
 void                *p_dataItem )
{
 t_genericArray_info
   *p_GA_info;
 char
   *p_arrayPos;

 /* Make sure dataItemPosition is compatible with the type of data structure */
 /* and that it can be read from a given position in the generic array.      */
 /* Also, convert a relative data item position to an absolute one.          */

 validate_existing_genericArray_position (dataType, arrayCode, &dataItemPosition);

 /* Set up a pointer to the generic array header */

 p_GA_info = pointer_to_GA_info (arrayCode);

 /* Get the data requested */

 p_arrayPos = (char *) p_GA_info->pArray + (dataItemPosition - 1) * p_GA_info->dataSize;
 memcpy (p_dataItem, p_arrayPos, p_GA_info->dataSize);
}

/*
---------------------------------------------------------------------
    Store data in a generic array
---------------------------------------------------------------------
*/

static void store_in_genericArray (
 t_dataStructure_type dataType,
 t_genericArray_code  arrayCode,
 int                  dataItemPosition,
 void                *p_dataItem )
{
 size_t
   newArraySize;
 unsigned int
   newMaxDataItems;
 t_genericArray_info
   *p_GA_info;
 char
   *p_arrayPos;

 /* Ensure that...                                                  */
 /*  - dataItemPosition is compatible with the data structure type  */
 /*  - the generic array can accommodate dataItemPosition           */
 /* Also, convert a relative data item position to an absolute one. */

 validate_new_genericArray_position (dataType, arrayCode, &dataItemPosition);

 /* Set up a pointer to the generic array header */

 p_GA_info = pointer_to_GA_info (arrayCode);

 /* If the generic array is not large enough to accommodate the new data      */
 /* then try to extend it... but is it extensible or is it fixed size?        */
 /*                                                                           */
 /* Whether or not the generic array is extensible, we know that EVENTUALLY   */
 /* it should be able to accommodate dataItemPosition; the call to            */
 /* validate_new_genericArray_position() has ensured that much. We also know  */
 /* that CURRENTLY it isn't large enough, as dataItemPosition > maxDataItems. */
 /* So, the array must be extensible (it may or may not have an absolute      */
 /* maximum size)... UNLESS the array is still empty (no memory has been      */
 /* allocated to it yet) and this is the first call to this method, in which  */
 /* case totDataItems == maxDataItems == 0.                                   */
 /*                                                                           */
 /* IN SUMMARY: we know that either one of the following conditions is true:  */
 /*  - the array is empty but will be big enough when its memory is allocated */
 /*  - the array is not empty but it is extensible, either indefinitely or    */
 /*      or sufficiently to accommodate the new data                          */
 /*                                                                           */
 /* OK, we know that we need to (and can) extend the array at least once      */
 /* but how many times do we need to go through an array extension loop?      */
 /*  - Sets, stacks, queues: only once, as dataItemPosition==currDataItems+1  */
 /*  - Arrays: as many times as necessary, as dataItemPosition could be       */
 /*      anything                                                             */
 /*                                                                           */
 /* Therefore it is safe to keep extending the array until it's big enough    */

 while (dataItemPosition > (int) p_GA_info->maxDataItems) {

   /* Before extending the generic array we may need to increase the block size */

   if (p_GA_info->totResizes_currBlockSize >= p_GA_info->resizeLimit) {
     p_GA_info->currBlockSize *= p_GA_info->resizeFactor;
     p_GA_info->totResizes_currBlockSize = 0;
   }

   /* The block size has the correct value now, so extend the generic array */

   p_GA_info->totResizes++;
   p_GA_info->totResizes_currBlockSize++;
   newMaxDataItems = p_GA_info->maxDataItems + p_GA_info->currBlockSize;
   newArraySize = (size_t) newMaxDataItems * p_GA_info->dataSize;
   if (p_GA_info->pArray == NULL)
     p_GA_info->pArray = (void *) malloc (newArraySize);
   else
     p_GA_info->pArray = (void *) realloc ((void *) p_GA_info->pArray, newArraySize);
   if (p_GA_info->pArray == NULL) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "%s %d: unable to allocate %d bytes to generic array", dataType2dataTypeStr (dataType), arrayCode, (int) newArraySize);
     data_error (0, __FILE__, __func__, auxErrorStr);
     return;
   }

   /* Clear the extra memory just added */

   memset ((char *) p_GA_info->pArray + (p_GA_info->totDataItems * p_GA_info->dataSize), 0, (size_t) (newMaxDataItems - p_GA_info->maxDataItems));

   p_GA_info->currArraySize = newArraySize;
   p_GA_info->maxDataItems = newMaxDataItems;
 }

 /* Update the array's totDataItems counter */

 if ( (dataType == t_dataStructure_queue) || 
      (dataType == t_dataStructure_stack) || 
      (dataItemPosition > (int) p_GA_info->totDataItems) ) 
   p_GA_info->totDataItems = (unsigned int) dataItemPosition;

 /* Store the data */

 p_arrayPos = (char *) p_GA_info->pArray + (dataItemPosition - 1) * p_GA_info->dataSize;
 memcpy (p_arrayPos, p_dataItem, p_GA_info->dataSize);
}

/*
---------------------------------------------------------------------
    Remove data from a generic array
---------------------------------------------------------------------
*/

static void remove_from_genericArray (
 t_dataStructure_type dataType,
 t_genericArray_code  arrayCode,
 int                  dataItemPosition )
{
 t_genericArray_info
   *p_GA_info;

 /* Make sure dataItemPosition is compatible with the type of data structure */
 /* and that it can be read from a given position in the generic array.      */
 /* Also, convert a relative data item position to an absolute one.          */

 validate_existing_genericArray_position (dataType, arrayCode, &dataItemPosition);

 /* Set up a pointer to the generic array header */

 p_GA_info = pointer_to_GA_info (arrayCode);

 /* The generic array could be empty already!!! */

 if (p_GA_info->totDataItems == 0) {
     snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "%s %d underflow: unable to remove data as generic array is empty", dataType2dataTypeStr (dataType), arrayCode);
     data_error (0, __FILE__, __func__, auxErrorStr);
     return;
 }

 /* Move data down the array to close the gap where the removed data is */

 memmove ((void *)       ((char *) p_GA_info->pArray + (dataItemPosition - 1) * p_GA_info->dataSize),
          (const void *) ((char *) p_GA_info->pArray + (dataItemPosition      * p_GA_info->dataSize)),
          (size_t)       (p_GA_info->totDataItems - dataItemPosition) * p_GA_info->dataSize );

 /* Clear the memory space at the far end of the array that got vacated by this data move */

 memset ((void *) ((char *) p_GA_info->pArray + (p_GA_info->totDataItems - 1) * p_GA_info->dataSize),
         (int)    0,
         (size_t) p_GA_info->dataSize);

 /* Update the array's totDataItems counter */

 p_GA_info->totDataItems--;
}

/*
*---------------------------------------------------------------------
*
*   METHODS FOR ARRAYS
*
*---------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------
    Set up an empty array and return its numeric code
---------------------------------------------------------------------------
*/

/* Version 1 */

DATA_t_array_code DATA_new_array (size_t dataSize, unsigned int maxDataItems)
{
 t_genericArray_code
   genericArray_code;

 DATA_clear_error_condition();
 genericArray_code = new_genericArray (
  (t_dataStructure_type) t_dataStructure_array,
  (size_t)               dataSize,
  (unsigned int)         maxDataItems,
  (unsigned int)         default_blockSize    [t_dataStructure_array],
  (unsigned int)         default_resizeLimit  [t_dataStructure_array],
  (unsigned int)         default_resizeFactor [t_dataStructure_array] );
 if (has_data_structure_failed)
   return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
 return ((DATA_t_array_code) genericArray_code);
}

/* Verson 2 */

DATA_t_array_code DATA_new_array2 (
 size_t       dataSize,
 unsigned int maxDataItems,
 unsigned int initialBlockSize,
 unsigned int resizeLimit,
 unsigned int resizeFactor )
{
 t_genericArray_code
   genericArray_code;

 DATA_clear_error_condition();
 genericArray_code = new_genericArray (
  (t_dataStructure_type) t_dataStructure_array,
  (size_t)               dataSize,
  (unsigned int)         maxDataItems,
  (unsigned int)         initialBlockSize,
  (unsigned int)         resizeLimit,
  (unsigned int)         resizeFactor );
 if (has_data_structure_failed)
   return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
 return ((DATA_t_array_code) genericArray_code);
}

/*
--------------------------------------------------------------------------------
    Copy part of an existing array into a new one and return its numeric code.
    Only data items that match the provided data pattern will be copied.
    To copy the whole array, call DATA_copy_array (arrayCode, NULL, 0);
--------------------------------------------------------------------------------
*/

/* The main method does the real work */

#ifdef ENSURE_NO_DATA_STRUCTURE_ERROR
#undef ENSURE_NO_DATA_STRUCTURE_ERROR
#endif
#define ENSURE_NO_DATA_STRUCTURE_ERROR                          \
   if (has_data_structure_failed) {                             \
     DATA_destroy_array (newArray);                             \
     free (p_dataItem);                                         \
     return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);   \
   }

DATA_t_array_code DATA_copy_array_FAL (
 DATA_t_array_code  arrayCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 DATA_t_array_code
   newArray;
 unsigned int
   iSourcePos,
   iDestPos,
   sourceSize;
 Bool
   b_match;
 void
   *p_dataItem;
 t_genericArray_info
   *p_GA_info;
  
 if ((patternSubsets == 0) && (p_dataPattern != NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Did not expect a data pattern as patternSubsets = 0");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if ((patternSubsets > 0) && (p_dataPattern == NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Expected a data pattern as patternSubsets > 0 (%ud)", patternSubsets);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if (p_dataPattern == NULL) {
   newArray = (DATA_t_array_code) copy_genericArray ((t_genericArray_code) arrayCode);
   if (has_data_structure_failed) {
     DATA_destroy_array (newArray);
     return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
   else
     return (newArray);
 }
 p_dataItem = new_dummy_data_item (arrayCode);
 if (p_dataItem == NULL)
   return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
 p_GA_info = pointer_to_GA_info (arrayCode);
 newArray = (DATA_t_array_code) new_genericArray (
   (t_genericArray_code)   arrayCode,
   (size_t)                p_GA_info->dataSize,
   (unsigned int)          p_GA_info->absMaxDataItems,
   (unsigned int)          p_GA_info->initialBlockSize,
   (unsigned int)          p_GA_info->resizeLimit,
   (unsigned int)          p_GA_info->resizeFactor );
 ENSURE_NO_DATA_STRUCTURE_ERROR
 sourceSize = DATA_get_array_size (arrayCode);
 ENSURE_NO_DATA_STRUCTURE_ERROR
 for (iSourcePos = 1, iDestPos = 0; iSourcePos <= sourceSize; iSourcePos++) {
   DATA_read_from_array (arrayCode, iSourcePos, p_dataItem);
   ENSURE_NO_DATA_STRUCTURE_ERROR
   b_match = data_items_match (p_dataItem, p_dataPattern, p_GA_info->dataSize, patternSubsets, p_patternSubsetParams);
   ENSURE_NO_DATA_STRUCTURE_ERROR
   if (b_match)
     DATA_store_in_array (newArray, ++iDestPos, p_dataItem);
   ENSURE_NO_DATA_STRUCTURE_ERROR
 }
 free (p_dataItem);
 return (newArray);
}

/* The wrapper method does some of the necessary checking and builds the list of data pattern parameters */

DATA_t_array_code DATA_copy_array (
 DATA_t_array_code  arrayCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 DATA_t_array_code
   newArray;
 t_genericArray_info
   *p_GA_info;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
 p_GA_info = pointer_to_GA_info (arrayCode);
 check_range_int (
   (int)    patternSubsets,
   (int)    0,
   (int)    p_GA_info->dataSize,
   (char *) "patternSubsets",
   (char *) __func__,
   (char *) __FILE__ );
 if (has_data_structure_failed)
   return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
 if ((patternSubsets == 0) && (p_dataPattern != NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Did not expect a data pattern as patternSubsets = 0");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if ((patternSubsets > 0) && (p_dataPattern == NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Expected a data pattern as patternSubsets > 0 (%ud)", patternSubsets);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_array_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (arrayCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 newArray = DATA_copy_array_FAL (arrayCode, p_dataPattern, patternSubsets, p_patternSubsetParams);
 free (p_patternSubsetParams);
 return (newArray);
}

/*
---------------------------------------------------------------------
    Store data in a array
---------------------------------------------------------------------
*/

void DATA_store_in_array (
 DATA_t_array_code  arrayCode,
 unsigned int  dataItemPosition,
 void         *p_dataItem )
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return;
 store_in_genericArray (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode,
   (int)                  dataItemPosition,
   (void *)               p_dataItem );
}

/*
---------------------------------------------------------------------------------
    Store multiple copies of a data item in specified positions of a array.
    Target positions are passed to the function in a variable length argument 
    list, in pairs of integers (from, to) specifying ranges of array positions.
---------------------------------------------------------------------------------
*/

/* The main method does the real work */

static void DATA_multiple_store_in_array_FAL (
 DATA_t_array_code  arrayCode,
 void         *p_dataItem,
 unsigned int  arrayBlocks,
 unsigned int *p_arrayBlockParams )
{
 unsigned int
   iArrayBlock,
   iArrayPos,
   fromArrayPos,
   toArrayPos,
   *p_param;

 for (iArrayBlock = 0, p_param = p_arrayBlockParams; iArrayBlock < arrayBlocks; iArrayBlock++) {
   fromArrayPos = *(p_param++);
   toArrayPos   = *(p_param++);
   for (iArrayPos = fromArrayPos; iArrayPos <= toArrayPos; iArrayPos++) {
     DATA_clear_error_condition();
     store_in_genericArray (
       (t_dataStructure_type) t_dataStructure_array,
       (t_genericArray_code)  arrayCode,
       (int)                  iArrayPos,
       (void *)               p_dataItem );
     if (has_data_structure_failed)
       return;
   }
 }
}

/* The wrapper method does all the necessary checking */

void DATA_multiple_store_in_array (
 DATA_t_array_code  arrayCode,
 void         *p_dataItem,
 unsigned int  arrayBlocks,
 ... )
{
 va_list
   varArgs;
 unsigned int
   *p_arrayBlockParams;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return;

 /* The function's variable arguments will be checked by convert_arrayPos_varArgs() */
 /* so there is no need to do this now; just store the data                         */
 
 va_start (varArgs, arrayBlocks);
 p_arrayBlockParams = convert_arrayPos_varArgs (arrayCode, arrayBlocks, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if (p_arrayBlockParams != NULL)
   DATA_multiple_store_in_array_FAL (arrayCode, p_dataItem, arrayBlocks, p_arrayBlockParams);
 free (p_arrayBlockParams);
}

/*
------------------------------------------------------------------------
    Return by reference the data in a given position of an array
    Element positions start from 1
------------------------------------------------------------------------
*/

void DATA_read_from_array (
 DATA_t_array_code  arrayCode,
 unsigned int  dataItemPosition,
 void         *p_dataItem )
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return;
 read_from_genericArray (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode,
   (int)                  dataItemPosition,
   (void *)               p_dataItem );
}

/*
---------------------------------------------------------------------------------
    Return a boolean saying whether a array contains at least one data item
    that meets specific criteria.
---------------------------------------------------------------------------------
*/

/* The main method does the real work */

static Bool DATA_is_in_array_FAL (
 DATA_t_array_code  arrayCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   position;
   
 DATA_clear_error_condition();
 position = position_in_genericArray (
   (t_genericArray_code) arrayCode,
   (unsigned int)        1,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 return ((Bool) (position != 0));
}

/* The wrapper method does all the necessary checking */

Bool DATA_is_in_array (
 DATA_t_array_code   arrayCode, 
 void          *p_dataPattern, 
 unsigned int   patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 Bool
   b_found = FALSE;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return ((Bool) FALSE);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (arrayCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((Bool) FALSE);
 b_found = DATA_is_in_array_FAL (
   (t_genericArray_code) arrayCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (b_found);
}

/*
------------------------------------------------------------------------------
    Return the position of the Nth occurrence of a array element that matches
    the specified criteria.
    Element positions start from 1.
    Return 0 if element occurs less than N times in the array.
------------------------------------------------------------------------------
*/

/* The main method does the real work */

static unsigned int DATA_where_in_array_FAL (
 DATA_t_array_code  arrayCode, 
 unsigned int  occurrence,
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   position;
   
 DATA_clear_error_condition();
 position = position_in_genericArray (
   (t_genericArray_code) arrayCode,
   (unsigned int)        occurrence,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (position);
}

/* The wrapper method does all the necessary checking */

unsigned int DATA_where_in_array (
 DATA_t_array_code   arrayCode, 
 unsigned int   occurrence,
 void          *p_dataPattern, 
 unsigned int   patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 unsigned int
   position;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (arrayCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((unsigned int) 0);
 position = DATA_where_in_array_FAL (
   (t_genericArray_code) arrayCode,
   (unsigned int)        occurrence,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (position);
}

/*
--------------------------------------------------------------------------------
    Return the number of elements in a array that match the specified criteria
--------------------------------------------------------------------------------
*/

/* The main method does the real work */

unsigned int DATA_count_in_array_FAL (
 DATA_t_array_code  arrayCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   occurrences;
   
 DATA_clear_error_condition();
 occurrences = occurrences_in_genericArray (
   (t_genericArray_code) arrayCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (occurrences);
}

/* The wrapper method does all the necessary checking */

unsigned int DATA_count_in_array (
 DATA_t_array_code  arrayCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 unsigned int
   occurrences;
 int
   *p_patternSubsetParams = NULL;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (arrayCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((unsigned int) 0);
 occurrences = DATA_count_in_array_FAL (
   (t_genericArray_code) arrayCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (occurrences);
}

/*
---------------------------------------------------------------------
    Return a boolean saying whether the array is empty
---------------------------------------------------------------------
*/

Bool DATA_is_empty_array (DATA_t_array_code arrayCode)
{
 unsigned int
   arraySize;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 arraySize = DATA_get_array_size (arrayCode);
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 return ((Bool) (arraySize == 0));
}

/*
---------------------------------------------------------------------
    Return the largest array index in use (1 onwards)
---------------------------------------------------------------------
*/

unsigned int DATA_get_array_size (DATA_t_array_code arrayCode)
{
 unsigned int
   arraySize;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 arraySize = elements_in_genericArray ((t_genericArray_code) arrayCode);
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (arraySize);
}

/*
---------------------------------------------------------------------
    Clear the contents of an array
---------------------------------------------------------------------
*/

void DATA_clear_array (DATA_t_array_code arrayCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return;
 clear_genericArray ((t_genericArray_code) arrayCode);
}

/*
---------------------------------------------------------------------
    Dismiss (free) an array and make the underlying
    generic array available for re-use
---------------------------------------------------------------------
*/

void DATA_destroy_array (DATA_t_array_code arrayCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_array,
   (t_genericArray_code)  arrayCode );
 if (has_data_structure_failed)
   return;
 destroy_genericArray ((t_genericArray_code) arrayCode);
}

/*
*---------------------------------------------------------------------
*
*   METHODS FOR SETS
*
*---------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------
    Set up an empty set and return its numeric code
---------------------------------------------------------------------------
*/

/* Version 1 */

DATA_t_set_code DATA_new_set (
 size_t        dataSize, 
 unsigned int  maxDataItems,
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 DATA_t_set_code
   DATA_set_code;
 t_genericArray_info
   *p_GA_info;
 int
   *p_patternSubsetParams = NULL;

 DATA_clear_error_condition();
 check_range_int (
   (int)    patternSubsets,
   (int)    0,
   (int)    dataSize,
   (char *) "patternSubsets",
   (char *) __func__,
   (char *) __FILE__ );
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 DATA_set_code = (DATA_t_set_code) new_genericArray (
  (t_dataStructure_type) t_dataStructure_set,
  (size_t)               dataSize,
  (unsigned int)         maxDataItems,
  (unsigned int)         default_blockSize    [t_dataStructure_set],
  (unsigned int)         default_resizeLimit  [t_dataStructure_set],
  (unsigned int)         default_resizeFactor [t_dataStructure_set] );
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (DATA_set_code, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 p_GA_info = pointer_to_GA_info (DATA_set_code);
 p_GA_info->patternSubsets = patternSubsets;
 p_GA_info->pPatternSubsetParams = p_patternSubsetParams; 
 return (DATA_set_code);
}

/* Version 2 */

DATA_t_set_code DATA_new_set2 (
 size_t        dataSize,
 unsigned int  maxDataItems,
 unsigned int  initialBlockSize,
 unsigned int  resizeLimit,
 unsigned int  resizeFactor,
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 DATA_t_set_code
   DATA_set_code;
 t_genericArray_info
   *p_GA_info;
 int
   *p_patternSubsetParams = NULL;

 DATA_clear_error_condition();
 check_range_int (
   (int)    patternSubsets,
   (int)    0,
   (int)    dataSize,
   (char *) "patternSubsets",
   (char *) __func__,
   (char *) __FILE__ );
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 DATA_set_code = (DATA_t_set_code) new_genericArray (
  (t_dataStructure_type) t_dataStructure_set,
  (size_t)               dataSize,
  (unsigned int)         maxDataItems,
  (unsigned int)         initialBlockSize,
  (unsigned int)         resizeLimit,
  (unsigned int)         resizeFactor );
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (DATA_set_code, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 p_GA_info = pointer_to_GA_info (DATA_set_code);
 p_GA_info->patternSubsets = patternSubsets;
 p_GA_info->pPatternSubsetParams = p_patternSubsetParams; 
 return (DATA_set_code);
}

/*
-----------------------------------------------------------------------------
    Copy part of an existing set into a new one and return its numeric code.
    Only set elements that match the provided data pattern will be copied.
    To copy the whole set, call DATA_copy_set (setCode, NULL, 0);
-----------------------------------------------------------------------------
*/

/* The main method does the real work */

#ifdef ENSURE_NO_DATA_STRUCTURE_ERROR
#undef ENSURE_NO_DATA_STRUCTURE_ERROR
#endif
#define ENSURE_NO_DATA_STRUCTURE_ERROR                        \
   if (has_data_structure_failed) {                           \
     DATA_destroy_set (newSet);                               \
     free (p_dataItem);                                       \
     return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);   \
   }

DATA_t_set_code DATA_copy_DATA_set_FAL (
 DATA_t_set_code    setCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 DATA_t_set_code
   newSet;
 unsigned int
   iData,
   setSize;
 Bool
   b_match;
 void
   *p_dataItem;
 t_genericArray_info
   *p_GA_info;

 if ((patternSubsets == 0) && (p_dataPattern != NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Did not expect a data pattern as patternSubsets = 0");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if ((patternSubsets > 0) && (p_dataPattern == NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Expected a data pattern as patternSubsets > 0 (%ud)", patternSubsets);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if (p_dataPattern == NULL) {
   newSet = (DATA_t_set_code) copy_genericArray ((t_genericArray_code) setCode);
   if (has_data_structure_failed) {
     DATA_destroy_set (newSet);
     return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
   else
     return (newSet);
 }
 p_dataItem = new_dummy_data_item (setCode);
 if (p_dataItem == NULL)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 p_GA_info = pointer_to_GA_info (setCode);
 newSet = (DATA_t_set_code) new_genericArray (
   (t_genericArray_code) setCode,
   (size_t)              p_GA_info->dataSize,
   (unsigned int)        p_GA_info->absMaxDataItems,
   (unsigned int)        p_GA_info->initialBlockSize,
   (unsigned int)        p_GA_info->resizeLimit,
   (unsigned int)        p_GA_info->resizeFactor );
 ENSURE_NO_DATA_STRUCTURE_ERROR
 setSize = DATA_get_set_size (setCode);
 ENSURE_NO_DATA_STRUCTURE_ERROR
 for (iData = 1; iData <= setSize; iData++) {
   DATA_read_from_set (setCode, iData, p_dataItem);
   ENSURE_NO_DATA_STRUCTURE_ERROR
   b_match = data_items_match (p_dataItem, p_dataPattern, p_GA_info->dataSize, patternSubsets, p_patternSubsetParams);
   ENSURE_NO_DATA_STRUCTURE_ERROR
   if (b_match)
     DATA_add_to_set (newSet, p_dataItem, TRUE);
   ENSURE_NO_DATA_STRUCTURE_ERROR
 }
 free (p_dataItem);
 return (newSet);
}

/* The wrapper method does some of the necessary checking and builds the list of data pattern parameters */

DATA_t_set_code DATA_copy_set (
 DATA_t_set_code    setCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 DATA_t_set_code
   resultSet;
 t_genericArray_info
   *p_GA_info;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 p_GA_info = pointer_to_GA_info (setCode);
 check_range_int (
   (int)    patternSubsets,
   (int)    0,
   (int)    p_GA_info->dataSize,
   (char *) "patternSubsets",
   (char *) __func__,
   (char *) __FILE__ );
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 if ((patternSubsets == 0) && (p_dataPattern != NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Did not expect a data pattern as patternSubsets = 0");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if ((patternSubsets > 0) && (p_dataPattern == NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Expected a data pattern as patternSubsets > 0 (%ud)", patternSubsets);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (setCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 resultSet = DATA_copy_DATA_set_FAL (setCode, p_dataPattern, patternSubsets, p_patternSubsetParams);
 free (p_patternSubsetParams);
 return (resultSet);
}

/*
--------------------------------------------------------------------------------
    If a data item is not in a set then add it. If it is, then use the boolean
    parameter to decide whether to overwrite the existing item.
    Remember that set membership is defined by the data pattern subsets that
    were specified at the time of set creation.
--------------------------------------------------------------------------------
*/

void DATA_add_to_set (DATA_t_set_code setCode, void *p_dataItem, Bool b_overwrite)
{
 int
   position;
 t_genericArray_info
   *p_GA_info;
   
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return;
 p_GA_info = pointer_to_GA_info (setCode);
 position = (int) position_in_genericArray (
   (t_genericArray_code) setCode,
   (unsigned int)        1,
   (void *)              p_dataItem,
   (unsigned int)        p_GA_info->patternSubsets,
   (int *)               p_GA_info->pPatternSubsetParams );
 if (has_data_structure_failed)
   return;
 if ((position != 0) && (! b_overwrite))
   return;
 if (position == 0) 
   position = END_OF_SET;
 store_in_genericArray (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode,
   (int)                  position,
   (void *)               p_dataItem );
}

/*
------------------------------------------------------------------------
    Return by reference the data in a given position of a set
    Element positions start from 1
------------------------------------------------------------------------
*/

void DATA_read_from_set (
 DATA_t_set_code    setCode,
 unsigned int  dataItemPosition,
 void         *p_dataItem )
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return;
 read_from_genericArray (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode,
   (int)                  dataItemPosition,
   (void *)               p_dataItem );
}

/*
--------------------------------------------------------------------------------
    Remove certain data items from a set. Consider only the specified data
    pattern subsets when deciding whether that item should be removed from
    the set. 
    
    This function may be called by external code that uses this module, but
    it may also be called by other functions in this module. External code
    should call the wrapper method DATA_remove_from_set(); code in this module
    may call either DATA_remove_from_set() or DATA_remove_from_DATA_set_FAL(), depending
    on whether the variable argument list has already been processed.
--------------------------------------------------------------------------------
*/

/* The main method does the real work */

void DATA_remove_from_DATA_set_FAL (
 DATA_t_set_code    setCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 int
   position;
   
 DATA_clear_error_condition();
   position = (int) position_in_genericArray (
   (t_genericArray_code) setCode,
   (unsigned int)        1,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return;
 while (position > 0) {
   remove_from_genericArray (
     (t_dataStructure_type) t_dataStructure_set,
     (t_genericArray_code)  setCode,
     (int)                  position );
   if (has_data_structure_failed)
     return;
   position = (int) position_in_genericArray (
     (t_genericArray_code) setCode,
     (unsigned int)        1,
     (void *)              p_dataPattern,
     (unsigned int)        patternSubsets,
     (int *)               p_patternSubsetParams );
   if (has_data_structure_failed)
     return;
 }
}

/* The wrapper method does all the necessary checking */

void DATA_remove_from_set (
 DATA_t_set_code    setCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return;

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (setCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 DATA_remove_from_DATA_set_FAL (
   (t_genericArray_code) setCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return;
}

/*
---------------------------------------------------------------------------------
    Return a boolean saying whether a set contains at least one data item
    that meets the specific criteria.
---------------------------------------------------------------------------------
*/

/* The main method does the real work */

static Bool DATA_is_in_DATA_set_FAL (
 DATA_t_set_code    setCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   position;
   
 DATA_clear_error_condition();
 position = position_in_genericArray (
   (t_genericArray_code) setCode,
   (unsigned int)        1,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 return ((Bool) (position != 0));
}

/* The wrapper method does all the necessary checking */

Bool DATA_is_in_set (
 DATA_t_array_code  setCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 Bool
   b_found = FALSE;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return ((Bool) FALSE);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (setCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((Bool) FALSE);
 b_found = DATA_is_in_DATA_set_FAL (
   (t_genericArray_code) setCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (b_found);
}

/*
------------------------------------------------------------------------------
    Return the position of the Nth occurrence of a set element that matches
    the specified criteria.
    Element positions start from 1.
    Return 0 if element occurs less than N times in the set.
------------------------------------------------------------------------------
*/

/* The main method does the real work */

static unsigned int DATA_where_in_DATA_set_FAL (
 DATA_t_set_code    setCode, 
 unsigned int  occurrence,
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   position;
   
 DATA_clear_error_condition();
 position = position_in_genericArray (
   (t_genericArray_code) setCode,
   (unsigned int)        occurrence,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (position);
}

/* The wrapper method does all the necessary checking */

unsigned int DATA_where_in_set (
 DATA_t_set_code     setCode, 
 unsigned int   occurrence,
 void          *p_dataPattern, 
 unsigned int   patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 unsigned int
   position;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (setCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((unsigned int) 0);
 position = DATA_where_in_DATA_set_FAL (
   (t_genericArray_code) setCode,
   (unsigned int)        occurrence,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (position);
}

/*
--------------------------------------------------------------------------------
    Return the number of elements in a set that match the specified criteria
--------------------------------------------------------------------------------
*/

/* The main method does the real work */

unsigned int DATA_count_in_DATA_set_FAL (
 DATA_t_set_code    setCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   occurrences;
   
 DATA_clear_error_condition();
 occurrences = occurrences_in_genericArray (
   (t_genericArray_code) setCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (occurrences);
}

/* The wrapper method does all the necessary checking */

unsigned int DATA_count_in_set (
 DATA_t_set_code    setCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 unsigned int
   occurrences;
 int
   *p_patternSubsetParams = NULL;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (setCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((unsigned int) 0);
 occurrences = DATA_count_in_DATA_set_FAL (
   (t_genericArray_code) setCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (occurrences);
}

/*
---------------------------------------------------------------------
    Return a boolean saying whether the set is empty
---------------------------------------------------------------------
*/

Bool DATA_is_empty_set (DATA_t_set_code setCode)
{
 unsigned int
   setSize;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 setSize = DATA_get_set_size (setCode);
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 return ((Bool) (setSize == 0));
}

/*
---------------------------------------------------------------------
    Return the number of elements in a set
---------------------------------------------------------------------
*/

unsigned int DATA_get_set_size (DATA_t_set_code setCode)
{
 unsigned int
   arraySize;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 arraySize = elements_in_genericArray ((t_genericArray_code) setCode);
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (arraySize);
}

/*
-------------------------------------------------------------------------------
    Perform the union of two sets and return the result in a third set
-------------------------------------------------------------------------------
*/

DATA_t_set_code DATA_set_union (DATA_t_set_code set1, DATA_t_set_code set2)
{
 unsigned int
   iData;
 void
   *p_dataItem;
 DATA_t_set_code
   resultSet;

 DATA_clear_error_condition();
 validate_two_DATA_set_operation (set1, set2, "Union");
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);

 /* All seems well; perform the set union */
 
 resultSet = DATA_copy_set (set1, NULL, 0);
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 for (iData = 1; iData <= DATA_get_set_size (set2); iData++) {
   p_dataItem = pointer_to_dataItem (set2, iData);
   DATA_add_to_set (resultSet, p_dataItem, FALSE);
   if (has_data_structure_failed) {
     DATA_destroy_set (resultSet);
     return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
 }
 return (resultSet);
}

/*
-------------------------------------------------------------------------------
    Perform the intersection of two sets and return the result in a third set
-------------------------------------------------------------------------------
*/

DATA_t_set_code DATA_set_intersection (DATA_t_set_code set1, DATA_t_set_code set2)
{
 unsigned int
   iData;
 Bool
   b_isDuplicate;
 void
   *p_dataItem;
 DATA_t_set_code
   resultSet;

 DATA_clear_error_condition();
 validate_two_DATA_set_operation (set1, set2, "Intersection");
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);

 /* All seems well; perform the set intersection */
 
 resultSet = DATA_copy_set (set1, NULL, 0);
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 for (iData = 1; iData <= DATA_get_set_size (set1); iData++) {
   p_dataItem = pointer_to_dataItem (set1, iData);
   b_isDuplicate = DATA_is_in_set (set2, p_dataItem, 0);
   if (has_data_structure_failed) {
     DATA_destroy_set (resultSet);
     return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
   if (! b_isDuplicate) {
     DATA_remove_from_set (resultSet, p_dataItem, 0);
     if (has_data_structure_failed) {
       DATA_destroy_set (resultSet);
       return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
     }
   }
 }
 return (resultSet);
}

/*
-------------------------------------------------------------------------------
    Perform the difference of two sets and return the result in a third set
-------------------------------------------------------------------------------
*/

DATA_t_set_code DATA_set_difference (DATA_t_set_code set1, DATA_t_set_code set2)
{
 unsigned int
   iData;
 Bool
   b_isDuplicate;
 void
   *p_dataItem;
 DATA_t_set_code
   resultSet;

 DATA_clear_error_condition();
 validate_two_DATA_set_operation (set1, set2, "Difference");
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);

 /* All seems well; perform the set difference */
 
 resultSet = DATA_copy_set (set1, NULL, 0);
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 for (iData = 1; iData <= DATA_get_set_size (set1); iData++) {
   p_dataItem = pointer_to_dataItem (set1, iData);
   b_isDuplicate = DATA_is_in_set (set2, p_dataItem, 0);
   if (has_data_structure_failed) {
     DATA_destroy_set (resultSet);
     return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
   if (b_isDuplicate) {
     DATA_remove_from_set (resultSet, p_dataItem, 0);
     if (has_data_structure_failed) {
       DATA_destroy_set (resultSet);
       return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
     }
   }
 }
 return (resultSet);
}

/*
---------------------------------------------------------------------------------------
    Perform the symmetric difference of two sets and return the result in a third set
---------------------------------------------------------------------------------------
*/

DATA_t_set_code DATA_set_symmetric_difference (DATA_t_set_code set1, DATA_t_set_code set2)
{
 int
   unsigned iData;
 Bool
   b_isDuplicate;
 void
   *p_dataItem;
 DATA_t_set_code
   resultSet;

 DATA_clear_error_condition();
 validate_two_DATA_set_operation (set1, set2, "Symmetric difference");
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);

 /* All seems well; perform the set symmetric difference */
 
 resultSet = DATA_copy_set (set1, NULL, 0);
 if (has_data_structure_failed)
   return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
 for (iData = 1; iData <= DATA_get_set_size (set2); iData++) {
   p_dataItem = pointer_to_dataItem (set2, iData);
   b_isDuplicate = DATA_is_in_set (resultSet, p_dataItem, 0);
   if (has_data_structure_failed) {
     DATA_destroy_set (resultSet);
     return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
   if (b_isDuplicate)
     DATA_remove_from_set (resultSet, p_dataItem, 0);
   else
     DATA_add_to_set (resultSet, p_dataItem, FALSE);
   if (has_data_structure_failed) {
     DATA_destroy_set (resultSet);
     return ((DATA_t_set_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
 }
 return (resultSet);
}

/*
---------------------------------------------------------
    Return a boolean saying whether two sets are equal
---------------------------------------------------------
*/

DATA_t_set_code DATA_are_equal_sets (DATA_t_set_code set1, DATA_t_set_code set2)
{
 unsigned int
   iData,
   size1,
   size2;
 Bool
   b_isDuplicate;
 void
   *p_dataItem;

 DATA_clear_error_condition();
 validate_two_DATA_set_operation (set1, set2, "Set equality");
 if (has_data_structure_failed)
   return (FALSE);
 size1 = DATA_get_set_size (set1);
 if (has_data_structure_failed)
   return (FALSE);
 size2 = DATA_get_set_size (set2);
 if (has_data_structure_failed)
   return (FALSE);
 if (size1 != size2)
   return (FALSE);

 /* All seems well; perform the set equality check */
 
 for (iData = 1; iData <= size1; iData++) {
   p_dataItem = pointer_to_dataItem (set1, iData);
   b_isDuplicate = DATA_is_in_set (set2, p_dataItem, 0);
   if (has_data_structure_failed)
     return (FALSE);
   if (! b_isDuplicate)
     return (FALSE);
 }
 for (iData = 1; iData <= size2; iData++) {
   p_dataItem = pointer_to_dataItem (set2, iData);
   b_isDuplicate = DATA_is_in_set (set1, p_dataItem, 0);
   if (has_data_structure_failed)
     return (FALSE);
   if (! b_isDuplicate)
     return (FALSE);
 }
 return (TRUE);
}

/*
--------------------------------------------------------------
    Return a boolean saying whether set1 is a subset of set2
--------------------------------------------------------------
*/

DATA_t_set_code DATA_is_subset (DATA_t_set_code set1, DATA_t_set_code set2)
{
 int
   unsigned iData;
 unsigned int
   size1,
   size2;
 Bool
   b_isDuplicate;
 void
   *p_dataItem;

 DATA_clear_error_condition();
 validate_two_DATA_set_operation (set1, set2, "Subset check");
 if (has_data_structure_failed)
   return (FALSE);
 size1 = DATA_get_set_size (set1);
 if (has_data_structure_failed)
   return (FALSE);
 size2 = DATA_get_set_size (set2);
 if (has_data_structure_failed)
   return (FALSE);
 if (size1 > size2)
   return (FALSE);

 /* All seems well; perform the subset check */
 
 for (iData = 1; iData <= size1; iData++) {
   p_dataItem = pointer_to_dataItem (set1, iData);
   b_isDuplicate = DATA_is_in_set (set2, p_dataItem, 0);
   if (has_data_structure_failed)
     return (FALSE);
   if (! b_isDuplicate)
     return (FALSE);
 }
 return (TRUE);
}

/*
---------------------------------------------------------------------
    Clear the contents of a set
---------------------------------------------------------------------
*/

void DATA_clear_set (DATA_t_set_code setCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return;
 clear_genericArray ((t_genericArray_code) setCode);
}

/*
---------------------------------------------------------------------
    Dismiss (free) a set and make the underlying
    generic array available for re-use
---------------------------------------------------------------------
*/

void DATA_destroy_set (DATA_t_set_code setCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_set,
   (t_genericArray_code)  setCode );
 if (has_data_structure_failed)
   return;
 destroy_genericArray ((t_genericArray_code) setCode);
}

/*
*---------------------------------------------------------------------
*
*   METHODS FOR STACKS
*
*---------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------
    Set up an empty stack and return its numeric code
---------------------------------------------------------------------------
*/

/* Version 1 */

DATA_t_stack_code DATA_new_stack (size_t dataSize, unsigned int maxDataItems)
{
 t_genericArray_code
   genericArray_code;

 DATA_clear_error_condition();
 genericArray_code = new_genericArray (
  (t_dataStructure_type) t_dataStructure_stack,
  (size_t)               dataSize,
  (unsigned int)         maxDataItems,
  (unsigned int)         default_blockSize    [t_dataStructure_stack],
  (unsigned int)         default_resizeLimit  [t_dataStructure_stack],
  (unsigned int)         default_resizeFactor [t_dataStructure_stack] );
 if (has_data_structure_failed)
   return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
 return ((DATA_t_stack_code)  genericArray_code);
}

/* Version 2 */

DATA_t_stack_code DATA_new_stack2 (
 size_t       dataSize,
 unsigned int maxDataItems,
 unsigned int initialBlockSize,
 unsigned int resizeLimit,
 unsigned int resizeFactor )
{
 t_genericArray_code
   genericArray_code;

 DATA_clear_error_condition();
 genericArray_code = new_genericArray (
  (t_dataStructure_type) t_dataStructure_stack,
  (size_t)               dataSize,
  (unsigned int)         maxDataItems,
  (unsigned int)         initialBlockSize,
  (unsigned int)         resizeLimit,
  (unsigned int)         resizeFactor );
 if (has_data_structure_failed)
   return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
 return ((DATA_t_stack_code)  genericArray_code);
}

/*
--------------------------------------------------------------------------------
    Copy part of an existing stack onto a new one and return its numeric code.
    Only data items that match the provided data pattern will be copied; their
    relative positions on the stack will be kept.
    To copy the whole stack, call DATA_copy_stack (stackCode, NULL, 0);
--------------------------------------------------------------------------------
*/

/* The main method does the real work */

#ifdef ENSURE_NO_DATA_STRUCTURE_ERROR
#undef ENSURE_NO_DATA_STRUCTURE_ERROR
#endif
#define ENSURE_NO_DATA_STRUCTURE_ERROR                          \
   if (has_data_structure_failed) {                             \
     DATA_destroy_stack (newStack);                             \
     free (p_dataItem);                                         \
     return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);   \
   }

static DATA_t_stack_code DATA_copy_stack_FAL (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 DATA_t_stack_code
   newStack;
 int
   iSourcePos,
   sourceSize;
 Bool
   b_match;
 void
   *p_dataItem;
 t_genericArray_info
   *p_GA_info;

   
 if ((patternSubsets == 0) && (p_dataPattern != NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Did not expect a data pattern as patternSubsets = 0");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if ((patternSubsets > 0) && (p_dataPattern == NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Expected a data pattern as patternSubsets > 0 (%ud)", patternSubsets);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if (p_dataPattern == NULL) {
   newStack = (DATA_t_stack_code) copy_genericArray ((t_genericArray_code) stackCode);
   if (has_data_structure_failed) {
     DATA_destroy_array (newStack);
     return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
   else
     return (newStack);
 }
 p_dataItem = new_dummy_data_item (stackCode);
 if (p_dataItem == NULL)
   return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
 p_GA_info = pointer_to_GA_info (stackCode);
 newStack = (DATA_t_stack_code) new_genericArray (
   (t_genericArray_code)   stackCode,
   (size_t)                p_GA_info->dataSize,
   (unsigned int)          p_GA_info->absMaxDataItems,
   (unsigned int)          p_GA_info->initialBlockSize,
   (unsigned int)          p_GA_info->resizeLimit,
   (unsigned int)          p_GA_info->resizeFactor );
 ENSURE_NO_DATA_STRUCTURE_ERROR
 sourceSize = (int) DATA_get_stack_height (stackCode);
 ENSURE_NO_DATA_STRUCTURE_ERROR
 for (iSourcePos = 1; iSourcePos <= sourceSize; iSourcePos++) {
   read_from_genericArray (t_dataStructure_stack, stackCode, iSourcePos, p_dataItem);
   ENSURE_NO_DATA_STRUCTURE_ERROR
   b_match = data_items_match (p_dataItem, p_dataPattern, p_GA_info->dataSize, patternSubsets, p_patternSubsetParams);
   ENSURE_NO_DATA_STRUCTURE_ERROR
   if (b_match)
     DATA_push_onto_stack (newStack, p_dataItem);
   ENSURE_NO_DATA_STRUCTURE_ERROR
 }
 free (p_dataItem);
 return (newStack);
}

/* The wrapper method does some of the necessary checking and builds the list of data pattern parameters */

DATA_t_stack_code DATA_copy_stack (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 DATA_t_stack_code
   newStack;
 t_genericArray_info
   *p_GA_info;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
 p_GA_info = pointer_to_GA_info (stackCode);
 check_range_int (
   (int)    patternSubsets,
   (int)    0,
   (int)    p_GA_info->dataSize,
   (char *) "patternSubsets",
   (char *) __func__,
   (char *) __FILE__ );
 if (has_data_structure_failed)
   return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
 if ((patternSubsets == 0) && (p_dataPattern != NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Did not expect a data pattern as patternSubsets = 0");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if ((patternSubsets > 0) && (p_dataPattern == NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Expected a data pattern as patternSubsets > 0 (%ud)", patternSubsets);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_stack_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (stackCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 newStack = DATA_copy_stack_FAL (stackCode, p_dataPattern, patternSubsets, p_patternSubsetParams);
 free (p_patternSubsetParams);
 return (newStack);
}

/*
---------------------------------------------------------------------
    Push data onto the top of a stack
---------------------------------------------------------------------
*/

void DATA_push_onto_stack (DATA_t_stack_code stackCode, void *p_dataItem)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return;
 store_in_genericArray (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode,
   (int)                  TOP_OF_STACK,
   (void *)               p_dataItem );
}

/*
---------------------------------------------------------------------
    Return by reference the data at the top of the stack
    but don't remove it
---------------------------------------------------------------------
*/

void DATA_top_of_stack (DATA_t_stack_code stackCode, void *p_dataItem)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return;
 read_from_genericArray (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode,
   (int)                  TOP_OF_STACK,
   (void *)               p_dataItem );
}

/*
------------------------------------------------------------------------
    Pop data from the top of the stack and return the data by reference
------------------------------------------------------------------------
*/

void DATA_pop_from_stack (DATA_t_stack_code stackCode, void *p_dataItem)
{
 unsigned int
   stackSize;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return;
 stackSize = DATA_get_stack_height (stackCode);
 if (has_data_structure_failed)
   return;
 if (stackSize == 0) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Stack %d is empty", stackCode);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
 read_from_genericArray (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode,
   (int)                  TOP_OF_STACK,
   (void *)               p_dataItem );
 if (has_data_structure_failed)
   return;
 remove_from_genericArray (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode,
   (int)                  TOP_OF_STACK );
}

/*
---------------------------------------------------------------------------------
    Return a boolean saying whether a stack contains at least one data item
    that meets specific criteria.
---------------------------------------------------------------------------------
*/

/* The main method does the real work */

static Bool is_on_stack_FAL (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   position;
   
 DATA_clear_error_condition();
 position = position_in_genericArray (
   (t_genericArray_code) stackCode,
   (unsigned int)        1,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 return ((Bool) (position != 0));
}

/* The wrapper method does all the necessary checking */

Bool DATA_is_on_stack (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 Bool
   b_found = FALSE;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return ((Bool) FALSE);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (stackCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((Bool) FALSE);
 b_found = is_on_stack_FAL (
   (t_genericArray_code) stackCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (b_found);
}

/*
------------------------------------------------------------------------------
    Return the position of the Nth occurrence of a stack element that matches
    the specified criteria.
    Element positions start from 1 (top of stack).
    Return 0 if element occurs less than N times in the stack.
------------------------------------------------------------------------------
*/

/* The main method does the real work */

static unsigned int where_on_stack_FAL (
 DATA_t_stack_code  stackCode, 
 unsigned int  occurrence,
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   position;
   
 DATA_clear_error_condition();
 position = position_in_genericArray (
   (t_genericArray_code) stackCode,
   (unsigned int)        occurrence,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (position);
}

/* The wrapper method does all the necessary checking */

unsigned int DATA_where_on_stack (
 DATA_t_stack_code  stackCode, 
 unsigned int  occurrence,
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 unsigned int
   position;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (stackCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((unsigned int) 0);
 position = where_on_stack_FAL (
   (t_genericArray_code) stackCode,
   (unsigned int)        occurrence,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (position);
}

/*
--------------------------------------------------------------------------------
    Return the number of elements in a stack that match the specified criteria
--------------------------------------------------------------------------------
*/

/* The main method does the real work */

unsigned int DATA_count_on_stack_FAL (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   occurrences;
   
 DATA_clear_error_condition();
 occurrences = occurrences_in_genericArray (
   (t_genericArray_code) stackCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (occurrences);
}

/* The wrapper method does all the necessary checking */

unsigned int DATA_count_on_stack (
 DATA_t_stack_code  stackCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 unsigned int
   occurrences;
 int
   *p_patternSubsetParams = NULL;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (stackCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((unsigned int) 0);
 occurrences = DATA_count_on_stack_FAL (
   (t_genericArray_code) stackCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (occurrences);
}

/*
---------------------------------------------------------------------
    Return a boolean saying whether the stack is empty
---------------------------------------------------------------------
*/

Bool DATA_is_empty_stack (DATA_t_stack_code stackCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 return ((Bool) (DATA_get_stack_height (stackCode) == 0));
}

/*
---------------------------------------------------------------------
    Return the size of a stack
---------------------------------------------------------------------
*/

unsigned int DATA_get_stack_height (DATA_t_stack_code stackCode)
{
 unsigned int
   arraySize;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 arraySize = elements_in_genericArray ((t_genericArray_code) stackCode);
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (arraySize);
}

/*
---------------------------------------------------------------------
    Remove all elements in a stack
---------------------------------------------------------------------
*/

void DATA_clear_stack (DATA_t_stack_code stackCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return;
 clear_genericArray ((t_genericArray_code) stackCode);
}

/*
---------------------------------------------------------------------
    Dismiss (free) a stack and make the underlying
    generic array available for re-use
---------------------------------------------------------------------
*/

void DATA_destroy_stack (DATA_t_stack_code stackCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_stack,
   (t_genericArray_code)  stackCode );
 if (has_data_structure_failed)
   return;
 destroy_genericArray ((t_genericArray_code) stackCode);
}

/*
*---------------------------------------------------------------------
*
*   METHODS FOR QUEUES
*
*---------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------
    Set up an empty queue and return its numeric code
---------------------------------------------------------------------------
*/

/* Version 1 */

DATA_t_queue_code DATA_new_queue (size_t dataSize, unsigned int maxDataItems)
{
 t_genericArray_code
   genericArray_code;

 DATA_clear_error_condition();
 genericArray_code = new_genericArray (
  (t_dataStructure_type) t_dataStructure_queue,
  (size_t)               dataSize,
  (unsigned int)         maxDataItems,
  (unsigned int)         default_blockSize    [t_dataStructure_queue],
  (unsigned int)         default_resizeLimit  [t_dataStructure_queue],
  (unsigned int)         default_resizeFactor [t_dataStructure_queue] );
 if (has_data_structure_failed)
   return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
 return ((DATA_t_queue_code)  genericArray_code);
}

/* Version 2 */

DATA_t_queue_code DATA_new_queue2 (
 size_t       dataSize,
 unsigned int maxDataItems,
 unsigned int initialBlockSize,
 unsigned int resizeLimit,
 unsigned int resizeFactor )
{
 t_genericArray_code
   genericArray_code;

 DATA_clear_error_condition();
 genericArray_code = new_genericArray (
  (t_dataStructure_type) t_dataStructure_queue,
  (size_t)               dataSize,
  (unsigned int)         maxDataItems,
  (unsigned int)         initialBlockSize,
  (unsigned int)         resizeLimit,
  (unsigned int)         resizeFactor );
 if (has_data_structure_failed)
   return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
 return ((DATA_t_queue_code)  genericArray_code);
}

/*
--------------------------------------------------------------------------------
    Copy part of an existing queue into a new one and return its numeric code.
    Only data items that match the provided data pattern will be copied; their
    relative positions in the queue will be kept.
    To copy the whole queue, call DATA_copy_queue (queueCode, NULL, 0);
--------------------------------------------------------------------------------
*/

/* The main method does the real work */

#ifdef ENSURE_NO_DATA_STRUCTURE_ERROR
#undef ENSURE_NO_DATA_STRUCTURE_ERROR
#endif
#define ENSURE_NO_DATA_STRUCTURE_ERROR                          \
   if (has_data_structure_failed) {                             \
     DATA_destroy_queue (newQueue);                             \
     free (p_dataItem);                                         \
     return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);   \
   }

static DATA_t_queue_code DATA_copy_queue_FAL (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 DATA_t_queue_code
   newQueue;
 int
   iSourcePos,
   sourceSize;
 Bool
   b_match;
 void
   *p_dataItem;
 t_genericArray_info
   *p_GA_info;

   
 if ((patternSubsets == 0) && (p_dataPattern != NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Did not expect a data pattern as patternSubsets = 0");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if ((patternSubsets > 0) && (p_dataPattern == NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Expected a data pattern as patternSubsets > 0 (%ud)", patternSubsets);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if (p_dataPattern == NULL) {
   newQueue = (DATA_t_queue_code) copy_genericArray ((t_genericArray_code) queueCode);
   if (has_data_structure_failed) {
     DATA_destroy_array (newQueue);
     return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
   }
   else
     return (newQueue);
 }
 p_dataItem = new_dummy_data_item (queueCode);
 if (p_dataItem == NULL)
   return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
 p_GA_info = pointer_to_GA_info (queueCode);
 newQueue = (DATA_t_queue_code) new_genericArray (
   (t_genericArray_code)   queueCode,
   (size_t)                p_GA_info->dataSize,
   (unsigned int)          p_GA_info->absMaxDataItems,
   (unsigned int)          p_GA_info->initialBlockSize,
   (unsigned int)          p_GA_info->resizeLimit,
   (unsigned int)          p_GA_info->resizeFactor );
 ENSURE_NO_DATA_STRUCTURE_ERROR
 sourceSize = (int) DATA_get_queue_length (queueCode);
 ENSURE_NO_DATA_STRUCTURE_ERROR
 for (iSourcePos = 1; iSourcePos <= sourceSize; iSourcePos++) {
   read_from_genericArray (t_dataStructure_queue, queueCode, iSourcePos, p_dataItem);
   ENSURE_NO_DATA_STRUCTURE_ERROR
   b_match = data_items_match (p_dataItem, p_dataPattern, p_GA_info->dataSize, patternSubsets, p_patternSubsetParams);
   ENSURE_NO_DATA_STRUCTURE_ERROR
   if (b_match)
     DATA_join_queue (newQueue, p_dataItem);
   ENSURE_NO_DATA_STRUCTURE_ERROR
 }
 free (p_dataItem);
 return (newQueue);
}

/* The wrapper method does some of the necessary checking and builds the list of data pattern parameters */

DATA_t_queue_code DATA_copy_queue (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 DATA_t_queue_code
   newQueue;
 t_genericArray_info
   *p_GA_info;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
 p_GA_info = pointer_to_GA_info (queueCode);
 check_range_int (
   (int)    patternSubsets,
   (int)    0,
   (int)    p_GA_info->dataSize,
   (char *) "patternSubsets",
   (char *) __func__,
   (char *) __FILE__ );
 if (has_data_structure_failed)
   return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
 if ((patternSubsets == 0) && (p_dataPattern != NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Did not expect a data pattern as patternSubsets = 0");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 if ((patternSubsets > 0) && (p_dataPattern == NULL)) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Expected a data pattern as patternSubsets > 0 (%ud)", patternSubsets);
   data_error (0, __FILE__, __func__, auxErrorStr);
   return ((DATA_t_queue_code) UNKNOWN_GENERIC_ARRAY_CODE);
 }
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (queueCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 newQueue = DATA_copy_queue_FAL (queueCode, p_dataPattern, patternSubsets, p_patternSubsetParams);
 free (p_patternSubsetParams);
 return (newQueue);
}

/*
---------------------------------------------------------------------
    Add data to the back of the queue
---------------------------------------------------------------------
*/

void DATA_join_queue (DATA_t_queue_code queueCode, void *p_dataItem)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return;
 store_in_genericArray (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode,
   (int)                  BACK_OF_QUEUE,
   (void *)               p_dataItem );
}

/*
---------------------------------------------------------------------
    Return by reference the data at the front of the queue
    but don't remove it
---------------------------------------------------------------------
*/

void DATA_front_of_queue (DATA_t_queue_code queueCode, void *p_dataItem)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return;
 read_from_genericArray (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode,
   (int)                  FRONT_OF_QUEUE,
   (void *)               p_dataItem );
}

/*
------------------------------------------------------------------------------
    Remove data from the front of the queue and return the data by reference
------------------------------------------------------------------------------
*/

void DATA_leave_queue (DATA_t_queue_code queueCode, void *p_dataItem)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return;
 read_from_genericArray (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode,
   (int)                  FRONT_OF_QUEUE,
   (void *)               p_dataItem );
 if (has_data_structure_failed)
   return;
 remove_from_genericArray (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode,
   (int)                  FRONT_OF_QUEUE );
}

/*
---------------------------------------------------------------------------------
    Return a boolean saying whether a queue contains at least one data item
    that meets specific criteria.
---------------------------------------------------------------------------------
*/

/* The main method does the real work */

static Bool DATA_is_in_queue_FAL (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   position;
   
 DATA_clear_error_condition();
 position = position_in_genericArray (
   (t_genericArray_code) queueCode,
   (unsigned int)        1,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 return ((Bool) (position != 0));
}

/* The wrapper method does all the necessary checking */

Bool DATA_is_in_queue (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 Bool
   b_found = FALSE;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return ((Bool) FALSE);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (queueCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((Bool) FALSE);
 b_found = DATA_is_in_queue_FAL (
   (t_genericArray_code) queueCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (b_found);
}

/*
------------------------------------------------------------------------------
    Return the position of the Nth occurrence of a queue element that matches
    the specified criteria.
    Element positions start from 1 (front of queue).
    Return 0 if element occurs less than N times in the queue.
------------------------------------------------------------------------------
*/

/* The main method does the real work */

static unsigned int DATA_where_in_queue_FAL (
 DATA_t_queue_code  queueCode, 
 unsigned int  occurrence,
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   position;
   
 DATA_clear_error_condition();
 position = position_in_genericArray (
   (t_genericArray_code) queueCode,
   (unsigned int)        occurrence,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (position);
}

/* The wrapper method does all the necessary checking */

unsigned int DATA_where_in_queue (
 DATA_t_queue_code  queueCode, 
 unsigned int  occurrence,
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 int
   *p_patternSubsetParams = NULL;
 unsigned int
   position;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (queueCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((unsigned int) 0);
 position = DATA_where_in_queue_FAL (
   (t_genericArray_code) queueCode,
   (unsigned int)        occurrence,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (position);
}

/*
--------------------------------------------------------------------------------
    Return the number of elements in a queue that match the specified criteria
--------------------------------------------------------------------------------
*/

/* The main method does the real work */

unsigned int DATA_count_in_queue_FAL (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 int          *p_patternSubsetParams )
{
 unsigned int
   occurrences;
   
 DATA_clear_error_condition();
 occurrences = occurrences_in_genericArray (
   (t_genericArray_code) queueCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (occurrences);
}

/* The wrapper method does all the necessary checking */

unsigned int DATA_count_in_queue (
 DATA_t_queue_code  queueCode, 
 void         *p_dataPattern, 
 unsigned int  patternSubsets,
 ... )
{
 va_list
   varArgs;
 unsigned int
   occurrences;
 int
   *p_patternSubsetParams = NULL;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);

 /* The function's variable arguments will be checked by convert_dataPattern_varArgs() */
 /* so there is no need to do this now; just look for the data                         */
 
 va_start (varArgs, patternSubsets);
 p_patternSubsetParams = convert_dataPattern_varArgs (queueCode, patternSubsets, varArgs, __func__, __FILE__);
 va_end (varArgs);
 if ((patternSubsets > 0) && (p_patternSubsetParams == NULL))
   return ((unsigned int) 0);
 occurrences = DATA_count_in_queue_FAL (
   (t_genericArray_code) queueCode,
   (void *)              p_dataPattern,
   (unsigned int)        patternSubsets,
   (int *)               p_patternSubsetParams );
 free (p_patternSubsetParams);
 return (occurrences);
}

/*
---------------------------------------------------------------------
    Return a boolean saying whether the queue is empty
---------------------------------------------------------------------
*/

Bool DATA_is_empty_queue (DATA_t_queue_code queueCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return ((Bool) FALSE);
 return ((Bool) (DATA_get_queue_length (queueCode) == 0));
}

/*
---------------------------------------------------------------------
    Return the size of a queue
---------------------------------------------------------------------
*/

unsigned int DATA_get_queue_length (DATA_t_queue_code queueCode)
{
 unsigned int
   arraySize;

 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 arraySize = elements_in_genericArray ((t_genericArray_code) queueCode);
 if (has_data_structure_failed)
   return ((unsigned int) 0);
 return (arraySize);
}

/*
---------------------------------------------------------------------
    Remove all elements in a queue
---------------------------------------------------------------------
*/

void DATA_clear_queue (DATA_t_queue_code queueCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return;
 clear_genericArray ((t_genericArray_code) queueCode);
}

/*
---------------------------------------------------------------------
    Dismiss (free) a queue and make the underlying
    generic array available for re-use
---------------------------------------------------------------------
*/

void DATA_destroy_queue (DATA_t_queue_code queueCode)
{
 DATA_clear_error_condition();
 validate_genericArray_type (
   (t_dataStructure_type) t_dataStructure_queue,
   (t_genericArray_code)  queueCode );
 if (has_data_structure_failed)
   return;
 destroy_genericArray ((t_genericArray_code) queueCode);
}

/*
*---------------------------------------------------------------------
*
*   INITIALIZE & DISMISS data structures
*
*---------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------
    Initialize this modules internal variables and data structures
---------------------------------------------------------------------
*/

void DATA_initialize_module (Bool abort_on_error)
{
 unsigned int
   arrayIndex;
 t_genericArray_info
   *p_GA_info;

 if (is_data_structures_initialized) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Module already initialized; must DATA_destroy_all() first");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }
 DATA_clear_error_condition();
 is_data_structures_initialized = TRUE;
 b_abort_on_error = abort_on_error;

 tot_genericArrays =
 genericArrays_infoVector_totResizes =
 genericArrays_infoVector_totResizes_currBlockSize =
   (unsigned int) 0;

 max_genericArrays =
 genericArrays_infoVector_currBlockSize =
   (unsigned int) GENERIC_ARRAYS_INFO_VECTOR_INITIAL_BLOCK_SIZE;

 genericArrays_infoVector_resizeLimit =
   (unsigned int) GENERIC_ARRAYS_INFO_VECTOR_BLOCK_RESIZE_LIMIT;

 genericArrays_infoVector_resizeFactor =
   (unsigned int) GENERIC_ARRAYS_INFO_VECTOR_BLOCK_RESIZE_FACTOR;

 genericArrays_infoVector_currSize =
   (size_t) genericArrays_infoVector_currBlockSize * sizeof (t_genericArray_info);

 genericArrays_infoVector =
   (t_genericArray_info *) malloc (genericArrays_infoVector_currSize);

 if (genericArrays_infoVector == NULL) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Unable to realloc genericArrays_infoVector");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }

 memset ((void *) genericArrays_infoVector, 0, genericArrays_infoVector_currSize);

 for (arrayIndex = 0; arrayIndex < max_genericArrays; arrayIndex++) {
   p_GA_info = &(genericArrays_infoVector [arrayIndex]);
   p_GA_info->b_engaged                = (Bool)                 FALSE;
   p_GA_info->b_extensible             = (Bool)                 FALSE;
   p_GA_info->dataType                 = (t_dataStructure_type) UNKNOWN_DATA_STRUCTURE_TYPE;
   p_GA_info->dataSize                 = (size_t)               0;
   p_GA_info->currArraySize            = (size_t)               0;
   p_GA_info->totDataItems             = (unsigned int)         0;
   p_GA_info->maxDataItems             = (unsigned int)         0;
   p_GA_info->absMaxDataItems          = (unsigned int)         0;
   p_GA_info->initialBlockSize         = (unsigned int)         0;
   p_GA_info->currBlockSize            = (unsigned int)         0;
   p_GA_info->totResizes_currBlockSize = (unsigned int)         0;
   p_GA_info->totResizes               = (unsigned int)         0;
   p_GA_info->resizeLimit              = (unsigned int)         0;
   p_GA_info->resizeFactor             = (unsigned int)         0;
   p_GA_info->pArray                   = (void *)               NULL;
 }
}

/*
---------------------------------------------------------------------
    Clear the modules internal variables and data structures
    and free all dynamically allocated memory
---------------------------------------------------------------------
*/

void DATA_destroy_all (void)
{
 unsigned int
   arrayIndex;

 if (! is_data_structures_initialized) {
   snprintf (auxErrorStr, MAX_ERROR_MESSAGE_LENGTH, "Nothing to destroy; must DATA_initialize_module() first");
   data_error (0, __FILE__, __func__, auxErrorStr);
   return;
 }

 is_data_structures_initialized = FALSE;
 DATA_clear_error_condition();
 for (arrayIndex = 0; arrayIndex < max_genericArrays; arrayIndex++)
   destroy_genericArray ((t_genericArray_code) arrayIndex + GENERIC_ARRAY_START_CODE);

 tot_genericArrays =
   max_genericArrays =
   genericArrays_infoVector_currBlockSize =
   genericArrays_infoVector_totResizes =
   genericArrays_infoVector_totResizes_currBlockSize =
   genericArrays_infoVector_resizeLimit =
   genericArrays_infoVector_resizeFactor = (unsigned int) 0;

 genericArrays_infoVector_currSize = (size_t) 0;

 if (genericArrays_infoVector != NULL) {
   free ((void *) genericArrays_infoVector);
   genericArrays_infoVector = NULL;
 }
}

/*
---------------------------------------------------------------------
    Return boolean indicating whether the data structures module 
    has been initialized
---------------------------------------------------------------------
*/

Bool DATA_is_module_initialized (void) {
  return (is_data_structures_initialized);
}

/*
---------------------------------------------------------------------
    Return boolean indicating whether the last
    data structure operation failed
---------------------------------------------------------------------
*/

Bool DATA_has_data_structure_failed (void) {
  return (has_data_structure_failed);
}
