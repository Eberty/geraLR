/*
*-----------------------------------------------------------------------
*
*   File         : command_line.c
*   Created      : 1995-03-05
*   Last Modified: 2012-05-27
*
*   Management of command line arguments.
*
*-----------------------------------------------------------------------
*/

/*
*-----------------------------------------------------------------------
*   INCLUDE FILES
*-----------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

#include "mystrings.h"
#include "error.h"
#include "commline.h"

/*
*---------------------------------------------------------------------
*
*   INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
*   Function prototypes
*---------------------------------------------------------------------
*/

void  COMMLINE_free_commLine_data     (void);
Bool  COMMLINE_argPos2argVal          (int arg_pos, void *arg_val);
void  COMMLINE_display_usage          (void);
char *COMMLINE_get_commLine_error     (void);
char *COMMLINE_get_program_short_name (char *full_name);
Bool  COMMLINE_optId2optUses          (int opt_ID,  int *uses);
Bool  COMMLINE_optPos2optUse          (int opt_pos, int *opt_ID,  int  *opt_use);
Bool  COMMLINE_optUse2optArg          (int opt_ID,  int  opt_use, void *opt_arg);
Bool  COMMLINE_optUse2optPos          (int opt_ID,  int  opt_use, int  *opt_pos);
Bool  COMMLINE_parse_commLine         (int argc, char *argv[], int totOpts,...);
int   COMMLINE_total_options          (void);
int   COMMLINE_total_non_opt_args     (void);

void COMMLINE_validate_boolean_option (
 Bool   *p_var,
 int     optIndex,
 Bool    defaultValue,
 char   *varName,
 char   *procName,
 char   *progName );

void COMMLINE_validate_int_option (
 int  *p_var,
 int   optIndex,
 int   defaultValue,
 int   minValue,
 int   maxValue,
 char *varName,
 char *procName,
 char *progName );

void COMMLINE_validate_short_int_option (
 short int *p_var,
 int        optIndex,
 short int  defaultValue,
 short int  minValue,
 short int  maxValue,
 char      *varName,
 char      *procName,
 char      *progName );

void COMMLINE_validate_double_option (
 double *p_var,
 int     optIndex,
 double  defaultValue,
 double  minValue,
 double  maxValue,
 char   *varName,
 char   *procName,
 char   *progName );

void COMMLINE_validate_string_option (
 char  **p_var,
 int     optIndex,
 char   *defaultValue,
 int     minLength,
 int     maxLength,
 char   *varName,
 char   *procName,
 char   *progName );

void COMMLINE_validate_oneOf_string_options (
 char  **p_var,
 int     optIndex,
 int     optUse,
 char   *defaultValue,
 int     minLength,
 int     maxLength,
 char   *varName,
 char   *procName,
 char   *progName );

/*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION (not visible from other modules)
*
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
*   Constants
*---------------------------------------------------------------------
*/

/* Should we allow empty strings for long option names? */
/*   If so, leave the following #define as it is.       */
/*   If not, comment it out.                            */

#define ALLOW_EMPTY_OPT_LONG_NAMES

/*
*---------------------------------------------------------------------
*   Variables
*---------------------------------------------------------------------
*/

/* Options may be used in their short or long forms */

typedef enum {
  t_opt_size_short,
  t_opt_size_long
}
  t_option_size;

/* Info on an actual option */

typedef struct {
  int
    formalIndex,
    actualIndex;
  t_option_size
    size;
  char
   *argStr;
  struct t_optUse
   *next;
}
  t_optUse;

/* Info on a formal option */

typedef struct {
  int
    optID;
  char
    optChar,
   *optStr;
  COMMLINE_t_optArgType
    optArgType;
  int
    optMinUses,
    optMaxUses,
    optTotUses;
  t_optUse
   *optFirstUse,
   *optLastUse;
  int
    optTotExcls,
   *optExclList;
}
  t_optData;

/* Info on non-optional arguments (both actual and formal) */

typedef struct {
  COMMLINE_t_optArgType
    argType;
  char
   *argStr;
}
  t_nonOptArgsData;

/* Array of formal options */

static t_optData
 *optData = NULL;

/* Array of actual options */

static t_optUse
  **optUses = NULL;

/* Array of non-optional arguments */

static t_nonOptArgsData
 *nonOptArgsData = NULL;

/* Other self-explanatory variables */

static int
  formal_options = 0,
  actual_options = 0,
  non_opt_args   = 0;

static Bool
 *optExclData     = NULL,    /* Array of option exclusion info       */
  b_commLine_ok   = FALSE,   /* Problems parsing the command line?   */
  b_formalOpts_ok = FALSE,   /* Specifications of formal option ok?  */
  b_nonOptArgs_ok = FALSE;   /* Specifications non-optional args ok? */

static char
  commandLineError [ERROR_maxErrorMsgSize],
 *formatStr     = NULL,
 *program_name  = NULL,
 *str2val_error = NULL;

/*
*---------------------------------------------------------------------
*   Function prototypes
*---------------------------------------------------------------------
*/

static Bool *get_excl_data_address   (int  index1, int index2);
static void  make_options_exclusive  (int  index1, int index2);
static Bool  options_are_exclusive   (int  index1, int index2);
static int   optChar2optFormalIndex  (char option_char);
static int   optID2optFormalIndex    (int  option_ID);
static char *optActualIndex2optUsage (int  actual_index);
static char *optFormalIndex2optUsage (int  formal_index);
static char *timesStr                (int  times);
static char *value_matches_type      (char *string, COMMLINE_t_optArgType type);

/*
*---------------------------------------------------------------------
*   Take a "number of times" (int) and return
*   the corresponding adverb (string)
*---------------------------------------------------------------------
*/

static char *timesStr (int times)
{
 static char
   string [12];

 if (times < 0)
   return (NULL);
 switch (times) {
   case (0): strncpy (string, "0 times",  11); break;
   case (1): strncpy (string, "once",     11); break;
   case (2): strncpy (string, "twice",    11); break;
   default : snprintf (string, 11, "%d times", times);
 }
 return (string);
}

/*
*---------------------------------------------------------------------
*   Take an option's ID and return its formal index
*   (or -1 if the option ID is invalid)
*---------------------------------------------------------------------
*/

static int optID2optFormalIndex (int option_ID)
{
 int
   c_opt;
 t_optData
  *p_optData;

 for (c_opt = 0, p_optData = optData;
        c_opt < formal_options;
          c_opt++, p_optData++ )
   if (p_optData->optID == option_ID)
     return (c_opt);
 return (-1);
}

/*
*---------------------------------------------------------------------
*   Take an option's character and return its formal index
*   (or -1 if the option character is invalid)
*---------------------------------------------------------------------
*/

static int optChar2optFormalIndex (char option_char)
{
 int
   c_opt;
 t_optData
  *p_optData;

 for (c_opt = 0, p_optData = optData;
        c_opt < formal_options;
          c_opt++, p_optData++ )
   if (p_optData->optChar == option_char)
     return (c_opt);
 return (-1);
}

/*
*---------------------------------------------------------------------
*   Take an option's formal index and return a pointer to a static
*   string representing the option's formal usage, such as
*     "[-o]" or "[--output]" or "[-o|--output]"
*---------------------------------------------------------------------
*/

static char *optFormalIndex2optUsage (int formal_index)
{
 size_t
   optStrLength = 0,
   usageLength  = 0;
 char
   *auxStr = NULL;
 t_optData
  *p_optData = (t_optData *) optData + formal_index;

 /* Length of option's long name and max length of usage string */

 usageLength =  (size_t) (optStrLength = strlen(p_optData->optStr)) + 8;

 /* Allocate memory block for the strings */

 formatStr = (char *) realloc (formatStr, usageLength * sizeof (char));
 if (formatStr ==  NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::optFormalIndex2optUsage(%d): realloc error [formatStr]",
     program_name,
     formal_index );
   return (NULL);
 }
 auxStr = (char *) malloc (usageLength);
 if (auxStr ==  NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::optFormalIndex2optUsage(%d): malloc error [auxStr]",
     program_name,
     formal_index );
   return (NULL);
 }

 /* Prepare the usage string */

 strcpy (formatStr, "[");
 strcpy (auxStr, "");
 if (isalnum (p_optData->optChar)) {
   snprintf (auxStr, usageLength, "-%c", p_optData->optChar);
   if (optStrLength > 0)
     strcat (auxStr, "|");
 }
 strcat (formatStr, auxStr);
 strcpy (auxStr, "");
 if (optStrLength > 0)
   snprintf (auxStr, usageLength, "--%s", p_optData->optStr);
 strcat (formatStr, auxStr);
 strcat (formatStr, "]");

 /* Free "auxStr" and return the final string */

 free (auxStr);
 return (formatStr);
}

/*
*---------------------------------------------------------------------
*   Take an option's actual index and return a pointer to a static
*   string representing the option's actual usage, such as
*     "[-o]" or "[--output file1]"
*---------------------------------------------------------------------
*/

