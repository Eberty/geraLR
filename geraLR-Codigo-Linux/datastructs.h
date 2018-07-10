/*
*---------------------------------------------------------------------
*
*   File         : datastructs.h
*   Created      : 2006-11-02
*   Last Modified: 2012-05-27
*
*   Type-safe implementation of several dynamically allocated data
*   structures and access methods - or as type safe as we can
*   make them in C
*
*---------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _DATASTRUCTS_DOT_H_
#define _DATASTRUCTS_DOT_H_

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <stdbool.h>

/*
*---------------------------------------------------------------------
*
*   INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

/* Access codes for the various types of data structures implemented */

typedef int
  DATA_t_array_code,
  DATA_t_set_code,
  DATA_t_stack_code,
  DATA_t_queue_code;

/* How subsets of data items are to be combined when looking for matches */

typedef enum {
  DATA_t_AND = 1,
  DATA_t_OR,
  DATA_t_XOR
}
  DATA_t_patternMatchType;

/*
*---------------------------------------------------------------------
*   Function prototypes
*---------------------------------------------------------------------
*/

/* Boolean indicating error condition; must be checked */
/* after each call to all data structure methods       */

extern bool  DATA_is_module_initialized     (void);
extern bool  DATA_has_data_structure_failed (void);

extern void  DATA_initialize_module (bool abort_on_error);
extern char *DATA_get_error         (void);
extern void  DATA_destroy_all       (void);

/*
-------------------------------------------------------------------------------
THE COLLECTION OF METHODS - DECLARATIONS
-------------------------------------------------------------------------------
*/

/*--------------------*/
/* Methods for ARRAYS */
/*--------------------*/

/* Set up an empty array and return its numeric code */

extern DATA_t_array_code DATA_new_array (size_t dataSize, unsigned int maxDataItems);

extern DATA_t_array_code DATA_new_array2 (
 size_t       dataSize,
 unsigned int maxDataItems,
 unsigned int initialBlockSize,
 unsigned int resizeLimit,
 unsigned int resizeFactor );

/* Copy part of an existing array into a new one and return the new array's numeric code */

