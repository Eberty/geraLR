/*
*---------------------------------------------------------------------
*
*   File         : error.c
*   Created      : 1995-03-05
*   Last Modified: 2012-05-27
*
*   Error management.
*
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "error.h"

/*
*---------------------------------------------------------------------
*
*   INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

/*                                */
/* General purpose error messages */
/*                                */

char
  ERROR_auxErrorMsg [ERROR_maxErrorMsgSize],
     ERROR_errorMsg [ERROR_maxErrorMsgSize];

/*                     */
/* Function prototypes */
/*                     */

void ERROR_short_fatal_error (
 const char *errorMsg ) __attribute__ ((noreturn));

void ERROR_fatal_error (
 int         errorCode,
 const char *moduleName,
 const char *procName,
 const char *errorMsg ) __attribute__ ((noreturn));

void ERROR_no_memory (
 int         errorCode,
 const char *moduleName,
 const char *procName,
 const char *varName ) __attribute__ ((noreturn));

void ERROR_check_range_char (
 char        value,
 char        min,
 char        max,
 const char *varName,
 const char *procName,
 const char *moduleName );

void ERROR_check_range_int (
 int         value,
 int         min,
 int         max,
 const char *varName,
 const char *procName,
 const char *moduleName );

void ERROR_check_range_double (
 double        value,
 double        min,
 double        max,
 const char   *varName,
 const char   *procName,
 const char   *moduleName );

Bool ERROR_str2all (
 const char       *string,
 ERROR_t_dataType  finalType,
 void             *p_result );

/*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION (not visible from other modules)
*
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
*   Prints out a short error message and aborts program.
*---------------------------------------------------------------------
*/

void ERROR_short_fatal_error (const char *errorMsg)
{
 fprintf (stderr, "\n%s\n", errorMsg);
 fflush (NULL);
 fclose (NULL);
 exit (EXIT_FAILURE);
}

/*
*---------------------------------------------------------------------
*   Prints out a detailed error message and aborts program.
*---------------------------------------------------------------------
*/

void ERROR_fatal_error (
 int         errorCode,
 const char *moduleName,
 const char *procName,
 const char *errorMsg )
{
 char
   auxStr [ERROR_maxErrorMsgSize];

 fprintf (stderr, "\n");
 if (errorCode)
   snprintf (auxStr, ERROR_maxErrorMsgSize, " %d", errorCode);
 else
   strncpy (auxStr, "", ERROR_maxErrorMsgSize);
 fprintf (stderr, "  Error%s in %s::%s()\n", auxStr, moduleName, procName);
 if (strcmp (errorMsg, ""))
   fprintf (stderr, "  %s\n", errorMsg);
 if (errorCode)
   fprintf (stderr, "  %s\n", strerror (errorCode));
 fprintf (stderr, "\n");
 fflush (NULL);
 fclose (NULL);
 exit (EXIT_FAILURE);
}

/*
*---------------------------------------------------------------------
*   Special purpose procedure: malloc() out-of-memory error
*---------------------------------------------------------------------
*/

void ERROR_no_memory (
 int         errorCode,
 const char *moduleName,
 const char *procName,
 const char *varName )
{
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot malloc \"%s\"", varName);
 ERROR_fatal_error (errorCode, moduleName, procName, ERROR_auxErrorMsg);
}

/*
*---------------------------------------------------------------------
*   Check if a character is within the expected range
*---------------------------------------------------------------------
*/

void ERROR_check_range_char (
 char        value,
 char        min,
 char        max,
 const char *varName,
 const char *procName,
 const char *moduleName )
{
 if (value < min) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Bad %s (`%c'); must be >= `%c'",
     varName, value, min);
   ERROR_fatal_error (0, moduleName, procName, ERROR_auxErrorMsg);
 }
 if (value > max) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Bad %s (`%c'); must be <= `%c'",
     varName, value, max);
   ERROR_fatal_error (0, moduleName, procName, ERROR_auxErrorMsg);
 }
}

/*
*---------------------------------------------------------------------
*   Check if an integer is within the expected range
*---------------------------------------------------------------------
*/

void ERROR_check_range_int (
 int         value,
 int         min,
 int         max,
 const char *varName,
 const char *procName,
 const char *moduleName )
{
 if (value < min) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Bad %s (%d); must be >= %d", varName, value, min);
   ERROR_fatal_error (0, moduleName, procName, ERROR_auxErrorMsg);
 }
 if (value > max) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Bad %s (%d); must be <= %d", varName, value, max);
   ERROR_fatal_error (0, moduleName, procName, ERROR_auxErrorMsg);
 }
}

/*
*---------------------------------------------------------------------
*   Check if a double is within the expected range
*---------------------------------------------------------------------
*/

void ERROR_check_range_double (
 double        value,
 double        min,
 double        max,
 const char   *varName,
 const char   *procName,
 const char   *moduleName )
{
 if (value < min) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Bad %s (%f); must be >= %f", varName, value, min);
   ERROR_fatal_error (0, moduleName, procName, ERROR_auxErrorMsg);
 }
 if (value > max) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Bad %s (%f); must be <= %f", varName, value, max);
   ERROR_fatal_error (0, moduleName, procName, ERROR_auxErrorMsg);
 }
}