static char *optActualIndex2optUsage (int actual_index)
{
 size_t
   usageLength = 0;
 char
   *auxStr = NULL;
 t_optUse
   *p_optUse = optUses [actual_index];
 int
   formal_index = p_optUse->formalIndex;
 t_optData
   *p_optData = (t_optData *) optData + formal_index;

 /* Work out the length of usage string */

 usageLength = 1;                                      /* Eg. '['         */
 switch (p_optUse->size) {                             /*                 */
   case (t_opt_size_short) :                           /*                 */
     usageLength += 2;                                 /* Eg. '-o'        */
     break;                                            /*                 */
   case (t_opt_size_long) :                            /*                 */
     usageLength += 2 + strlen (p_optData->optStr);    /* Eg. '--output'  */
     break;                                            /*                 */
   default:                                            /*                 */
     snprintf (commandLineError,                       /*                 */
       ERROR_maxErrorMsgSize,                          /*                 */
       "%s::%s(): Invalid optUses[%d].size (%d)",      /*                 */
       program_name,                                   /*                 */
       __func__,                                   /*                 */
       actual_index,                                   /*                 */
       p_optUse->size );                               /*                 */
    return (NULL);                                     /*                 */
 }                                                     /*                 */
 if (p_optData->optArgType != COMMLINE_opt_arg_none)   /*                 */
   usageLength += 1 + strlen (p_optUse->argStr);       /* Eg. ' filename' */
 usageLength += 2;                                     /* Eg. ']\0'       */

 /* Allocate memory block for the strings */

 formatStr = (char *) realloc (formatStr, usageLength * sizeof (char));
 if (formatStr ==  NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::optActualIndex2optUsage(%d): Malloc error [formatStr]",
     program_name,
     actual_index );
   return (NULL);
 }
 auxStr = (char *) malloc (usageLength * sizeof (char));
 if (auxStr ==  NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::optActualIndex2optUsage(%d): Malloc error [auxStr]",
     program_name,
     actual_index );
   return (NULL);
 }

 /* Prepare the usage string */

 strcpy (formatStr, "[");
 strcpy (auxStr, "");
 if (p_optUse->size == t_opt_size_short)
   snprintf (auxStr, usageLength, "-%c", p_optData->optChar);
 else
   snprintf (auxStr, usageLength, "--%s", p_optData->optStr);
 strcat (formatStr, auxStr);
 if (p_optData->optArgType != COMMLINE_opt_arg_none) {
   snprintf (auxStr, usageLength, " %s", p_optUse->argStr);
   strcat (formatStr, auxStr);
 }
 strcat (formatStr, "]");

 /* Free "auxStr" and return the final string */

 free (auxStr);
 return (formatStr);
}

/*
*---------------------------------------------------------------------
*   Return the address of a given element in "optExclData"
*---------------------------------------------------------------------
*/

static Bool *get_excl_data_address (int index1, int index2)
{
 return ((Bool *) optExclData + (index1 * formal_options) + index2);
}

/*
*---------------------------------------------------------------------
*   Make two options mutually exclusive by setting
*   the appropriate Bools in "optExclData" to TRUE
*---------------------------------------------------------------------
*/

static void make_options_exclusive (int index1, int index2)
{
 Bool
  *p_Bool;

 *(p_Bool = get_excl_data_address (index1, index2)) = TRUE;
 *(p_Bool = get_excl_data_address (index2, index1)) = TRUE;
}

/*
*---------------------------------------------------------------------
*   Check whether two options are mutually exclusive by
*   looking at the appropriate Bools in "optExclData"
*---------------------------------------------------------------------
*/

static Bool options_are_exclusive (int index1, int index2)
{
 Bool
  *p_Bool1,
  *p_Bool2;

 p_Bool1 = get_excl_data_address (index1, index2);
 p_Bool2 = get_excl_data_address (index2, index1);
 return ((*p_Bool1) || (*p_Bool2));
}

/*
*---------------------------------------------------------------------
*   Free all memory dinamically allocated by the parsing procedure.
*   Return a boolean indicating whether it was successful.
*---------------------------------------------------------------------
*/

void COMMLINE_free_commLine_data (void)
{
 int
   c_opt;
 t_optData
   *p_optData;
 t_optUse
   *p_optUse;
 struct t_optUse
   *p_nextUse;

 if (optData) {
   for (c_opt = 0, p_optData = optData;
          c_opt < formal_options;
            c_opt++, p_optData++ ) {
     free (p_optData->optStr);
     free (p_optData->optExclList);
     p_optUse = p_optData->optFirstUse;
     while (p_optUse) {
       p_nextUse = p_optUse->next;
       free (p_optUse);
       p_optUse = (t_optUse *) p_nextUse;
     }
   }
   free (optData);
 }
 if (optUses)
   free (optUses);
 if (nonOptArgsData)
   free (nonOptArgsData);
 if (optExclData)
   free (optExclData);
 if (formatStr)
   free (formatStr);
 if (str2val_error)
   free (str2val_error);
}

/*
*---------------------------------------------------------------------
*   Obtain short program invocation name by stripping off
*   directory names from the full program name
*---------------------------------------------------------------------
*/

char *COMMLINE_get_program_short_name (char *full_name)
{
 char
   *short_name = strrchr (full_name, '/');

 if (short_name == NULL)
   short_name = full_name;
 else
   short_name++;
 return (short_name);
}

/*
*---------------------------------------------------------------------
*   Parse the command line, returning a boolean indicating whether
*   the procedure was successful.
*
*   This procedure takes some fixed and some variable parameters,
*   which are described below.
*   The ones marked [F] are fixed; all the others are variable.
*
*     int .............[F].......... argc, as received by main()
*     char ** .........[F].......... argv, as received by main()
*     int .............[F].......... formalOpts (# of formal options)
*
*       if formalOpts > 0, then for each option:
*
*       int ........................ option identifier
*       char ....................... option character
*       char * ..................... option string
*       COMMLINE_t_optArgType ...... option argument type
*       int  ....................... minimum # of uses in command line
*       int  ....................... maximum # of uses in command line
*       int  ....................... totExcls (size of option excl list)
*
*         if totExcls > 0, then for each option in the exclusion list:
*
*         int ...................... option identifier
*
*     int .............[F].......... nonOptArgs
*
*         if nonOptArgs > 0, then for each argument:
*
*         COMMLINE_t_optArgType .... option argument type
*
*---------------------------------------------------------------------
*/

Bool COMMLINE_parse_commLine (
 int   argc,
 char *argv[],
 int   formalOpts, ...)
{
 int
   c_arg,
   c_excl,
   c_opt,
   actual_opt_index,
   formal_opt_index,
   long_opt_index,
   nonOptArgs,
   optIndex,
  *p_exclID,
   remaining_args,
   short_option;
 char
  *actual_opt_arg,
  *opt_string,
  *shortOptsStr;
 size_t
  optExclListSize;
 t_option_size
   actual_opt_size;
 t_optData
  *p_optData;
 t_nonOptArgsData
  *p_argData;
 struct option
  *longOptsData;
 va_list
   argPtr;

 b_commLine_ok = b_formalOpts_ok = b_nonOptArgs_ok = FALSE;
 program_name = COMMLINE_get_program_short_name (argv[0]);

 /* Make sure the number of formal options is valid */

 if (formalOpts < 0) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): Invalid number of formal options (%d)",
     program_name,
     __func__,
     formalOpts );
   COMMLINE_free_commLine_data();
   return (FALSE);
 }

 /* Allocate and initialize memory for the formal option data array */

 optData = (t_optData *) malloc (formalOpts * sizeof (t_optData));
 if (optData == NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): Malloc error [optData]",
     __func__,
     program_name );
   COMMLINE_free_commLine_data();
   return (FALSE);
 }
 for (c_opt = 0, p_optData = optData;
        c_opt < formalOpts;
          c_opt++, p_optData++ ) {

   p_optData->optID       = (int)                   0;
   p_optData->optChar     = (char)                  '\0';
   p_optData->optStr      = (char *)                NULL;
   p_optData->optArgType  = (COMMLINE_t_optArgType) COMMLINE_opt_arg_none;
   p_optData->optMinUses  = (int)                   0;
   p_optData->optMaxUses  = (int)                   0;
   p_optData->optTotUses  = (int)                   0;
   p_optData->optFirstUse = (t_optUse *)            NULL;
   p_optData->optLastUse  = (t_optUse *)            NULL;
   p_optData->optTotExcls = (int)                   0;
   p_optData->optExclList = (int *)                 NULL;
 }

 /* Allocate and initialize memory for the option usage list */

 {
   int
     c_use,
     c_argvIndex,
     maxActualOpts;
   t_optUse
     **p_optUsePtr;

   /* The user may have provided several options witn a single '-' such as */
   /*   progname -abcde nonOptArg                                          */
   /* or even                                                              */
   /*   progname -ab -cde nonOptArg                                        */
   /* where a, b, c, d, e are five separate options. The safest thing to   */
   /* do is to assume that everything in argv[1] onwards are all options,  */
   /* otherwise we might allocate insufficient memory for them.            */

   for (maxActualOpts = 0, c_argvIndex = 1; c_argvIndex < argc; c_argvIndex++)
     maxActualOpts += strlen (argv[c_argvIndex]) - 1;
   optUses = (t_optUse **) malloc (maxActualOpts * sizeof (t_optUse *));
   if (optUses == NULL) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): Malloc error [optUses]",
       __func__,
       program_name );
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   for (c_use = 0, p_optUsePtr = optUses;
          c_use < maxActualOpts;
            c_use++, p_optUsePtr++ )
     *p_optUsePtr = (t_optUse *) NULL;
 }

 /* Process formal options */

 optExclListSize = formalOpts * sizeof (int);
 va_start (argPtr, formalOpts);

 for (c_opt = 0, p_optData = optData;
        c_opt < formalOpts;
          c_opt++, p_optData++ ) {

   p_optData->optID    = (int)  va_arg (argPtr, int);
   p_optData->optChar  = (char) va_arg (argPtr, int);
   if (! isalnum (p_optData->optChar))  {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: optChar  '\\x%x %c'",
       program_name,
       __func__,
       p_optData->optID,
       (unsigned int) p_optData->optChar,
       p_optData->optChar );
     strcat (commandLineError, "is not alphanumeric");
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   if ((p_optData->optChar == ':') || (p_optData->optChar == '?')) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: optChar '%c' is reserved",
       program_name,
       __func__,
       p_optData->optID,
       p_optData->optChar );
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   opt_string = (char *) va_arg (argPtr, char *);

   /* Should we allow empty strings for long option names? */