extern DATA_t_array_code DATA_copy_array (
 DATA_t_array_code  arrayCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Store a data item in a array */

extern void DATA_store_in_array (
 DATA_t_array_code arrayCode,
 unsigned int elementPosition,
 void        *p_dataItem );

/* Store multiple copies of a data item in specified positions in a array */

extern void DATA_multiple_store_in_array (
 DATA_t_array_code  arrayCode,
 void         *p_dataItem,
 unsigned int  arrayBlocks,
 ... );

/* Retrieve a data item stored in a array */

extern void DATA_read_from_array (
 DATA_t_array_code arrayCode,
 unsigned int elementPosition,
 void        *p_dataItem );

/* Find out if a data item is in a array */

extern bool DATA_is_in_array (
 DATA_t_array_code  arrayCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Find the position of a data item in a array */

extern unsigned int DATA_where_in_array (
 DATA_t_array_code   arrayCode,
 unsigned int   occurrence,
 void          *p_dataPattern,
 unsigned int   patternSubsets,
 ... );

/* Find out how many data items in a array share the same portions of data */

extern unsigned int DATA_count_in_array (
 DATA_t_array_code  arrayCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Is the array empty? */

extern bool DATA_is_empty_array (DATA_t_array_code arrayCode);

/* Obtain the largest array index in use (1 onwards) */

extern unsigned int DATA_get_array_size (DATA_t_array_code arrayCode);

/* Clear all data stored in a array */

extern void DATA_clear_array (DATA_t_array_code arrayCode);

/* Delete an array and its data */

extern void DATA_destroy_array (DATA_t_array_code arrayCode);

/*------------------*/
/* Methods for SETS */
/*------------------*/

/* Set up an empty set, define set membership and return its numeric code */

extern DATA_t_set_code DATA_new_set (
 size_t        dataSize,
 unsigned int  maxDataItems,
 unsigned int  patternSubsets,
 ... );

extern DATA_t_set_code DATA_new_set2 (
 size_t        dataSize,
 unsigned int  maxDataItems,
 unsigned int  initialBlockSize,
 unsigned int  resizeLimit,
 unsigned int  resizeFactor,
 unsigned int  patternSubsets,
 ... );

/* Copy part of an existing set into a new one and return its numeric code */

extern DATA_t_set_code DATA_copy_set (
 DATA_t_set_code    setCode,
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

 /* Add a data item to a set */

extern void DATA_add_to_set (DATA_t_set_code setCode, void *p_dataItem, bool b_overwrite);

/* Retrieve a data item in a set */

extern void DATA_read_from_set (
 DATA_t_set_code    setCode,
 unsigned int  elementPosition,
 void         *p_dataItem );

/* Remove certain data items from a set */

extern void DATA_remove_from_set (
 DATA_t_set_code    setCode,
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

/* Find out if a data item is in a set */

extern bool DATA_is_in_set (
 DATA_t_set_code    setCode,
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

/* Find the position of a data item in a set */

extern unsigned int DATA_where_in_set (
 DATA_t_set_code    setCode,
 unsigned int  occurrence,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Find out how many data items in a set share the same portions of data */

extern unsigned int DATA_count_in_set (
 DATA_t_set_code    setCode,
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

/* Is the set empty? */

extern bool DATA_is_empty_set (DATA_t_set_code setCode);

/* Obtain the number of data items in a set */

extern unsigned int DATA_get_set_size (DATA_t_set_code setCode);

/* Perform the union of set 1 and set 2 and return the result in a third set */

extern DATA_t_set_code DATA_set_union (DATA_t_set_code set1, DATA_t_set_code set2);

/* Perform the intersection of set1 and set2 and return the result in a third set */

extern DATA_t_set_code DATA_set_intersection (DATA_t_set_code set1, DATA_t_set_code set2);

/* Work out s1-s2 and return the result in a third set */

extern DATA_t_set_code DATA_set_difference (DATA_t_set_code set1, DATA_t_set_code set2);

/* Work out s1 XOR s2 and return the result in a third set */

extern DATA_t_set_code DATA_set_symmetric_difference (DATA_t_set_code set1, DATA_t_set_code set2);

/* Find out if two sets are equal */

extern bool DATA_are_equal_sets (DATA_t_set_code set1, DATA_t_set_code set2);

/* Find out if set2 is a subset of set2 */

extern bool DATA_is_subset (DATA_t_set_code set1, DATA_t_set_code set2);

/* Remove all data from a set */

extern void DATA_clear_set (DATA_t_set_code setCode);

/* Delete a set and its data */

extern void DATA_destroy_set (DATA_t_set_code setCode);

/*--------------------*/
/* Methods for STACKS */
/*--------------------*/

/* Set up an empty stack and return its numeric code */

extern DATA_t_stack_code DATA_new_stack (size_t dataSize, unsigned int maxDataItems);

extern DATA_t_stack_code DATA_new_stack2 (
 size_t       dataSize,
 unsigned int maxDataItems,
 unsigned int initialBlockSize,
 unsigned int resizeLimit,
 unsigned int resizeFactor );

/* Copy part of an existing stack into a new one and return its numeric code */

extern DATA_t_stack_code DATA_copy_stack (
 DATA_t_stack_code  stackCode,
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

/* Push a data item onto the top of the stack */

extern void DATA_push_onto_stack (DATA_t_stack_code stackCode, void *p_dataItem);

/* Read the data item at the top of the stack without removing it */

extern void DATA_top_of_stack (DATA_t_stack_code stackCode, void *p_dataItem);

/* Pop the data item from the top of the stack and return the data */

extern void DATA_pop_from_stack (DATA_t_stack_code stackCode, void *p_dataItem);

/* Find out if a stack contains at least one data item that meets the specified criteria */

extern bool DATA_is_on_stack (
 DATA_t_stack_code  stackCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Return the position of the Nth occurrence of a stack element that matches the specified criteria */

extern unsigned int DATA_where_on_stack (
 DATA_t_stack_code   stackCode,
 unsigned int   occurrence,
 void          *p_dataPattern,
 unsigned int   patternSubsets,
 ... );

/* Find out how many data items on a stack meet the specified criteria */

extern unsigned int DATA_count_on_stack (
 DATA_t_stack_code  stackCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Is the stack empty? */

extern bool DATA_is_empty_stack (DATA_t_stack_code stackCode);

/* Obtain the number of data items in a stack */

extern unsigned int DATA_get_stack_height (DATA_t_stack_code stackCode);

/* Remove all data items in a stack */

extern void DATA_clear_stack (DATA_t_stack_code stackCode);

/* Delete a stack and its data */

extern void DATA_destroy_stack (DATA_t_stack_code stackCode);

/*--------------------*/
/* Methods for QUEUES */
/*--------------------*/

/* Set up an empty queue and return its numeric code */

extern DATA_t_queue_code DATA_new_queue (size_t dataSize, unsigned int maxDataItems);

extern DATA_t_queue_code DATA_new_queue2 (
 size_t       dataSize,
 unsigned int maxDataItems,
 unsigned int initialBlockSize,
 unsigned int resizeLimit,
 unsigned int resizeFactor );

/* Copy part of an existing queue into a new one and return its numeric code */

extern DATA_t_queue_code DATA_copy_queue (
 DATA_t_queue_code  queueCode,
 void         *p_dataItem,
 unsigned int  patternSubsets,
 ... );

/* Add a data item to the rear of the queue */

extern void DATA_join_queue (DATA_t_queue_code queueCode, void *p_dataItem);

/* Read the data item at the front of the queue without removing it */

extern void DATA_front_of_queue (DATA_t_queue_code queueCode, void *p_dataItem);

/* Remove the data item from the front of the queue and return the data */

extern void DATA_leave_queue (DATA_t_queue_code queueCode, void *p_dataItem);

/* Find out if a queue contains at least one data item that meets the specified criteria */

extern bool DATA_is_in_queue (
 DATA_t_queue_code  queueCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Return the position of the Nth occurrence of a queue element that matches the specified criteria */

extern unsigned int DATA_where_in_queue (
 DATA_t_queue_code   queueCode,
 unsigned int   occurrence,
 void          *p_dataPattern,
 unsigned int   patternSubsets,
 ... );

/* Find out how many data items in a queue meet the specified criteria */

extern unsigned int DATA_count_in_queue (
 DATA_t_queue_code  queueCode,
 void         *p_dataPattern,
 unsigned int  patternSubsets,
 ... );

/* Is the queue empty? */

extern bool DATA_is_empty_queue (DATA_t_queue_code queueCode);

/* Obtain the number of data items in a queue */

extern unsigned int DATA_get_queue_length (DATA_t_queue_code queueCode);

/* Remove all data items in a queue */

extern void DATA_clear_queue (DATA_t_queue_code queueCode);

/* Delete a queue and its data */

extern void DATA_destroy_queue (DATA_t_queue_code queueCode);


/*
-------------------------------------------------------------------------------
 DATA STRUCTURES: CREATION: PARAMETERS AND ARGUMENTS
-------------------------------------------------------------------------------

All data structures provided in this module are dynamically allocated and can
grow as needed during program execution. How they grow, or whether they are
allowed to grow at all, is determined by their creation parameters.

There are two versions of each data structure creation method. Version 2
allows finer control of the memory allocation strategy used during program
execution. The complete set of arguments is described below. Note that these
are all the arguments taken by the ARRAY, STACK and QUEUE creation methods;
the SET creation methods take extra arguments, described shortly.

 (1) dataSize: size in bytes of the data type that will be stored; can be
     obtained with a call to sizeof().

 (2) maxDataItems: If > 0, the data structure will not grow beyond this limit,
     otherwise the data structure may grow indefinitely (no upper limit).

 (3) initialBlockSize: the data structure is initialized with just enough space
     for this number of data items (a data item is a piece of data stored in
     a dynamically allocated data structure, e.g. a set element). If at any
     moment the data structure runs out of space it will automatically be
     given more space for new data items.

     In the case of arrays, if we try to store data in a position greater
     than its current size, the array will be extended accordingly, unless
     maxDataItems is less than the specified position.
     Therefore arrays must be used with care: storing a 10-byte data type in
     array position 100000 will cause the (re)allocation of 1MB of memory!

 (4) resizeLimit: number of times the data structure can been extended with
     the current value of initialBlockSize. The next memory reallocation will
     cause initialBlockSize to grow by a factor of resizeFactor:
        initialBlockSize *= resizeFactor
     NOTE: the initialBlockSize argument is an initial value only; it may be
     increased during program execution as the data structure gets repeatedly
     extended.

 (5) resizeFactor: as explained above.

Version 1 of creation methods take only arguments 1 and 2 (dataSize and
maxDataItems). Version 2 takes all five arguments.

EXAMPLE

  typedef struct { ... }
    t_record;
  DATA_t_queue_code
    queue_of_ints,
    queue_of_recs;

  queue_of_ints = DATA_new_queue (sizeof (int), 0);
  queue_of_recs = DATA_new_queue2 (sizeof (t_record), 10000, 20, 10, 3);

In this example, queue_of_ints may grow indefinitely. However, how it grows
(ie. how often and by how much each time) is handled automatically based on
default values compiled into the code.

On the other hand, queue_of_recs will start off with just enough space for
20 records. When we add the 21st record to the queue, it is automatically
extended by the same amount of 20 records, so now it is large enough to hold
40 records. The first 10 times it gets extended like this, the same amount of
extra space is added to the queue. The next time the queue needs extending
(11th time), the amount of extra space is tripled before the queue is actually
extended; initialBlockSize is now 60. This value will be used for the next 9
queue extensions. The next time the queue needs extending (21th time), the
amount of extra space is tripled again before the queue is extended again,
and so on. However, the queue will not get bigger than the maximum specified
size of 10000.

In order to create a fixed size (ie. non-extensible) data structure, call
version 2 of any data structure creation method and pass an argument of 0
to initialBlockSize, resizeLimit AND resizeFactor, but NOT to maxDataItems.
The data structure will be allocated the right amount of memory for
maxDataItems from the start and that will not change (unless of course the
data structure is cleared or destroyed). Calling the creation methods with
only one or two of the above parameters set to 0, rather than all three,
causes the method to fail.

SETS

The two set creation methods take the arguments described above plus a few
more. The extra arguments are used to define the set membership attribute
of that set's elements. That is to say, the extra arguments specify which
bytes of the data items stored in the set are relevant to decide whether a
new data item should be allowed in the set (remember that regular sets do
not allow repetition, as opposed to multisets). This is only meaningful when
the set elements are declared as compound data (such as structs or arrays
or both), in which case not all fields of a data item may be relevant to
establish set membership. If the set contains only simple data (char, int,
float, double) then the whole data item is relevant for set membership.

EXAMPLE

Suppose we create a set of records as declared below

  typedef struct {char c; int i; float f} record;

Also, suppose we don't care about data duplication in the char or float fields
but only in the int field. Even though the set contains records, we really
want it to behave as a set of ints. What defines set membership is the int
field alone. To create such a set with version 1 of the set creation method,
we should call

  setCode = DATA_new_set (sizeof (record), 0, 1, sizeof(R.c), sizeof(R.i));

The first two parameters deal with memory allocation for the set, while the
other three provide the data pattern that will be used to determine whether
new data items should be allowed into this set.

To create a set of characters only, we could call

  setCode = DATA_new_set (sizeof (char), 0, 0);

For further explanation, see section "DATA STRUCTURE ACCESS: PATTERN MATCHING"
below.

-------------------------------------------------------------------------------
 DATA STRUCTURE CREATION: RETURN VALUES
-------------------------------------------------------------------------------

All data structure creation methods return a numeric code. This code is the
only way to access the corresponding data structure.

-------------------------------------------------------------------------------
DATA STRUCTURE ACCESS: POSITION OF DATA IN A DATA STRUCTURE
-------------------------------------------------------------------------------

To store data in a array we need to say where to store it: the array index.
For other types of data structures, a position is NOT provided:
 - set:   data may be stored anywhere; the actual position is irrelevant;
 - stack: data are always stored at the top of the stack;
 - queue: data are always stored at the back of the queue.

When retrieving data from a data structure, the position parameter has
different meanings depending on the data structure involved. When retrieving
data from a...
 - array: the position is interpreted as the array index;
 - set:   the position is interpreted as an ordinal, eg. 3rd element of set;
 - stack: there is no position parameter; data always goes on/out of the top;
 - queue: no position parameter; new data is always stored in the back of
          the queue and data is retrieved from the front of the queue.

-------------------------------------------------------------------------------
DATA STRUCTURE ACCESS: LOOK-UP METHODS AND MULTIPLE DATA OCCURRENCES
-------------------------------------------------------------------------------

Most look-up methods are not affected by data duplication, either because
duplicate data are not allowed (they are stored only once, as in sets), or
because there is no doubt about which duplicate can be accessed (top of stack,
front of the queue). Also, the DATA_is_on_stack() and DATA_is_in_queue() methods return
true if the data is on the stack or in the queue, regardless of how many copies
of the data these data structures hold.

There is one exception to this. Arrays may store multiple copies of the same
data (as opposed to sets), but there is no pre-defined order for accessing them
(as opposed to stacks and queues). Therefore, when looking for data in a array,
we may need to specify which one of multiple copies we are looking for.

The method DATA_count_in_array() returns the number of copies ("occurrences") of a
specific data item that are stored in the array. The method DATA_where_in_array()
takes an "occurrence" argument to indicate the particular instance we are
looking for. Its type is unsigned int and its value must be greater than zero
but no greater than the number of data items stored in the array.

-------------------------------------------------------------------------------
DATA STRUCTURE ACCESS: PATTERN MATCHING
-------------------------------------------------------------------------------

When looking for specific data in a array of integers or queue of doubles, for
example, the entire data item (all of its bytes) must be analysed to ensure
that a match is found. That is because we can only find an integer or a double
by finding all of their bytes. Generally speaking, two integers or doubles are
equal if and only if all of their bytes are equal.

This is not necessarily the case when the data structure stores more complex
data. In a array of records it may not make sense to consider all fields in
order to decide whether a record matches what is being looked for.

PARTIAL MATCHES

Data look-up methods can be told to consider only subsets of the stored data
when looking for mathches. This is a PARTIAL MATCH, as opposed to a full match
which requires all data bytes to match. For example, we may create a set of
structs with 3 data fields and specify that set membership is based on data
fields "1 AND 3" only. Such set may contain several elements with duplicate
values in any data field, but not in data fields 1 and 3 simultaneously.
Each combination of "data in field 1" AND "data in field 3" must be unique;
this is an intrinsic feature of all regular sets (as opposed to multisets,
which allow repetition). When we try to add a new element to this set, a look
up method will check if an existing set element holds in its data fields 1 and
3 the same data as the new element being added to the set. If that is the case
then the new element will not be added.

DATA ITEMS AND DATA PATTERNS

Some data structure access methods require a data item as a parameter. The
DATA_store_in_array() method, for example, takes a data item and stores it in a
given array position. Other methods accept, or even require, a data pattern
against which matches will be attempted. The DATA_where_in_array() method described
below is an example; the p_dataPattern parameter points to a data pattern that
will drive the matching process.

Data passed as parameters to these methods may be of two kinds: "real" data
(or DATA ITEMS) and "dummy" data (or DATA PATTERNS). Both are pointers to the
same user-defined data type; the difference is that a DATA ITEM actually holds
real data, to be stored in a data structure. When passed as argument to a data
structure method, it requires no further specification of how it should be
processed. On the other hand, a DATA PATTERN is not actually stored in the
data structure; it only holds the byte pattern that will determine which data
items will be operated on. When passed as argument to a data structure method,
it is not sufficient to fully specify how real data items should be processed;
the PATTERN MATCH PARAMETERS (match type and pattern subsets) are also needed.
Of course a DATA PATTERN may be identical with a DATA ITEM, in which case the
PATTERN MATCH PARAMETERS would consist of a DATA PATTERN, pattern subsets = 0
and no match type.

Some methods take a DATA ITEM, some take a DATA PATTERN and some take neither,
but no method takes both. Also, nearly all methods that take a DATA PATTERN
also take PATTERN MATCH PARAMETERS and vice-versa. The two exceptions are
DATA_new_set() and DATA_new_set2(). These methods take PATTERN MATCH PARAMETERS that
define membership for every set, so there's no need for a DATA PATTERN. These
pattern match parameters are provided only once at set creation, stored as
fixed set attributes, and used whenever the set is used in any operation.

MATCH PARAMETERS

These are used to specify how a partial match is to be performed. Separate
portions of a data pattern, or PATTERN SUBSETS, are specified by a variable
argument list. Consider the method prototype below:

  unsigned int DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,
    (unsigned int)       occurrence,
    (void *)             p_dataPattern,
    (int)                patternSubsets,
    ... );

The patternSubsets parameter indicates how many different portions, or subsets,
of each data item will be used. The actual pattern subset specification comes
next (indicated by the "...") in the following form:
  - if more than one pattern subset is to be used, an indication of how they
    should be combined
  - for each pattern subset, a pair of integers indicating:
    - the position of its first byte in the data item, and
    - its size in bytes.

Calls to the method above will have the general form

  DATA_where_in_array (
    (DATA_t_array_code)        arrayCode,
    (unsigned int)             occurrence,
    (void *)                   p_dataPattern,
    (int)                      patternSubsets,
    (DATA_t_patternMatchType)  patternMatchType,
    (int)                      patternSubsetStart_1,
    (int)                      patternSubsetSize_1,
    (int)                      patternSubsetStart_2,
    (int)                      patternSubsetSize_2,
     ...                       ...
    (int)                      patternSubsetStart_N,
    (int)                      patternSubsetSize_N );

where

  - patternMatchType works like a boolean operator that will combine the
    results of partial matches; must be either DATA_t_AND, DATA_t_OR or DATA_t_XOR;
  - patternSubsetStart_N is the position in the data pattern where the Nth
    subset starts (positions are numbered 0 onwards);
  - patternSubsetSize_N is the number of bytes in the Nth pattern subset
    (N = patternSubsets).

Note that

  - If patternSubsets = 0 then the entire data item will need to match (no data
    subsets will be used) and no further arguments should be provided.
  - If patternSubsets = 1 then a patternMatchType CANNOT be specified; it would
    make no sense anyway as there is only one data block to consider, therefore
    there are not multiple partial matches to combine.
  - If patternSubsets > 1 then a boolean operator MUST be provided.
  - If patternSubsetStart_I = 0 then pattern subset I starts at the begining
    of the data item.
  - If patternSubsetStart_I, patternSubsetSize_I or both are < 0 then pattern
    subset I specifies a negative match, i.e. the pattern subset MUST NOT MATCH
    the data item in order for a positive match to occur. This is equivalent to
    the *NOT* boolean operation.

EXAMPLES

Suppose we create a dynamic array of records as declared below

  typedef struct {char c; int i; float f} record;
  arrayCode = DATA_new_array (sizeof (record), 0);

We add records to the array and then decide to look for records that match
(or don't match) parts of a particular pattern, say

  record R = { 'A', 1, 2.3 };

In this scenario:

...................................................................................................................................................
  DATA_where_in_array (arrayCode, 4, &R, 0);

    will consider entire data items rather than parts of them, and will return
    the position of the 4th record in the array that matches all fields of R.
    NOTE that this will have the same effect as any one of the calls below:

  DATA_where_in_array (arrayCode, 4, &R, 1, 0, sizeof(R));
  DATA_where_in_array (arrayCode, 4, &R, 1, 0, sizeof(R.c) + sizeof(R.i) + sizeof (R.f));
  DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,                       //
    (unsigned int)  4,                                    //
    (void *)        &R,                                   //
    (int)           1,                                    //
    (int)           ((void *) &(R.c) - (void *) &(R)),    // This yields 0 as R.c and R start at the same memory address
    (int)           sizeof(R.c)+sizeof(R.i)+sizeof(R.f)); //
  DATA_where_in_array (arrayCode, 4, &R, 2, DATA_t_AND, 0, sizeof(R.c), sizeof (R.c), sizeof(R.i) + sizeof (R.f));
  DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,                       //
    (unsigned int)  4,                                    //
    (void *)        &R,                                   //
    (int)           2,                                    //
    (DATA_t_patternMatchType)   DATA_t_AND,               //
    (int)           ((void *) &(R.c) - (void *) &(R)),    // This yields 0 as R.c and R start at the same memory address
    (int)           sizeof (R.c),                         //
    (int)           ((void *) &(R.i) - (void *) &(R)),    // This yields the same result as sizeof(R.c) as R.i starts where R.c ends
    (int)           sizeof (R.i) + sizeof (R.f) );        //
  DATA_where_in_array (arrayCode, 4, &R, 2, DATA_t_AND, 0, sizeof(R.c)+sizeof(R.i), sizeof(R.c)+sizeof(R.i), sizeof (R.f));
  DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,                       //
    (unsigned int)  4,                                    //
    (void *)        &R,                                   //
    (int)           2,                                    //
    (DATA_t_patternMatchType)   DATA_t_AND,               //
    (int)           ((void *) &(R.c) - (void *) &(R)),    // This yields 0 as R.c and R start at the same memory address
    (int)           sizeof (R.c) + sizeof (R.i) ,         //
    (int)           ((void *) &(R.f) - (void *) &(R)),    // This yields the same result as sizeof(R.c)+sizeof(R.i) as R.f starts where R.i ends
    (int)           sizeof (R.f) );                       //
  DATA_where_in_array (arrayCode, 4, &R, 3, DATA_t_AND, 0, sizeof(R.c), sizeof(R.c), sizeof(R.i), sizeof(R.c)+sizeof(R.i), sizeof(R.f));
  DATA_where_in_array (arrayCode, 4, &R, 3, DATA_t_AND, 0, sizeof(R.c), sizeof(R.c), sizeof(R.i), sizeof(R)-sizeof(R.f), sizeof(R.f));
  DATA_where_in_array (arrayCode, 4, &R, 3, DATA_t_AND, 0, sizeof(R.c), sizeof(R)-sizeof(R.f)-sizeof(R.i), sizeof(R.i), sizeof(R)-sizeof(R.f), sizeof(R.f));
  DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,                       //
    (unsigned int)  4,                                    //
    (void *)        &R,                                   //
    (int)           3,                                    //
    (DATA_t_patternMatchType)   DATA_t_AND,               //
    (int)           ((void *) &(R.c) - (void *) &(R)),    // This example shows how to access individual
    (int)           sizeof (R.c),                         // fields of the data structure, making it
    (int)           ((void *) &(R.i) - (void *) &(R)),    // easy to build up complex match patterns
    (int)           sizeof (R.i),                         //
    (int)           ((void *) &(R.f) - (void *) &(R)),    //
    (int)           sizeof (R.f) );                       //
...................................................................................................................................................
  DATA_where_in_array (arrayCode, 2, &R, 1, sizeof(R.c)+sizeof(R.i), sizeof(R.f));

    will look for the 2nd record in the array that has a 2.3 in the "f" field.
    NOTE that this will have the same effect as any one of the calls below:

  DATA_where_in_array (arrayCode, 2, &R, 1, sizeof(R)-sizeof(R.f), sizeof(R.f));
  DATA_where_in_array (arrayCode, 2, &R, 1, sizeof(R.c)+sizeof(R.i), sizeof(R.f));
  DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,                       //
    (unsigned int)  2,                                    //
    (void *)        &R,                                   //
    (int)           1,                                    //
    (int)           ((void *) &(R.f) - (void *) &(R)),    // This yields the same result as sizeof(R.c)+sizeof(R.i) as R.f starts where R.i ends
    (int)           sizeof (R.f) );                       //
...................................................................................................................................................
  DATA_where_in_array (arrayCode, 3, &R, 2, DATA_t_AND, 0, sizeof(R.c), sizeof(R.c)+sizeof(R.i), sizeof(R.f));

    will look for the 3rd record in the array that meets ALL of the criteria below:
    - it has a 'A' in the "c" field, AND
    - it has a 2.3 in the "f" field.
    NOTE that this will have the same effect as

  DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,                       //
    (unsigned int)  3,                                    //
    (void *)        &R,                                   //
    (int)           2,                                    //
    (DATA_t_patternMatchType)   DATA_t_AND,               //
    (int)           ((void *) &(R.c) - (void *) &(R)),    // This yields 0 as R.c and R start at the same memory address
    (int)           sizeof (R.c),                         //
    (int)           ((void *) &(R.f) - (void *) &(R)),    // This yields the same result as sizeof(R.c)+sizeof(R.i) as R.f starts where R.i ends
    (int)           sizeof (R.f) );                       //
...................................................................................................................................................
  DATA_where_in_array (arrayCode, 2, &R, 3, DATA_t_OR, 0, sizeof(R.c), sizeof(R.c), sizeof(R.i), sizeof(R.c)+sizeof(R.i), sizeof(R.f));

    will look for the 2nd record in the array that meets AT LEAST one of the criteria below:
      - it has a 'A' in the "c" field, OR
      - it has a 1 in the "i" field, OR
      - it has a 2.3 in the "f" field.
    NOTE that this will have the same effect as

  DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,                       //
    (unsigned int)  2,                                    //
    (void *)        &R,                                   //
    (int)           3,                                    //
    (DATA_t_patternMatchType)   DATA_t_OR,                //
    (int)           ((void *) &(R.c) - (void *) &(R)),    // This yields 0 as R.c and R start at the same memory address
    (int)           sizeof (R.c),                         //
    (int)           ((void *) &(R.i) - (void *) &(R)),    // This yields the same result as sizeof(R.c) as R.i starts where R.c ends
    (int)           sizeof (R.i),                         //
    (int)           ((void *) &(R.f) - (void *) &(R)),    // This yields the same result as sizeof(R.c)+sizeof(R.i) as R.f starts where R.i ends
    (int)           sizeof (R.f) );                       //
...................................................................................................................................................
  DATA_where_in_array (arrayCode, 1, &R, 3, DATA_t_XOR, 0, sizeof(R.c), sizeof(R.c), sizeof(R.i), sizeof(R.c)+sizeof(R.i), sizeof(R.f));

    will look for the 1st record in the array that meets EXACTLY one of the criteria below:
      - it has a 'A' in the "c" field, OR
      - it has a 1 in the "i" field, OR
      - it has a 2.3 in the "f" field,
    but NOT any combination of the above.
...................................................................................................................................................
  DATA_where_in_array (arrayCode, 3, &R, 2, DATA_t_AND, 0, -sizeof(R.c), sizeof(R.c)+sizeof(R.i), -sizeof(R.f));

    will look for the 3rd record in the array that meets ALL of the criteria below:
    - it DOES NOT have a 'A' in the "c" field, AND
    - it DOES NOT have a 2.3 in the "f" field.
    NOTE that this will have the same effect as any one of the calls below:

  DATA_where_in_array (arrayCode, 3, &R, 2, DATA_t_AND, 0, -sizeof(R.c), -(sizeof(R.c)+sizeof(R.i)), sizeof(R.f));
  DATA_where_in_array (arrayCode, 3, &R, 2, DATA_t_AND, 0, -sizeof(R.c), -(sizeof(R.c)+sizeof(R.i)), -sizeof(R.f));
  DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,                       //
    (unsigned int)  3,                                    //
    (void *)        &R,                                   //
    (int)           2,                                    //
    (DATA_t_patternMatchType)   DATA_t_AND,               //
    (int)           ((void *) &(R.c) - (void *) &(R)),    // This yields 0 as R.c and R start at the same memory address
    (int)           sizeof (R.c),                         //
    (int)           -(((void *) &(R.f) + (void *) &(R))), // This yields the same result as -(sizeof(R.c)+sizeof(R.i)) as R.f starts where R.i ends
    (int)           sizeof (R.f) );                       // and this number is < 0 therefore this specifies a negative match
  DATA_where_in_array (
    (DATA_t_array_code)  arrayCode,                       //
    (unsigned int)  3,                                    //
    (void *)        &R,                                   //
    (int)           2,                                    //
    (DATA_t_patternMatchType)   DATA_t_AND,               //
    (int)           ((void *) &(R.c) - (void *) &(R)),    // This yields 0 as R.c and R start at the same memory address
    (int)           sizeof (R.c),                         //
    (int)           ((void *) &(R) - (void *) &(R.f)),    // This yields the same result as -(sizeof(R.c)+sizeof(R.i)) as R.f starts where R.i ends
    (int)           sizeof (R.f) );                       // and this number is < 0 therefore this specifies a negative match
...................................................................................................................................................

LIST OF METHODS

The list below indicates, for each method, whether it works with data items,
data patterns and pattern subsets. The ones that work with pattern subsets
take variable length argument lists.

NOTE that the collection of methods varies from one type of data structure
to another. For example, there is a method for removing a data item from a
set, but not one for removing data items from an array.

     +----------------------------------+---------+---------+---------+
     |                                  |  DATA   |  DATA   | PATTERN |
     |                                  |  ITEM   | PATTERN | SUBSETS |
     +----------------------------------+---------+---------+---------+
     | Methods for ARRAYS                                             |
     +----------------------------------+---------+---------+---------+
     |   DATA_new_array                 |   no    |   no    |   no    |
     |   DATA_new_array2                |   no    |   no    |   no    |
     |   DATA_copy_array                |   no    |   YES   |   YES   |
     |   DATA_store_in_array            |   YES   |   no    |   no    |
     |   DATA_multiple_store_in_array   |   YES   |   no    |   no    |
     |   DATA_read_from_array           |   no    |   no    |   no    |
     |   DATA_is_in_array               |   no    |   YES   |   YES   |
     |   DATA_where_in_array            |   no    |   YES   |   YES   |
     |   DATA_count_in_array            |   no    |   YES   |   YES   |
     |   DATA_is_empty_array            |   no    |   no    |   no    |
     |   DATA_get_array_size            |   no    |   no    |   no    |
     |   DATA_clear_array               |   no    |   no    |   no    |
     |   DATA_destroy_array             |   no    |   no    |   no    |
     +----------------------------------+---------+---------+---------+
     | Methods for SETS                                               |
     +----------------------------------+---------+---------+---------+
     |   DATA_new_set                   |   no    |   no    |   YES   |
     |   DATA_new_set2                  |   no    |   no    |   YES   |
     |   DATA_copy_set                  |   no    |   YES   |   YES   |
     |   DATA_add_to_set                |   YES   |   no    |   no    |
     |   DATA_read_from_set             |   no    |   no    |   no    |
     |   DATA_remove_from_set           |   no    |   YES   |   YES   |
     |   DATA_is_in_set                 |   no    |   YES   |   YES   |
     |   DATA_where_in_set              |   no    |   YES   |   YES   |
     |   DATA_count_in_set              |   no    |   YES   |   YES   |
     |   DATA_is_empty_set              |   no    |   no    |   no    |
     |   DATA_get_set_size              |   no    |   no    |   no    |
     |   DATA_set_union                 |   no    |   no    |   no    |
     |   DATA_set_intersection          |   no    |   no    |   no    |
     |   DATA_set_difference            |   no    |   no    |   no    |
     |   DATA_set_symmetric_difference  |   no    |   no    |   no    |
     |   DATA_are_equal_sets            |   no    |   no    |   no    |
     |   DATA_is_subset                 |   no    |   no    |   no    |
     |   DATA_clear_set                 |   no    |   no    |   no    |
     |   DATA_destroy_set               |   no    |   no    |   no    |
     +----------------------------------+---------+---------+---------+
     | Methods for STACKS                                             |
     +----------------------------------+---------+---------+---------+
     |   DATA_new_stack                 |   no    |   no    |   no    |
     |   DATA_new_stack2                |   no    |   no    |   no    |
     |   DATA_copy_stack                |   no    |   YES   |   YES   |
     |   DATA_push_onto_stack           |   YES   |   no    |   no    |
     |   DATA_top_of_stack              |   no    |   no    |   no    |
     |   DATA_bottom_of_stack           |   no    |   no    |   no    |
     |   DATA_pop_from_stack            |   no    |   no    |   no    |
     |   DATA_is_on_stack               |   no    |   YES   |   YES   |
     |   DATA_where_on_stack            |   no    |   YES   |   YES   |
     |   DATA_count_on_stack            |   no    |   YES   |   YES   |
     |   DATA_is_empty_stack            |   no    |   no    |   no    |
     |   DATA_get_stack_height          |   no    |   no    |   no    |
     |   DATA_clear_stack               |   no    |   no    |   no    |
     |   DATA_destroy_stack             |   no    |   no    |   no    |
     +----------------------------------+---------+---------+---------+
     | Methods for QUEUES                                             |
     +----------------------------------+---------+---------+---------+
     |   DATA_new_queue                 |   no    |   no    |   no    |
     |   DATA_new_queue2                |   no    |   no    |   no    |
     |   DATA_copy_queue                |   no    |   YES   |   YES   |
     |   DATA_join_queue                |   YES   |   no    |   no    |
     |   DATA_front_of_queue            |   no    |   no    |   no    |
     |   DATA_back_of_queue             |   no    |   no    |   no    |
     |   DATA_leave_queue               |   no    |   no    |   no    |
     |   DATA_is_in_queue               |   no    |   YES   |   YES   |
     |   DATA_where_in_queue            |   no    |   YES   |   YES   |
     |   DATA_count_in_queue            |   no    |   YES   |   YES   |
     |   DATA_is_empty_queue            |   no    |   no    |   no    |
     |   DATA_get_queue_length          |   no    |   no    |   no    |
     |   DATA_clear_queue               |   no    |   no    |   no    |
     |   DATA_destroy_queue             |   no    |   no    |   no    |
     +----------------------------------+---------+---------+---------+

-------------------------------------------------------------------------------
VARIABLE LENGTH ARGUMENT LISTS
-------------------------------------------------------------------------------

We have described above one type of method that takes variable length argument
lists (the ones that can work with data patterns).

Another method takes variable length argument lists: DATA_multiple_store_in_array().
It stores multiple copies of a data item in specified positions of a array.
Target positions are passed to it in a variable length argument list, in the
form of pairs of integers (from, to) specifying ranges of array positions.

-------------------------------------------------------------------------------
DATA STRUCTURE HOUSEKEEPING - CLEARING DATA AND FREEING MEMORY
-------------------------------------------------------------------------------

Any data structure can have its contents cleared: all data items are removed
but the data structure itself, now empty, is still available.

If the data structure is no longer needed, the memory allocated to it may be
freed, after which that particular data structure is no longer accessible.

*/

#endif  /* ifndef _DATASTRUCTS_DOT_H_ */