/*
*---------------------------------------------------------------------
*   Convert a string to another data type
*---------------------------------------------------------------------
*/

#ifdef  PARTIAL_MSG_LENGTH
# undef PARTIAL_MSG_LENGTH
#endif
#define PARTIAL_MSG_LENGTH 50

Bool ERROR_str2all (const char *string, ERROR_t_dataType finalType, void *p_result)
{
 char
   *tail,
    partialMsg [PARTIAL_MSG_LENGTH];
 Bool
   b_error;
 long int
   var_Long;
 unsigned long int
   var_ULong;
 double
   var_Double;
 int
   typeIndex;
 static char
   *typeName[] = {
     "long int",
     "unsigned long int",
     "double",
     "char",
     "string",
     "unknown"
   };

 strcpy (partialMsg, "");
 b_error = FALSE;
 switch (finalType) {
   case (ERROR_t_long_int) :

     /* Get rid of leading 0s to prevent strtoul()      */
     /* from interpreting the string as an octal number */

     while ((string[0] == '0') && (isdigit (string[1])))
       string++;

     typeIndex = 0;
     errno = 0;
     var_Long = strtol (string, &tail, 0);
     b_error = (errno != 0) || ((var_Long==0) && (tail==string)) || (strlen(tail)>0);
     if (b_error) {
       strcpy (partialMsg, "strtol failed. ");
       * (long int *) p_result = (long int) 0;
     }
     else
       * (long int *) p_result = var_Long;
     break;
   case (ERROR_t_ulong_int) :

     /* Get rid of leading 0s to prevent strtoul()      */
     /* from interpreting the string as an octal number */

     while ((string[0] == '0') && (isdigit (string[1])))
       string++;

     typeIndex = 1;
     errno = 0;
     var_ULong = strtoul (string, &tail, 0);
     b_error = (errno != 0) || ((var_ULong==0) && (tail==string)) || (strlen(tail)>0);
     if (b_error) {
       strncpy (partialMsg, "strtoul failed. ", PARTIAL_MSG_LENGTH);
       * (unsigned long int *) p_result = (unsigned long int) 0;
     }
     else
       * (unsigned long int *) p_result = var_ULong;
     break;
   case (ERROR_t_double) :
     typeIndex = 2;
     errno = 0;
     var_Double = strtod (string, &tail);
	 /* GCC complains about the following test:                                              */
   /*   b_error = (errno != 0) || ((var_Double==0) && (tail==string)) || (strlen(tail)>0); */
	 /*                                         ^                                            */
	 /*   comparing floating point with == or != is unsafe [-Wfloat-equal]                   */
	 /* so we changed it                                                                     */
     b_error = (errno != 0) || (tail==string) || (strlen(tail)>0);
     if (b_error) {
       strncpy (partialMsg, "strtod failed. ", PARTIAL_MSG_LENGTH);
       * (double *) p_result = (double) 0;
     }
     else
       * (double *) p_result = var_Double;
     break;
   case (ERROR_t_char) :
     typeIndex = 3;
     if (strlen (string) != 1) {
       snprintf (partialMsg, PARTIAL_MSG_LENGTH, "Bad string lenght (%d). ", (int) strlen (string));
       * (char *) p_result = (char) 0;
       b_error = TRUE;
     }
     else if (! isprint (*string)) {
       strncpy (partialMsg, "isprint failed. ", PARTIAL_MSG_LENGTH);
       * (char *) p_result = (char) 0;
       b_error = TRUE;
     }
     else
       * (char *) p_result = *string;
     break;
   case (ERROR_t_string) :
     typeIndex = 4;
     if (strlen (string) < 1) {
       snprintf (partialMsg, PARTIAL_MSG_LENGTH, "Bad string lenght (%d). ", (int) strlen (string));
       * (char **) p_result = (char *) NULL;
       b_error = TRUE;
     }
     else {
       errno = 0;
       * (char **) p_result = strdup (string);
       if (errno != 0) {
         snprintf (partialMsg, PARTIAL_MSG_LENGTH, "strdup(\"%s\") failed. ", string);
         * (char **) p_result = (char *) NULL;
         b_error = TRUE;
       }
     }
     break;
   default:
     typeIndex = 5;
     strncpy (partialMsg, "Invalid data type. ", PARTIAL_MSG_LENGTH);
     b_error = TRUE;
     break;
 }
 if (b_error)
   snprintf (
     ERROR_auxErrorMsg,
	 ERROR_maxErrorMsgSize,
     "ERROR_str2all(): %sCannot convert \"%s\" to %s",
     partialMsg,
     string,
     typeName [typeIndex] );
 else
   strcpy (ERROR_auxErrorMsg, "");
 return (! b_error);
}

#undef PARTIAL_MSG_LENGTH