#ifndef ALLOW_EMPTY_OPT_LONG_NAMES

   if (strlen (opt_string) == 0) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: Invalid optStr (empty string)",
       program_name,
       __func__,
       p_optData->optID );
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }

#endif

   if ((p_optData->optChar == '\0') && (strlen (opt_string) == 0)) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: optChar & optStr are both unset",
       program_name,
       __func__,
       p_optData->optID );
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   errno = 0;
   p_optData->optStr = strdup (opt_string);
   if (p_optData->optStr == NULL) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: optData[%d].optStr = strdup (\"%s\") failed",
       program_name,
       __func__,
       p_optData->optID,
       c_opt,
       opt_string );
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   strcpy (p_optData->optStr, opt_string);
   p_optData->optArgType = (COMMLINE_t_optArgType) va_arg (argPtr, int);
   switch (p_optData->optArgType) {
     case (COMMLINE_opt_arg_none)      :
     case (COMMLINE_opt_arg_long_int)  :
     case (COMMLINE_opt_arg_ulong_int) :
     case (COMMLINE_opt_arg_double)    :
     case (COMMLINE_opt_arg_char)      :
     case (COMMLINE_opt_arg_string)    :
       break;
     default:
       snprintf (commandLineError,
	     ERROR_maxErrorMsgSize,
         "%s::%s(): optID %d: Invalid argument type (%d)",
         program_name,
         __func__,
         c_opt,
         p_optData->optArgType );
       COMMLINE_free_commLine_data();
       return (FALSE);
   }
   p_optData->optMinUses = (int) va_arg (argPtr, int);
   p_optData->optMaxUses = (int) va_arg (argPtr, int);
   if (p_optData->optMinUses < 0) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: Invalid optMinUses (%d)",
       program_name,
       __func__,
       p_optData->optID,
       p_optData->optMinUses );
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   if (p_optData->optMaxUses < 1) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: Invalid optMaxUses (%d)",
       program_name,
       __func__,
       p_optData->optID,
       p_optData->optMaxUses );
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   if (p_optData->optMinUses > p_optData->optMaxUses) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: optMinUses > optMaxUses",
       program_name,
       __func__,
       p_optData->optID );
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   p_optData->optTotExcls = (int) va_arg (argPtr, int);
   if (p_optData->optTotExcls == 0)
     continue;
   p_optData->optExclList = (int *) malloc (optExclListSize);
   if (p_optData->optExclList == NULL) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: Malloc error [optExclList]",
       program_name,
       __func__,
       p_optData->optID );
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   memset (p_optData->optExclList, 0, optExclListSize);
   for (c_excl = 0, p_exclID = p_optData->optExclList;
	  c_excl < p_optData->optTotExcls;
	    c_excl++, p_exclID++ )
     *p_exclID = va_arg (argPtr, int);

 }

 formal_options = formalOpts;
 b_formalOpts_ok = TRUE;

 /* All formal options have been successfully processed.   */
 /* Make sure the number of non-option arguments is valid. */

 nonOptArgs = (int) va_arg (argPtr, int);
 if (nonOptArgs < 0) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): Invalid number of non-option arguments (%d)",
     program_name,
     __func__,
     nonOptArgs );
   va_end (argPtr);
   COMMLINE_free_commLine_data();
   return (FALSE);
 }

 if ((non_opt_args = nonOptArgs) > 0) {

   /* Allocate and initialize memory for the non-option args array */

   nonOptArgsData =
     (t_nonOptArgsData *) malloc (nonOptArgs * sizeof (t_nonOptArgsData));
   if (nonOptArgsData == NULL) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): Malloc error [nonOptArgsData]",
       program_name,
       __func__ );
     va_end (argPtr);
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   for (c_arg = 0, p_argData = nonOptArgsData;
          c_arg < nonOptArgs;
            c_arg++, p_argData++ ) {

     p_argData->argStr  = (char *) NULL;
     p_argData->argType = (COMMLINE_t_optArgType) va_arg (argPtr, int);
     switch (p_argData->argType) {
       case (COMMLINE_opt_arg_long_int)  :
       case (COMMLINE_opt_arg_ulong_int) :
       case (COMMLINE_opt_arg_double)    :
       case (COMMLINE_opt_arg_char)      :
       case (COMMLINE_opt_arg_string)    :
       case (COMMLINE_opt_arg_none)      :
         break;
       default:
         snprintf (commandLineError,
		     ERROR_maxErrorMsgSize,
           "%s::%s(): Non-option arg %d: Invalid type (%d)",
           program_name,
           __func__,
           c_arg,
           p_argData->argType );
         p_argData->argType = (COMMLINE_t_optArgType) COMMLINE_opt_arg_none;
         COMMLINE_free_commLine_data();
         return (FALSE);
     }
   }
 }

 /* We are done with this procedure's parameter list */

 va_end (argPtr);
 b_nonOptArgs_ok = TRUE;

 /* Checking for duplicate information */

 for (c_opt = 0, p_optData = optData;
        c_opt < formalOpts;
          c_opt++, p_optData++ ) {

   int
     c_opt2;
   t_optData
     *p_optData2;

   for (c_opt2 = c_opt + 1, p_optData2 = p_optData + 1;
          c_opt2 < formalOpts;
            c_opt2++, p_optData2++ ) {

     /* Make sure no two options have the same "optID" */

     if (p_optData->optID == p_optData2->optID) {
       snprintf (commandLineError,
	     ERROR_maxErrorMsgSize,
         "%s::%s(): optID %d not unique",
         program_name,
         __func__,
         p_optData ->optID );
       va_end (argPtr);
       COMMLINE_free_commLine_data();
       return (FALSE);
     }

     /* Make sure no two options have the same "optChar" */

     if (p_optData->optChar == p_optData2->optChar) {
       snprintf (commandLineError,
	     ERROR_maxErrorMsgSize,
         "%s::%s(): optIDs %d & %d: Duplicate optChar '%c'",
         program_name,
         __func__,
         p_optData ->optID,
         p_optData2->optID,
         p_optData ->optChar );
       va_end (argPtr);
       COMMLINE_free_commLine_data();
       return (FALSE);
     }

     /* Make sure no two options have the same "optStr" */

     if (! strcmp (p_optData->optStr, p_optData2->optStr)) {
       snprintf (commandLineError,
	     ERROR_maxErrorMsgSize,
         "%s::%s(): optIDs %d & %d: Duplicate optStr \"%s\"",
         program_name,
         __func__,
         p_optData ->optID,
         p_optData2->optID,
         p_optData ->optStr );
       va_end (argPtr);
       COMMLINE_free_commLine_data();
       return (FALSE);
     }
   }
 }

 /* Allocate and initialize memory for the option exclusion data array */

 {
   size_t
     optExclDataSize = formalOpts * formalOpts * sizeof (Bool);

   optExclData = (Bool *) malloc (optExclDataSize);
   if (optExclData == NULL) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): Malloc error [optExclData]",
       program_name,
       __func__ );
     COMMLINE_free_commLine_data();
     return (FALSE);
   }
   memset (optExclData, FALSE, optExclDataSize);
 }

 /* Use the exclusion info to set up the option exclusion data array */

 for (c_opt = 0, p_optData = optData;
        c_opt < formalOpts;
          c_opt++, p_optData++ ) {

   /* We now have access to the exclusion list of "c_opt".  */
   /* It may contain 0 or more elements, e.g. [2 4 7].      */
   /* Assuming c_opt == 1, we need to do a few things:      */
   /* 1. make sure the option IDs in this list are valid,   */
   /*    i.e. they correspond to existing options           */
   /* 2. mark "c_opt" and the options in its exclusion list */
   /*    as mutually exclusive, i.e. (1,2), (1,4) and (1,7) */
   /* 3. mark each pair of options in this list as mutually */
   /*    exclusive, i.e. (2,4), (2,7), and (4,7)            */

   for (c_excl = 0, p_exclID = p_optData->optExclList;
	  c_excl < p_optData->optTotExcls;
	    c_excl++, p_exclID++ ) {

     /* Make sure the option IDs in this exclusion list are valid. */
     /* This corresponds to step 1 as described above.             */

     optIndex = optID2optFormalIndex (*p_exclID);
     if (optIndex == -1) {
       snprintf (commandLineError,
	     ERROR_maxErrorMsgSize,
         "%s::%s(): Exclusion list of %s: Invalid optID %d",
         program_name,
         __func__,
         optFormalIndex2optUsage (c_opt),
         *p_exclID );
       COMMLINE_free_commLine_data();
       return (FALSE);
     }
   }

   for (c_excl = 0, p_exclID = p_optData->optExclList;
	  c_excl < p_optData->optTotExcls;
	    c_excl++, p_exclID++ ) {

     optIndex = optID2optFormalIndex (*p_exclID);

     /* At this point we know that "c_opt" and "optIndex" are      */
     /* valid, mutually exclusive options so we mark them as such. */
     /* This corresponds to step 2 as described above.             */

     make_options_exclusive (c_opt, optIndex);

     /* Now we need another loop to pick out other elements of  */
     /* the exclusion list and mark them as mutually exclusive. */
     /* This corresponds to step 3 as described above.          */

     {
       int
         c_excl2,
	 optIndex2,
	 *p_exclID2;

       for (c_excl2 = c_excl + 1, p_exclID2 = p_exclID + 1;
       	    c_excl2 < p_optData->optTotExcls;
       	      c_excl2++, p_exclID2++ ) {
         optIndex2 = optID2optFormalIndex (*p_exclID2);
         make_options_exclusive (c_opt, optIndex2);
       }
     }
   }
 }

 /* Make sure no option is mutually exclusive with itself! */

 for (c_opt = 0, p_optData = optData;
        c_opt < formalOpts;
          c_opt++, p_optData++ )
   if (options_are_exclusive (c_opt, c_opt)) {
     snprintf (commandLineError,
	   ERROR_maxErrorMsgSize,
       "%s::%s(): optID %d: Mutually exclusive with itself",
       program_name,
       __func__,
       p_optData->optID );
     COMMLINE_free_commLine_data();
     return (FALSE);
   }

 /* The description of formal options, contained in the variable */
 /* parameter list to this procedure, seems to be OK. We may now */
 /* proceed to extract data from the command line.               */

 /* No more calls to COMMLINE_free_commLine_data() in this procedure!     */
 /* This way the calling program can still have access to usage  */
 /* info, which can be quite useful -- especially when the user  */
 /* doesn't know about non-optional arguments!                   */

 /* Preparing the arguments to "getopt_long()"...                */
 /* - first we allocate and set the string of short option chars */

 shortOptsStr = (char *) malloc ((2*formalOpts+2) * sizeof (char));
 if (shortOptsStr == NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): Malloc error [shortOptsStr]",
     program_name,
     __func__ );
   return (FALSE);
 }
 {
   int
     auxIndex;

   shortOptsStr [0] = ':';
   for (c_opt = 0, auxIndex = 1, p_optData = optData;
          c_opt < formalOpts;
            c_opt++, p_optData++ )
     if (p_optData->optChar != '\0') {
       shortOptsStr [auxIndex++] = p_optData->optChar;
       if (p_optData->optArgType != COMMLINE_opt_arg_none)
         shortOptsStr [auxIndex++] = ':';
     }
   shortOptsStr [auxIndex] = '\0';
 }

 /* - then we allocate and set the array of long option parameters */

 longOptsData =
   (struct option *) malloc ((formalOpts+1) * sizeof (struct option));
 if (longOptsData == NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): Malloc error [longOptsData]",
     program_name,
     __func__ );
   return (FALSE);
 }
 {
   struct option
     *p_longOption;

   for (c_opt = 0, p_optData = optData, p_longOption = longOptsData;
          c_opt < formalOpts;
            c_opt++, p_optData++, p_longOption++ ) {
     p_longOption->name    = p_optData->optStr;
     p_longOption->has_arg = (p_optData->optArgType == COMMLINE_opt_arg_none) ?
       no_argument :
       required_argument ;
     p_longOption->flag = (int *) NULL;
     p_longOption->val  = p_optData->optID;
   }
   p_longOption->name    = 0;
   p_longOption->has_arg = 0;
   p_longOption->flag    = 0;
   p_longOption->val     = 0;
 }

 /* Note that, from now on, it makes no sense to mention */
 /* "optID" in error messages, since they will have been */
 /* caused by incorrect usage AT RUN-TIME !!!            */

 /* Parse the command line...               */
 /* First, disable automatic error messages */

 opterr = 0;
 while (TRUE) {

   short_option = getopt_long (
     (int)             argc,
     (char **)         argv,
     (const char *)    shortOptsStr,
     (struct option *) longOptsData,
     (int *)          &long_opt_index );

   /* Detect the end of the options */

   if (short_option == -1)
     break;

   switch (short_option) {

     /* A valid long option with optChar == '\0' */

     case 0 :
       actual_opt_size = t_opt_size_long;
       actual_opt_arg  = optarg;
       break;

     /* A valid option but a missing argument */

     case ':' :
       snprintf (commandLineError,
         ERROR_maxErrorMsgSize,
         "%s: Option [%s] requires an argument",
         program_name,
         argv [optind-1] );
       return (FALSE);

     /* An invalid short option */

     case '?' :
       snprintf (commandLineError,
         ERROR_maxErrorMsgSize,
         "%s: Unknown option '%s'",
         program_name,
         argv [optind-1] );
       return (FALSE);

     default:

       /* It could be a valid short option... */

       if (strchr (shortOptsStr, short_option) != NULL) {
         actual_opt_size  = t_opt_size_short;
         actual_opt_arg   = optarg;
       }
       else {

         /* When "getopt_long()" finds a long option and that   */
         /* option's "flag" == NULL, it returns the contents of */
         /* "val" -- in this case, the option's ID. So maybe    */
         /* this is what's happened...                          */

         actual_opt_size = t_opt_size_long;
         actual_opt_arg  = optarg;
       }
   }

   /* At this point we know we have found a valid   */
   /* option, but it may have been used incorrectly */

   /* Get the index of the formal option just found  */

   if (actual_opt_size == t_opt_size_long)
     formal_opt_index = long_opt_index;
   else {
     formal_opt_index = optChar2optFormalIndex ((char) short_option);
     if (formal_opt_index == -1) {
       snprintf (commandLineError,
         ERROR_maxErrorMsgSize,
         "%s: Hum... Couldn't find optFormalIndex [-%c]",
         program_name,
         short_option );
       return (FALSE);
     }
   }

   p_optData = optData + formal_opt_index;

   /* Create a new entry for this actual option */

   {
     t_optUse
       *p_newOptUse = (t_optUse *) malloc (sizeof (t_optUse));

     if (p_newOptUse == NULL) {
       snprintf (commandLineError,
         ERROR_maxErrorMsgSize,
         "%s: Malloc error; option new use %s",
         program_name,
         optFormalIndex2optUsage (formal_opt_index) );
       return (FALSE);
     }
     p_newOptUse->formalIndex = formal_opt_index;
     p_newOptUse->actualIndex = actual_options;
     p_newOptUse->size        = actual_opt_size;
     p_newOptUse->argStr      = actual_opt_arg;
     p_newOptUse->next        = NULL;

     /* Append the new entry to the formal option's usage list */

     if (p_optData->optFirstUse == NULL)
       p_optData->optFirstUse = p_newOptUse;
     else
       p_optData->optLastUse->next = (struct t_optUse *) p_newOptUse;
     p_optData->optLastUse = p_newOptUse;

     /* Update global option usage list */

     optUses [actual_options] = p_newOptUse;
   }

   /* Update option usage counters */

   actual_opt_index = actual_options++;
   p_optData->optTotUses++;

   /* Make sure the option was used correctly... */

   /* An option with a missing argument? */

   if ((p_optData->optArgType != COMMLINE_opt_arg_none) && (actual_opt_arg == NULL)) {
     snprintf (commandLineError,
       ERROR_maxErrorMsgSize,
       "%s: Option %s requires an argument",
       program_name,
       optActualIndex2optUsage (actual_opt_index) );
     return (FALSE);
   }

   /* An option with an unwanted argument? */

   if ((p_optData->optArgType == COMMLINE_opt_arg_none) && (actual_opt_arg != NULL)) {
     snprintf (commandLineError,
       ERROR_maxErrorMsgSize,
       "%s: Option %s does not take an argument",
       program_name,
       optActualIndex2optUsage (actual_opt_index) );
     return (FALSE);
   }

   /* An option with an expected argument of the right type? */

   {
     char
       *errorStr = value_matches_type (actual_opt_arg, p_optData->optArgType);

     if (errorStr != NULL) {
       snprintf (commandLineError,
         ERROR_maxErrorMsgSize,
         "%s: Option %s: %s",
         program_name,
         optActualIndex2optUsage (actual_opt_index),
         errorStr );
       return (FALSE);
     }
   }

 }

 free (shortOptsStr);
 free (longOptsData);

 /* No more options in the command line. Check a few */
 /* things before looking for non-option arguments.  */

 /* Make sure that all options have been used a valid number */
 /* of times, i.e.  optMinUses <= optTotUses <= optMaxUses   */

 for (c_opt = 0, p_optData = optData;
        c_opt < formalOpts;
          c_opt++, p_optData++ ) {
   if (p_optData->optTotUses < p_optData->optMinUses) {
     snprintf (commandLineError,
       ERROR_maxErrorMsgSize,
       "%s: Option %s used less than %s",
       program_name,
       optFormalIndex2optUsage (c_opt),
       timesStr (p_optData->optMinUses) );
     return (FALSE);
   }
   if (p_optData->optTotUses > p_optData->optMaxUses) {
     snprintf (commandLineError,
       ERROR_maxErrorMsgSize,
       "%s: Option %s used more than %s",
       program_name,
       optFormalIndex2optUsage (c_opt),
       timesStr (p_optData->optMaxUses) );
     return (FALSE);
   }
 }

 /* Make sure that mutually exclusive options have not been used */

 for (c_opt = 0, p_optData = optData;
        c_opt < formalOpts;
          c_opt++, p_optData++ ) {

   if (p_optData->optTotUses == 0)
     continue;

   /* Option "c_opt" has been used; check its exclusion list */

   for (c_excl = 0, p_exclID = p_optData->optExclList;
	  c_excl < p_optData->optTotExcls;
	    c_excl++, p_exclID++ ) {
     int
       formalIndex2 = optID2optFormalIndex (*p_exclID);
     t_optData
       *p_optData2 = optData + formalIndex2;

     if (p_optData2->optTotUses == 0)
       continue;

     {
       int
	     actualIndex;
       char
	     *auxStr1,
	     *auxStr2;

       if (p_optData->optTotUses == 1) {
	     actualIndex = p_optData->optFirstUse->actualIndex;
	     auxStr1 = strdup (optActualIndex2optUsage (actualIndex));
	     if (auxStr1 == NULL) {
	       snprintf (commandLineError,
             ERROR_maxErrorMsgSize,
             "%s::%s(): optID %d: auxStr1 = strdup (\"%s\") failed",
             program_name,
             __func__,
             p_optData->optID,
	         optActualIndex2optUsage (actualIndex) );
	       return (FALSE);
	     }
       }
       else {
	     auxStr1 = strdup (optFormalIndex2optUsage (c_opt));
	     if (auxStr1 == NULL) {
	       snprintf (commandLineError,
             ERROR_maxErrorMsgSize,
             "%s::%s(): optID %d: auxStr1 = strdup (\"%s\") failed",
             program_name,
             __func__,
             p_optData->optID,
	         optFormalIndex2optUsage (c_opt) );
	       return (FALSE);
	     }
       }

       if (p_optData2->optTotUses == 1) {
	     actualIndex = p_optData2->optFirstUse->actualIndex;
	     auxStr2 = strdup (optActualIndex2optUsage (actualIndex));
	     if (auxStr2 == NULL) {
	       snprintf (commandLineError,
             ERROR_maxErrorMsgSize,
             "%s::%s(): optID %d: auxStr2 = strdup (\"%s\") failed",
             program_name,
             __func__,
             p_optData2->optID,
	         optActualIndex2optUsage (actualIndex) );
	       return (FALSE);
	     }
       }
       else {
	     auxStr2 = strdup (optFormalIndex2optUsage (formalIndex2));
	     if (auxStr2 == NULL) {
	       snprintf (commandLineError,
             ERROR_maxErrorMsgSize,
             "%s::%s(): optID %d: auxStr2 = strdup (\"%s\") failed",
             program_name,
             __func__,
             p_optData2->optID,
	         optFormalIndex2optUsage (formalIndex2) );
	       return (FALSE);
	     }
       }

       snprintf (commandLineError,
         ERROR_maxErrorMsgSize,
         "%s: Use either %s or %s",
         program_name,
         auxStr1,
         auxStr2 );
       free (auxStr1);
       free (auxStr2);
       return (FALSE);
     }
   }
 }

 /* All options are OK. Now it's time to look    */
 /* for non-option arguments in the command line */

 /* Is there the right number of non-option args in the command line? */

 if ((remaining_args = argc - optind) != non_opt_args) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s: Found %d non-option arguments; expected %d",
     program_name,
     remaining_args,
     non_opt_args );
   return (FALSE);
 }

 if (non_opt_args > 0) {

   /* Make sure the arguments match their types */

   char
     *errorStr;

   for (c_arg = 0, p_argData = nonOptArgsData;
          c_arg < nonOptArgs;
            c_arg++, p_argData++ ) {
     p_argData->argStr = argv [optind + c_arg];
     errorStr = value_matches_type (p_argData->argStr, p_argData->argType);
     if (errorStr != NULL) {
       snprintf (commandLineError,
         ERROR_maxErrorMsgSize,
         "%s: Non-option argument [%s]: %s",
         program_name,
         p_argData->argStr,
         errorStr );
       return (FALSE);
     }
   }
 }

 /* Everything seems to be OK... */

 strcpy (commandLineError, "");
 return (b_commLine_ok = TRUE);
}

