/*
*---------------------------------------------------------------------
*
*   File         : error.h
*   Created      : 1995-03-05
*   Last Modified: 2012-05-27
*
*   Error management.
*
*---------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _ERROR_DOT_H_
#define _ERROR_DOT_H_

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <stdbool.h>
#include <errno.h>

/*
*---------------------------------------------------------------------
*   Definition of constants and macros
*---------------------------------------------------------------------
*/

#define ERROR_maxErrorMsgSize  1000

/* strdup is not standard C so let's give the compiler a */
/* prototype to prevent it from throwing ugly warnings   */

extern char *strdup(const char *str);

/*
*---------------------------------------------------------------------
*   Type definitions
*---------------------------------------------------------------------
*/

/*                                                  */
/* Used by the conversion procedure ERROR_str2all() */
/*                                                  */

typedef enum {
  ERROR_t_long_int,
  ERROR_t_ulong_int,
  ERROR_t_double,
  ERROR_t_char,
  ERROR_t_string
}
  ERROR_t_dataType;

/*
*---------------------------------------------------------------------
*   Data structures and variables
*---------------------------------------------------------------------
*/

/*                                */
/* General purpose error messages */
/*                                */

extern char
  ERROR_auxErrorMsg [ERROR_maxErrorMsgSize],
     ERROR_errorMsg [ERROR_maxErrorMsgSize];

/*
*---------------------------------------------------------------------
*   Procedures defined in "error.c"
*---------------------------------------------------------------------
*/

extern void ERROR_short_fatal_error (
 const char *ERROR_errorMsg ) __attribute__ ((noreturn));

extern void ERROR_fatal_error (
 int         errorCode,
 const char *moduleName,
 const char *procName,
 const char *ERROR_errorMsg ) __attribute__ ((noreturn));

extern void ERROR_no_memory (
 int         errorCode,
 const char *moduleName,
 const char *procName,
 const char *varName ) __attribute__ ((noreturn));

extern void ERROR_check_range_char (
 char        value,
 char        min,
 char        max,
 const char *varName,
 const char *procName,
 const char *moduleName );

extern void ERROR_check_range_int (
 int         value,
 int         min,
 int         max,
 const char *varName,
 const char *procName,
 const char *moduleName );

extern void ERROR_check_range_double (
 double        value,
 double        min,
 double        max,
 const char   *varName,
 const char   *procName,
 const char   *moduleName );

extern bool ERROR_str2all (
 const char       *string,
 ERROR_t_dataType  finalType,
 void             *p_result );

#endif /* ifndef _ERROR_DOT_H_ */