/*
*---------------------------------------------------------------------
*   This procedure is a bit tricky to explain with words...
*   Best to give an example:
*
*     Suppose the main() procedure of the calling program
*     defines the following formal options:
*
*         optID   optChar   optArgType
*         -----   -------   ----------
*          100       a       long int
*          200       b       long int
*          300       c       long int
*
*     Now suppose the main program, called "myProg", is
*     given the following command line:
*
*         prompt% myProg -a 777 -b 444 -a 999
*
*     After parsing the command line arguments, the following call
*
*         Bool found = COMMLINE_optPos2optUse (3, &optID, &optUse);
*
*     would set the following variables:
*
*         optID  = 100  (identifier of option 'a')
*         optUse = 2    (second use of 'a' in the command line)
*         found  = TRUE (success)
*
*---------------------------------------------------------------------
*/

Bool COMMLINE_optPos2optUse (int opt_pos, int *opt_ID, int *opt_use)
{
 int
   c_use;
 t_optData
   *p_optData;
 t_optUse
   *p_optUse,
   *p_optUse2;

 if (! b_commLine_ok) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_optPos2optUse(): Command line parse error",
     program_name );
   *opt_use = 0;
   return (FALSE);
 }
 if (opt_pos > actual_options) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_optPos2optUse(): Only %d option(s) in command line",
     program_name,
     actual_options );
   COMMLINE_free_commLine_data();
   *opt_use = 0;
   return (FALSE);
 }
 p_optUse  = optUses [opt_pos - 1];
 p_optData = optData + p_optUse->formalIndex;
 *opt_ID = p_optData->optID;
 for (c_use = 0, p_optUse2 = p_optData->optFirstUse;
        c_use < p_optData->optTotUses;
          c_use++, p_optUse2 = (t_optUse *) p_optUse2->next )
   if (p_optUse == p_optUse2)
     break;
 *opt_use = c_use;
 return (TRUE);
}

/*
*---------------------------------------------------------------------
*   This procedure is a bit tricky to explain with words...
*   Best to give an example:
*
*     Suppose the main() procedure of the calling program
*     defines the following formal options:
*
*         optID   optChar   optArgType
*         -----   -------   ----------
*          100       a       long int
*          200       b       long int
*          300       c       long int
*
*     Now suppose the main program, called "myProg", is
*     given the following command line:
*
*         prompt% myProg -a 777 -b 444 -a 999
*
*     After parsing the command line arguments, the following call
*
*         Bool found = COMMLINE_optUse2optPos (100, 2, &optPos);
*
*     would set the following variables:
*
*         optPos = 3    (third option in the command line)
*         found  = TRUE (success)
*
*---------------------------------------------------------------------
*/

Bool COMMLINE_optUse2optPos (int opt_ID, int opt_use, int *opt_pos)
{
 int
   c_use,
   formalIndex;
 t_optData
   *p_optData;
 t_optUse
   *p_optUse;

 if (! b_commLine_ok) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_optUse2optPos(): Command line parse error",
     program_name );
   *opt_pos = 0;
   return (FALSE);
 }
 formalIndex = optID2optFormalIndex (opt_ID);
 if (formalIndex == -1) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_optUse2optPos(): Invalid optID %d",
     program_name,
     opt_ID );
   *opt_pos = 0;
   return (FALSE);
 }
 p_optData = optData + formalIndex;
 if (opt_use > p_optData->optTotUses) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_optUse2optPos(): optID %d used only %s",
     program_name,
     opt_ID,
     timesStr (p_optData->optTotUses) );
   *opt_pos = 0;
   return (FALSE);
 }
 for (c_use = 1, p_optUse = p_optData->optFirstUse;
        c_use < opt_use;
          c_use++ )
   p_optUse = (t_optUse *) p_optUse->next;
 *opt_pos = p_optUse->actualIndex + 1;
 return (TRUE);
}

/*
*---------------------------------------------------------------------
*   Retrieve the value of a particular option argument.
*   This procedure is a bit tricky to explain with words...
*   Best to give an example:
*
*     Suppose the main() procedure of the calling program
*     defines the following formal options:
*
*         optID   optChar   optArgType
*         -----   -------   ----------
*          100       a       long int
*          200       b       long int
*          300       c       long int
*
*     Now suppose the main program, called "myProg", is
*     given the following command line:
*
*         prompt% myProg -a 777 -b 444 -a 999
*
*     After parsing the command line arguments, the following call
*
*         long longInt;
*         Bool found = COMMLINE_optUse2optArg (100, 2, &longInt);
*
*     would set the following variables:
*
*         longInt = 999  (argument to '-a' the 2nd time it was used)
*         found   = TRUE (success)
*
*---------------------------------------------------------------------
*/

Bool COMMLINE_optUse2optArg (int opt_ID, int opt_use, void *opt_arg)
{
 int
   c_use,
   formalIndex;
 char
   *optArg,
   *tail;
 Bool
   b_error;
 long int
   auxLong;
 unsigned long int
   auxULong;
 double
   auxDouble;
 t_optData
   *p_optData;
 t_optUse
   *p_optUse;

 if (! b_commLine_ok) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): Command line parse error",
     program_name,
     __func__ );
   return (FALSE);
 }
 formalIndex = optID2optFormalIndex (opt_ID);
 if (formalIndex == -1) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): Invalid optID %d",
     program_name,
     __func__,
     opt_ID );
   return (FALSE);
 }
 p_optData = optData + formalIndex;
 if (p_optData->optArgType == COMMLINE_opt_arg_none) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): optID %d takes no arguments",
     program_name,
     __func__,
     opt_ID );
   return (FALSE);
 }
 if (opt_use > p_optData->optTotUses) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): optID %d used only %s",
     program_name,
     __func__,
     opt_ID,
     timesStr (p_optData->optTotUses) );
   return (FALSE);
 }
 for (c_use = 1, p_optUse = p_optData->optFirstUse;
        c_use < opt_use;
          c_use++ )
   p_optUse = (t_optUse *) p_optUse->next;
 optArg = p_optUse->argStr;
 switch (p_optData->optArgType) {
   case (COMMLINE_opt_arg_long_int) :
     errno = 0;
     auxLong = strtol (optArg, &tail, 0);
     b_error =
       (errno != 0) || ((auxLong==0) && (tail==optArg)) || (strlen(tail)>0);
     * (long int *) opt_arg = (b_error? (long int) 0: auxLong);
     break;
   case (COMMLINE_opt_arg_ulong_int) :
     errno = 0;
     auxULong = strtoul (optArg, &tail, 0);
     b_error =
       (errno != 0) || ((auxULong==0) && (tail==optArg)) || (strlen(tail)>0);
     * (unsigned long int *) opt_arg =
       (b_error? (unsigned long int) 0: auxULong);
     break;
   case (COMMLINE_opt_arg_double) :
     errno = 0;
     auxDouble = strtod (optArg, &tail);

     /* GCC complains about the following test:                                             */
     /*   b_error = (errno != 0) || ((auxDouble==0) && (tail==optArg)) || (strlen(tail)>0); */
     /*                                        ^                                            */
     /*   comparing floating point with == or != is unsafe [-Wfloat-equal]                  */
     /* so we changed it                                                                    */

     b_error = (errno != 0) || (tail==optArg) || (strlen(tail)>0);
     * (double *) opt_arg = (b_error? (double) 0: auxDouble);
     break;
   case (COMMLINE_opt_arg_char) :
     b_error =
       (strlen (optArg) != 1) || (! isalnum (*optArg));
     * (char *) opt_arg = (b_error? (char) 0: *optArg);
     break;
   case (COMMLINE_opt_arg_string) :
     b_error = (strlen (optArg) < 1);
     * (char **) opt_arg = b_error ? (char *) NULL : strdup (optArg);
     break;
   case (COMMLINE_opt_arg_none) :
     b_error = TRUE;
     break;
   default:
     b_error = TRUE;
     snprintf (commandLineError,
       ERROR_maxErrorMsgSize,
       "%s::%s(): p_optData->optArgType %d unknown",
       program_name,
       __func__,
       p_optData->optArgType );
 }
 if (b_error)
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): optID %d [%d]: Could not retrieve argument",
     __func__,
     program_name,
     opt_ID,
     opt_use );
 else
   strcpy (commandLineError, "");
 return (! b_error);
}

/*
*---------------------------------------------------------------------
*   Retrieve the value of a particular non-option argument.
*   This procedure is a bit tricky to explain with words...
*   Best to give an example:
*
*     Suppose the main() procedure of the calling program
*     defines only the following non-option arguments:
*
*         nonOptArgType
*         -------------
*            long int
*            long int
*
*     Now suppose the main program, called "myProg", is
*     given the following command line:
*
*         prompt% myProg 777 444
*
*     After parsing the command line arguments, the following call
*
*         long longInt;
*         Bool found = COMMLINE_argPos2argVal (2, &longInt);
*
*     would set the following variables:
*
*         longInt = 444  (value of 2nd non-option argument)
*         found   = TRUE (success)
*
*---------------------------------------------------------------------
*/

Bool COMMLINE_argPos2argVal (int arg_pos, void *arg_val)
{
 char
   *argStr,
   *tail;
 Bool
   b_error;
 long int
   auxLong;
 unsigned long int
   auxULong;
 double
   auxDouble;
 t_nonOptArgsData
  *p_argData;

 if (! b_commLine_ok) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_argPos2argVal(): Command line parse error",
     program_name );
   return (FALSE);
 }
 if (arg_pos > non_opt_args) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_argPos2argVal(): There are only %d non-option args",
     program_name,
     non_opt_args );
   return (FALSE);
 }
 p_argData = nonOptArgsData + arg_pos - 1;
 argStr = p_argData->argStr;
 switch (p_argData->argType) {
   case (COMMLINE_opt_arg_long_int) :
     errno = 0;
     auxLong = strtol (argStr, &tail, 0);
     b_error =
       (errno != 0) || ((auxLong==0) && (tail==argStr)) || (strlen(tail)>0);
     * (long int *) arg_val = (b_error? (long int) 0: auxLong);
     break;
   case (COMMLINE_opt_arg_ulong_int) :
     errno = 0;
     auxULong = strtoul (argStr, &tail, 0);
     b_error =
       (errno != 0) || ((auxULong==0) && (tail==argStr)) || (strlen(tail)>0);
     * (unsigned long int *) arg_val =
       (b_error? (unsigned long int) 0: auxULong);
     break;
   case (COMMLINE_opt_arg_double) :
     errno = 0;
     auxDouble = strtod (argStr, &tail);

     /* GCC complains about the following test:                                             */
     /*   b_error = (errno != 0) || ((auxDouble==0) && (tail==argStr)) || (strlen(tail)>0); */
     /*                                        ^                                            */
     /*   comparing floating point with == or != is unsafe [-Wfloat-equal]                  */
     /* so we changed it                                                                    */

     b_error = (errno != 0) || (tail==argStr) || (strlen(tail)>0);
     * (double *) arg_val = (b_error? (double) 0: auxDouble);
     break;
   case (COMMLINE_opt_arg_char) :
     b_error =
       (strlen (argStr) != 1) || (! isalnum (*argStr));
     * (char *) arg_val = (b_error? (char) 0: *argStr);
     break;
   case (COMMLINE_opt_arg_string) :
     b_error = (strlen (argStr) < 1);
     * (char **) arg_val = b_error ? (char *) NULL : strdup (argStr);
     break;
   case (COMMLINE_opt_arg_none) :
     b_error = TRUE;
     break;
   default:
     b_error = TRUE;
     snprintf (commandLineError,
       ERROR_maxErrorMsgSize,
       "%s::%s(): p_argData->argType %d unknown",
       program_name,
       __func__,
       p_argData->argType );
 }
 if (b_error)
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): Non-option arg %d: Could not retrieve value",
     program_name,
     __func__,
     arg_pos );
 else
   strcpy (commandLineError, "");
 return (! b_error);
}

/*
*---------------------------------------------------------------------
*   Store in "uses" the number of times that option was used.
*   Return a bool indicating whether the procedure was succesful.
*---------------------------------------------------------------------
*/

Bool COMMLINE_optId2optUses (int opt_ID, int *uses)
{
 int
   formalIndex;

 if (! b_commLine_ok) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_optId2optUses(): Command line parse error",
     program_name );
   *uses = 0;
   return (FALSE);
 }
 if ((formalIndex = optID2optFormalIndex (opt_ID)) == -1) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_optId2optUses(): Invalid optID %d",
     program_name,
     opt_ID );
   *uses = 0;
   return (FALSE);
 }
 strcpy (commandLineError, "");
 *uses = optData[formalIndex].optTotUses;
 return (TRUE);
}

/*
*---------------------------------------------------------------------
*   Return the total number of options in the command line
*---------------------------------------------------------------------
*/

int COMMLINE_total_options (void)
{
 if (! b_commLine_ok) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_total_options(): Command line parse error",
     program_name );
   return (0);
 }
 else
   return (actual_options);
}

/*
*---------------------------------------------------------------------
*   Return the total number of non-option args in the command line
*---------------------------------------------------------------------
*/

int COMMLINE_total_non_opt_args (void)
{
 if (! b_commLine_ok) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_total_non_opt_args(): Command line parse error",
     program_name );
   return (0);
 }
 else
   return (non_opt_args);
}

/*
*---------------------------------------------------------------------
*   Return a pointer to the error message.
*---------------------------------------------------------------------
*/

char *COMMLINE_get_commLine_error (void)
{
 return (commandLineError);
}

/*
*---------------------------------------------------------------------
*   If "string" can be converted to "type" then return a NULL pointer.
*   Otherwise return a pointer to a static string containing the
*   corresponding error message.
*---------------------------------------------------------------------
*/

static char *value_matches_type (char *string, COMMLINE_t_optArgType type)
{
 size_t
   errorLength,
   stringLength;
 char
   *tail;
 long int
   auxLong;
 unsigned long int
   auxULong;
 double
   auxDouble;

 /* If "str2val_error" is already in use, it must be */
 /* freed before a new memory block can be allocated */

 if (str2val_error != NULL)
   free (str2val_error);

 /* Allocate memory block for "str2val_error" */

 stringLength = ((string == NULL) ? 0 : strlen (string));
 errorLength = 50 + stringLength;
 str2val_error = (char *) malloc (errorLength * sizeof (char));
 if (str2val_error ==  NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::value_matches_type(\"%s\"): Malloc error [str2val_error]",
     program_name,
     string );
   return (NULL);
 }

 /* Unexpected argument? */

 if ((type == COMMLINE_opt_arg_none) && (string != NULL)) {
   snprintf (str2val_error, errorLength, "Unexpected argument: \"%s\"", string);
   return (str2val_error);
 }

 /* Missing argument? */

 if ((type != COMMLINE_opt_arg_none) && (stringLength == 0)) {
   strncpy (str2val_error, "String expected", errorLength);
   return (str2val_error);
 }

 /* Try to convert "string" to the required data type */

 strcpy (str2val_error, "");
 switch (type) {
   case (COMMLINE_opt_arg_long_int) :
     errno = 0;
     auxLong = strtol (string, &tail, 0);
     if (errno)
       strncpy (str2val_error, "Long int overflow", errorLength);
     else if ((auxLong == 0) && (tail == string))
       snprintf (str2val_error, errorLength, "Bad integer syntax: '%s'", string);
     else if (strlen (tail) > 0)
       snprintf (str2val_error, errorLength, "Bad integer syntax: '%s'", tail);
     break;
   case (COMMLINE_opt_arg_ulong_int) :
     errno = 0;
     auxULong = strtoul (string, &tail, 0);
     if (errno)
       strncpy (str2val_error, "Unsigned long int overflow", errorLength);
     else if ((auxULong == 0) && (tail == string))
       snprintf (str2val_error, errorLength, "Bad integer syntax: '%s'", string);
     else if (strlen (tail) > 0)
       snprintf (str2val_error, errorLength, "Bad integer syntax: '%s'", tail);
     break;
   case (COMMLINE_opt_arg_double) :
     errno = 0;
     auxDouble = strtod (string, &tail);
     if (errno)
       strncpy (str2val_error, "Floating-point number overflow", errorLength);
     else if ((auxDouble == 0) && (tail == string))
       snprintf (str2val_error, errorLength, "Bad floating-point number syntax: '%s'", string);
     else if (strlen (tail) > 0)
       snprintf (str2val_error, errorLength, "Bad floating-point number syntax: '%s'", tail);
     break;
   case (COMMLINE_opt_arg_char) :
     if (strlen (string) != 1)
       strncpy (str2val_error, "Single character expected", errorLength);
     else if (! isalnum (*string))
       snprintf (str2val_error, errorLength, "Invalid char argument \\x%x '%c'", (unsigned int) *string, *string);
     break;
   case (COMMLINE_opt_arg_string) :
   case (COMMLINE_opt_arg_none) :
     break;
   default:
     snprintf (str2val_error, errorLength, "Optional argument type %d unknows", type);
 }
 if (! strcmp (str2val_error, "")) {
   free (str2val_error);
   str2val_error = NULL;
 }
 return (str2val_error);
}

/*
*---------------------------------------------------------------------
*   Validate usage of command line option with boolean argument.
*   The argument is a case-insensitive string.
*   Acceptable values are "yes", "true", "no" and "false".
*   Works only for the first use of the option.
*---------------------------------------------------------------------
*/

void COMMLINE_validate_boolean_option (
 Bool   *p_var,
 int     optIndex,
 Bool    defaultValue,
 char   *varName,
 char   *procName,
 char   *progName )
{
 int
   c_char,
   totOptUses;
 char
   *argStr = NULL;

 if (! COMMLINE_optId2optUses (optIndex, &totOptUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (totOptUses == 0) {
   (*p_var) = defaultValue;
   return;
 }
 if (! COMMLINE_optUse2optArg (optIndex, 1, &argStr))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 for (c_char = 0; c_char < (int) strlen (argStr); c_char++)
   argStr[c_char] = toupper(argStr[c_char]);
 if ((STRINGS_ARE_EQUAL (argStr, "YES")) || (STRINGS_ARE_EQUAL (argStr, "TRUE")))
   (*p_var) = TRUE;
 else if ((STRINGS_ARE_EQUAL (argStr, "NO")) || (STRINGS_ARE_EQUAL (argStr, "FALSE")))
   (*p_var) = FALSE;
 else {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid %s (%s)", varName, argStr);
   ERROR_fatal_error (0, progName, procName, ERROR_auxErrorMsg);
 }
}

/*
*---------------------------------------------------------------------
*   Validate usage of command line option with integer argument.
*   Works only for the first use of the option.
*---------------------------------------------------------------------
*/

void COMMLINE_validate_int_option (
 int  *p_var,
 int   optIndex,
 int   defaultValue,
 int   minValue,
 int   maxValue,
 char *varName,
 char *procName,
 char *progName )
{
 int
   totOptUses;
 unsigned long int
   argULong;

 if (! COMMLINE_optId2optUses (optIndex, &totOptUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (totOptUses == 0)
   (*p_var) = defaultValue;
 else {
   if (! COMMLINE_optUse2optArg (optIndex, 1, &argULong))
     ERROR_short_fatal_error (COMMLINE_get_commLine_error());
   (*p_var) = (int) argULong;
 }
 ERROR_check_range_int (
   (int)    (*p_var),
   (int)    minValue,
   (int)    maxValue,
   (char *) varName,
   (char *) procName,
   (char *) progName );
}

/*
*---------------------------------------------------------------------
*   Validate usage of command line option with short int argument.
*   Works only for the first use of the option.
*---------------------------------------------------------------------
*/

void COMMLINE_validate_short_int_option (
 short int *p_var,
 int        optIndex,
 short int  defaultValue,
 short int  minValue,
 short int  maxValue,
 char      *varName,
 char      *procName,
 char      *progName )
{
 int
   totOptUses;
 short int
   argDouble;

 if (! COMMLINE_optId2optUses (optIndex, &totOptUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (totOptUses == 0)
   (*p_var) = defaultValue;
 else {
   if (! COMMLINE_optUse2optArg (optIndex, 1, &argDouble))
     ERROR_short_fatal_error (COMMLINE_get_commLine_error());
   (*p_var) = argDouble;
 }
 ERROR_check_range_int (
   (int) (*p_var),
   (int) minValue,
   (int) maxValue,
   (char *) varName,
   (char *) procName,
   (char *) progName );
}

/*
*---------------------------------------------------------------------
*   Validate usage of command line option with double argument.
*   Works only for the first use of the option.
*---------------------------------------------------------------------
*/

void COMMLINE_validate_double_option (
 double *p_var,
 int     optIndex,
 double  defaultValue,
 double  minValue,
 double  maxValue,
 char   *varName,
 char   *procName,
 char   *progName )
{
 int
   totOptUses;
 double
   argDouble;

 if (! COMMLINE_optId2optUses (optIndex, &totOptUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (totOptUses == 0)
   (*p_var) = defaultValue;
 else {
   if (! COMMLINE_optUse2optArg (optIndex, 1, &argDouble))
     ERROR_short_fatal_error (COMMLINE_get_commLine_error());
   (*p_var) = argDouble;
 }
 ERROR_check_range_double (
   (double) (*p_var),
   (double) minValue,
   (double) maxValue,
   (char *) varName,
   (char *) procName,
   (char *) progName );
}

/*
*---------------------------------------------------------------------
*   Validate usage of command line option with string argument.
*   Works only for the first use of the option.
*---------------------------------------------------------------------
*/

void COMMLINE_validate_string_option (
 char  **p_var,
 int     optIndex,
 char   *defaultValue,
 int     minLength,
 int     maxLength,
 char   *varName,
 char   *procName,
 char   *progName )
{
 int
   totOptUses;
 size_t
   strSize,
   varDescrLength;
 char
   *argStr   = NULL,
   *varDescr = NULL;

 if (! COMMLINE_optId2optUses (optIndex, &totOptUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (totOptUses == 0) {
   if (defaultValue == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "%s was not provided", varName);
     ERROR_fatal_error (0, progName, procName, ERROR_auxErrorMsg);
   }
   argStr = defaultValue;
 }
 else if (! COMMLINE_optUse2optArg (optIndex, 1, &argStr))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 strSize = strlen (argStr);
 varDescrLength = strlen (varName) + 20;
 errno = 0;
 varDescr = (char *) realloc (varDescr, varDescrLength);
 if (varDescr == NULL)
   ERROR_no_memory (errno, progName, procName, "varDescr");
 snprintf (varDescr, varDescrLength, "length of %s", varName);
 ERROR_check_range_int (
   (int)    strSize,
   (int)    minLength,
   (int)    maxLength,
   (char *) varDescr,
   (char *) procName,
   (char *) progName );
 errno = 0;
 (*p_var) = (char *) realloc (*p_var, strSize + 2);
 if ((*p_var) == NULL)
   ERROR_no_memory (errno, progName, procName, varName);
 strncpy (*p_var, argStr, strSize + 2);
 free (argStr);
 free (varDescr);
}

/*
*---------------------------------------------------------------------
*   Validate usage of command line option with string argument.
*   Works for any use of the option.
*---------------------------------------------------------------------
*/

void COMMLINE_validate_oneOf_string_options (
 char  **p_var,
 int     optIndex,
 int     optUse,
 char   *defaultValue,
 int     minLength,
 int     maxLength,
 char   *varName,
 char   *procName,
 char   *progName )
{
 int
   totOptUses;
 size_t
   strSize,
   varDescrLength;
 char
   *argStr   = NULL,
   *varDescr = NULL;

 if (! COMMLINE_optId2optUses (optIndex, &totOptUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (totOptUses == 0) {
   if (defaultValue == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "%s was not provided", varName);
     ERROR_fatal_error (0, progName, procName, ERROR_auxErrorMsg);
   }
   argStr = defaultValue;
 }
 else if (! COMMLINE_optUse2optArg (optIndex, optUse, &argStr))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 strSize = strlen (argStr);
 varDescrLength = strlen (varName) + 20;
 errno = 0;
 varDescr = (char *) realloc (varDescr, varDescrLength);
 if (varDescr == NULL)
   ERROR_no_memory (errno, progName, procName, "varDescr");
 snprintf (varDescr, varDescrLength, "length of %s", varName);
 ERROR_check_range_int (
   (int)    strSize,
   (int)    minLength,
   (int)    maxLength,
   (char *) varDescr,
   (char *) procName,
   (char *) progName );
 errno = 0;
 (*p_var) = (char *) realloc (*p_var, strSize + 2);
 if ((*p_var) == NULL)
   ERROR_no_memory (errno, progName, procName, varName);
 strncpy (*p_var, argStr, strSize + 2);
 free (argStr);
 free (varDescr);
}

/*
*---------------------------------------------------------------------
*   Display detailed info about the options the calling program takes.
*---------------------------------------------------------------------
*/

#ifdef FORMAT_LENGTH
#undef FORMAT_LENGTH
#endif
#define FORMAT_LENGTH 80

#ifdef TYPENAME_LENGTH
#undef TYPENAME_LENGTH
#endif
#define TYPENAME_LENGTH 10

void COMMLINE_display_usage (void)
{
 int
   c_arg,
   c_char,
   c_excl,
   c_opt,
   excls,
   mostExcls,
   optIndex,
  *p_exclID;
 size_t
   ending,
   lineSize,
   nameLength,
   longestName;
 char
    format   [FORMAT_LENGTH],
    typeName [TYPENAME_LENGTH],
   *blanks = NULL,
   *dashes = NULL,
   *header = NULL;
 t_optData
   *p_optData;
 t_nonOptArgsData
  *p_argData;

 if (! b_formalOpts_ok) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::%s(): Command line parse error",
     program_name,
     __func__ );
   return;
 }
 printf ("\nUsage: %s ", program_name);
 if (formal_options > 0)
   printf ("[options] ");
 if (b_nonOptArgs_ok)
   for (c_arg = 0, p_argData = nonOptArgsData;
          c_arg < non_opt_args;
            c_arg++, p_argData++ )
     switch (p_argData->argType) {
       case (COMMLINE_opt_arg_long_int)  : printf ("long ")  ; break;
       case (COMMLINE_opt_arg_ulong_int) : printf ("u_long "); break;
       case (COMMLINE_opt_arg_double)    : printf ("double "); break;
       case (COMMLINE_opt_arg_char)      : printf ("char ")  ; break;
       case (COMMLINE_opt_arg_string)    : printf ("string "); break;
       case (COMMLINE_opt_arg_none)      :                     break;
       default:
         snprintf (commandLineError,
           ERROR_maxErrorMsgSize,
           "%s::%s(): Argument type %d unknown",
           program_name,
           __func__,
           p_argData->argType );
         return;
     }
 else
   printf ("...");

 printf ("\n");

 if (formal_options == 0)
   return;

 printf ("\n");
 longestName = 0;
 mostExcls = 0;
 for (c_opt = 0, p_optData = optData;
	c_opt < formal_options;
          c_opt++, p_optData++ ) {
   nameLength = strlen (p_optData->optStr);
   if (nameLength > longestName)
     longestName = nameLength;
   excls = p_optData->optTotExcls;
   if (excls > mostExcls)
     mostExcls = excls;
 }
 if (longestName < 4)
   longestName = 4;
 ending = (size_t) (mostExcls < 4 ? 10: 3 * mostExcls);
 lineSize = (size_t) 31 + longestName + ending;

 dashes = (char *) malloc ((lineSize + 1) * sizeof (char));
 if (dashes ==  NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_display_usage(): Malloc error [dashes]",
     program_name );
   return;
 }
 for (c_char = 0; c_char < (int) lineSize; c_char++)
   dashes [c_char] = '-';
 dashes [c_char] = '\0';

 blanks = (char *) malloc ((lineSize + 1) * sizeof (char));
 if (blanks ==  NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_display_usage(): Malloc error [blanks]",
     program_name );
   return;
 }
 for (c_char = 0; c_char < (int) lineSize; c_char++)
   blanks [c_char] = ' ';
 blanks [c_char] = '\0';

 header = (char *) malloc ((lineSize + 1) * sizeof (char));
 if (header ==  NULL) {
   snprintf (commandLineError,
     ERROR_maxErrorMsgSize,
     "%s::COMMLINE_display_usage(): Malloc error [header]",
     program_name );
   return;
 }

 strcpy  (header, "---+--------");
 strncat (header, dashes, longestName);
 strcat  (header, "-+--------+-------+");
 strncat (header, dashes, ending);
 printf  ("%s\n", header);

 snprintf (format, FORMAT_LENGTH, "   |Option  %%-%d.%ds | Arg    | Uses  | Excludes \n",
   (int) longestName, (int) longestName);
 printf (format, "name");
 snprintf (format, FORMAT_LENGTH, " # |Short   %%-%d.%ds |   type |Min Max|          \n",
   (int) longestName, (int) longestName);
 printf (format, "Long");
 printf  ("%s\n", header);

 snprintf (format, FORMAT_LENGTH, "%%2d | -%%c   --%%-%d.%ds | %%-6.6s | %%2d ",
   (int) longestName, (int) longestName);
 for (c_opt = 0, p_optData = optData;
	c_opt < formal_options;
          c_opt++, p_optData++ ) {
   switch (p_optData->optArgType) {
     case (COMMLINE_opt_arg_none)      : strncpy (typeName, "",       TYPENAME_LENGTH); break;
     case (COMMLINE_opt_arg_long_int)  : strncpy (typeName, "long",   TYPENAME_LENGTH); break;
     case (COMMLINE_opt_arg_ulong_int) : strncpy (typeName, "u_long", TYPENAME_LENGTH); break;
     case (COMMLINE_opt_arg_double)    : strncpy (typeName, "double", TYPENAME_LENGTH); break;
     case (COMMLINE_opt_arg_char)      : strncpy (typeName, "char",   TYPENAME_LENGTH); break;
     case (COMMLINE_opt_arg_string)    : strncpy (typeName, "string", TYPENAME_LENGTH); break;
     default: break;
   }
   printf (format,
     c_opt + 1,
     p_optData->optChar,
     p_optData->optStr,
     typeName,
     p_optData->optMinUses,
     p_optData->optMaxUses );
   if (p_optData->optMaxUses == COMMLINE_OPT_FREE_USE)
     printf (" + | ");
   else
     printf ("%2d | ", p_optData->optMaxUses);
   for (c_excl = 0, p_exclID = p_optData->optExclList;
	  c_excl < p_optData->optTotExcls;
	    c_excl++, p_exclID++ ) {
     optIndex = optID2optFormalIndex (*p_exclID);
     printf ("%2d ", optIndex + 1);
   }
   printf ("\n");
 }

 printf ("%s\n", header);
 printf ("[+] unlimited use,  [u_long] unsigned long int\n\n");
 free (header);
 free (blanks);
 free (dashes);
}


