/*
*------------------------------------------------------------------------
*
*   File         : mystrings.c
*   Created      : 2012-05-30
*   Last Modified: 2012-05-31
*
*   Safe manipulation of malloc'ed strings
*
*------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------
*   INCLUDE FILES
*------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "error.h"
#include "mystrings.h"

/*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION (not visible from other modules)
*
*---------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------
* Allocate an empty string. Return a pointer to it. The malloc'ed string
* should be freed at some point with a call to STRING_release().
*------------------------------------------------------------------------
*/

char *STRING_allocate (void) {
  char
    *newString;
  errno = 0;
  newString = (char *) malloc (sizeof(char));
  if (newString == NULL)
    ERROR_no_memory (errno, __FILE__, __func__, "newString");
  *newString = '\0';
  return (newString);
}

/*
*------------------------------------------------------------------------
* Set a string to the empty string, i.e. ""
* Return a pointer to the resulting string.
*
* NOTE:
*
* The call to realloc() may cause the string to be moved to a different
* memory location. This change of string address must take effect
* on the calling environment, so the destination string MUST be passed
* by reference.
*------------------------------------------------------------------------
*/

char *STRING_clear (char **stringVar) {
  errno = 0;
  *stringVar = (char *) realloc ((void *) *stringVar, sizeof(char));
  if (*stringVar == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "STRING_clear(\"NULL\"): realloc failed");
    ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  **stringVar = '\0';
  return (*stringVar);
}

/*
*------------------------------------------------------------------------
* Copy a string to a previously malloc'ed destination string.
* The original contents of the destination string is lost.
* Return a pointer to the resulting string.
*
* NOTE:
*
* The call to realloc() may cause the string to be moved to a different
* memory location. This change of string address must take effect
* on the calling environment, so the destination string MUST be passed
* by reference.
*------------------------------------------------------------------------
*/

char *STRING_copy (char **destString, const char *sourceString) {
  char
    *resultString;
  size_t
    resultStringLength = strlen(sourceString),
    resultStringMemBlockSize = (resultStringLength + 1) * sizeof(char);

  errno = 0;
  resultString = (char *) realloc ((void *) *destString, resultStringMemBlockSize);
  if (resultString == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
      "STRING_copy(\"%s\",\"%s\"): realloc(\"%s\",%d) failed",
      *destString, sourceString, *destString, (int) resultStringMemBlockSize);
    ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  (void) strncpy (resultString, sourceString, resultStringLength);
  if (errno != 0) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
      "STRING_copy(\"%s\",\"%s\"): strncpy(\"%s\",\"%s\",%d) failed",
      *destString, sourceString, resultString, sourceString, (int) resultStringLength);
    ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  resultString[resultStringLength] = 0;
  *destString = resultString;
  return (resultString);
}

/*
*------------------------------------------------------------------------
* Copy N chars of a string to a previously malloc'ed destination string.
* The original contents of the destination string is lost.
* - If N <= the source string length, only the first N chars are copied.
* - If N > the source string length, the destination is padded with '\0'.
* The source string is unchanged.
* Return a pointer to the resulting string.
* If the strings overlap, the behavior is undefined.
*
* NOTE:
*
* The call to realloc() may cause the string to be moved to a different
* memory location. This change of string address must take effect
* on the calling environment, so the destination string MUST be passed
* by reference.
*------------------------------------------------------------------------
*/

char *STRING_n_copy (char **destString, const char *sourceString, size_t charsToCopy) {
  size_t
    sourceStrLength,
    resultStringLength,
    resultStringMemBlockSize,
    charsInSourceToActuallyCopy,
    charsInDestToPad;
  char
    *resultString;

  sourceStrLength             = strlen (sourceString);
  charsInSourceToActuallyCopy = (charsToCopy < sourceStrLength ? charsToCopy : sourceStrLength);
  charsInDestToPad            = (charsToCopy > sourceStrLength ? charsToCopy - sourceStrLength : 0);
  resultStringLength          = charsInSourceToActuallyCopy + charsInDestToPad;
  resultStringMemBlockSize    = (resultStringLength + 1) * sizeof(char);
  errno = 0;
  resultString = (char *) realloc ((void *) *destString, resultStringMemBlockSize);
  if (resultString == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
      "STRING_n_copy(\"%s\",\"%s\",%d): realloc(\"%s\",%d) failed",
      *destString, sourceString, (int) charsToCopy, *destString, (int) resultStringMemBlockSize);
    ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  (void) strncpy (resultString, sourceString, charsInSourceToActuallyCopy);
  if (errno != 0) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
      "STRING_n_copy(\"%s\",\"%s\",%d): strncpy(\"%s\",\"%s\",%d) failed",
      *destString, sourceString, (int) charsToCopy, resultString, sourceString, (int) charsInSourceToActuallyCopy);
    ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  if (charsInDestToPad > 0)
    memset (&(resultString[charsInSourceToActuallyCopy]), 0, charsInDestToPad);
  resultString[resultStringLength] = 0;
  *destString = resultString;
  return (resultString);
}

/*
*------------------------------------------------------------------------
* Append a copy of a source string (including the terminating null byte)
* to the end of another.
* The first byte of the source string overwrites the null byte at the
* end of the destination string.
* The source string is unchanged.
* Return a pointer to the resulting string.
* If the strings overlap, the behavior is undefined.
*
* NOTE:
*
* The call to realloc() may cause the string to be moved to a different
* memory location. This change of string address must take effect
* on the calling environment, so the destination string MUST be passed
* by reference.
*------------------------------------------------------------------------
*/

char *STRING_concatenate (char **destString, const char *sourceString) {
  size_t
    destStringLength,
    sourceStringLength,
    resultStringLength,
    resultStringMemBlockSize;
  char
    *resultString;

  destStringLength   = strlen (*destString);
  sourceStringLength = strlen (sourceString);
  resultStringLength = destStringLength + sourceStringLength;
  resultStringMemBlockSize = (resultStringLength + 1) * sizeof(char);
  errno = 0;
  resultString = (char *) realloc ((void *) *destString, resultStringMemBlockSize);
  if (resultString == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
      "STRING_concatenate(\"%s\",\"%s\"): realloc(\"%s\",%d) failed",
      *destString, sourceString, *destString, (int) resultStringMemBlockSize);
    ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  (void) strncpy (&(resultString[destStringLength]), sourceString, sourceStringLength);
  if (errno != 0) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
      "STRING_concatenate(\"%s\",\"%s\"): strncpy(\"%s\",\"%s\",%d) failed",
      *destString, sourceString, resultString, sourceString, (int) sourceStringLength);
    ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  resultString[resultStringLength] = 0;
  *destString = resultString;
  return (resultString);
}

/*
*------------------------------------------------------------------------
* Store N copies of a given character in a previously malloc'ed string.
* The string is resized and its original contents is lost.
* The null byte that terminates the string does not count towards N,
* so the string ends up with N+1 bytes.
* If N = 0, this is equivalent to a call to STRING_clear().
* Return a pointer to the resulting string.
*
* NOTE:
*
* The call to realloc() may cause the string to be moved to a different
* memory location. This change of string address must take effect
* on the calling environment, so the string MUST be passed by reference.
*------------------------------------------------------------------------
*/

char *STRING_set (char **stringVar, unsigned char charToSet, size_t timesToAdd) {
  size_t
    resultStringMemBlockSize;
  char
    *resultString;

  resultStringMemBlockSize = (timesToAdd + 1) * sizeof(char);
  errno = 0;
  resultString = (char *) realloc ((void *) *stringVar, resultStringMemBlockSize);
  if (resultString == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
      "STRING_set(\"%s\",'%c',%zd): realloc(\"%s\",%zd) failed",
      *stringVar, charToSet, timesToAdd, *stringVar, resultStringMemBlockSize);
    ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  memset (resultString, (int) charToSet, timesToAdd);
  resultString[timesToAdd] = 0;
  *stringVar = resultString;
  return (resultString);
}

/*
*------------------------------------------------------------------------
* Append N copies of a given character to a previously malloc'ed string.
* If N = 0, the string is unchanged.
* Return a pointer to the resulting string.
*
* Designed to set the string in one or more calls. Example:
*
*   STRING_extend (&line, '+',  1);
*   STRING_extend (&line, '-', 10);
*   STRING_extend (&line, '+',  1);
*
* NOTE:
*
* The call to realloc() may cause the string to be moved to a different
* memory location. This change of string address must take effect
* on the calling environment, so the string MUST be passed by reference.
*------------------------------------------------------------------------
*/

char *STRING_extend (char **stringVar, unsigned char charToAdd, size_t timesToAdd) {
  size_t
    originalStringLength,
    resultStringLength,
    resultStringMemBlockSize;
  char
    *resultString;

  originalStringLength = strlen (*stringVar);
  resultStringLength = originalStringLength + timesToAdd;
  resultStringMemBlockSize = (resultStringLength + 1) * sizeof(char);
  errno = 0;
  resultString = (char *) realloc ((void *) *stringVar, resultStringMemBlockSize);
  if (resultString == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
      "STRING_extend(\"%s\",'%c',%zd): realloc(\"%s\",%zd) failed",
      *stringVar, charToAdd, timesToAdd, *stringVar, resultStringMemBlockSize);
    ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  memset (&(resultString[originalStringLength]), (int) charToAdd, timesToAdd);
  resultString[resultStringLength] = 0;
  *stringVar = resultString;
  return (resultString);
}

/*
*------------------------------------------------------------------------
* Free the memory block previously allocated to a string.
* The string must be passed by reference.
*------------------------------------------------------------------------
*/

void STRING_release (char **stringToFree) {
  if (*stringToFree != NULL) {
    free (*stringToFree);
    *stringToFree = NULL;
  }
}

/*
*-----------------------------------------------------------------------
* Convert a string to lowercase.
* Works with both satically and dinamically allocated strings.
*-----------------------------------------------------------------------
*/

void STRING_convert_lower_case (char *p_str)
{
 if (! p_str)
   return;
 for ( ; *p_str != 0; p_str++)
   *p_str = tolower(*p_str);
}

/*
*-----------------------------------------------------------------------
* Convert a string to uppercase.
* Works with both satically and dinamically allocated strings.
*-----------------------------------------------------------------------
*/

void STRING_convert_upper_case (char *p_str)
{
 if (! p_str)
   return;
 for ( ; *p_str != 0; p_str++)
   *p_str = toupper(*p_str);
}

/*
*---------------------------------------------------------------------------------
* Take a previously malloc'ed string, add leading and/or trailing characters,
* then trim it to size.
* The memory block occupied by the string is likely to be resized.
* Return a pointer to the resulting string.
* Example:
*
*     char
*       *cadeia01 = STRING_allocate(),
*       *cadeia02 = STRING_allocate(),
*       *cadeia03 = STRING_allocate(),
*       *cadeia04 = STRING_allocate(),
*       *cadeia05 = STRING_allocate(),
*       *cadeia06 = STRING_allocate(),
*       *cadeia07 = STRING_allocate(),
*       *cadeia08 = STRING_allocate(),
*       *cadeia09 = STRING_allocate(),
*       *cadeia10 = STRING_allocate(),
*       *cadeia11 = STRING_allocate(),
*       *cadeia12 = STRING_allocate(),
*       *cadeia13 = STRING_allocate(),
*       *cadeia14 = STRING_allocate(),
*       *cadeia15 = STRING_allocate(),
*       *cadeia16 = STRING_allocate(),
*       *cadeia17 = STRING_allocate(),
*       *cadeia18 = STRING_allocate();
*
*     (void) STRING_copy (&cadeia01, "abcde");
*     (void) STRING_copy (&cadeia02, "abcde");
*     (void) STRING_copy (&cadeia03, "abcde");
*     (void) STRING_copy (&cadeia04, "abcde");
*     (void) STRING_copy (&cadeia05, "abcde");
*     (void) STRING_copy (&cadeia06, "abcde");
*     (void) STRING_copy (&cadeia07, "abcde");
*     (void) STRING_copy (&cadeia08, "abcde");
*     (void) STRING_copy (&cadeia09, "abcde");
*     (void) STRING_copy (&cadeia10, "abcde");
*     (void) STRING_copy (&cadeia11, "abcde");
*     (void) STRING_copy (&cadeia12, "abcde");
*     (void) STRING_copy (&cadeia13, "abcde");
*     (void) STRING_copy (&cadeia14, "abcde");
*     (void) STRING_copy (&cadeia15, "abcde");
*     (void) STRING_copy (&cadeia16, "abcde");
*     (void) STRING_copy (&cadeia17, "abcde");
*     (void) STRING_copy (&cadeia18, "abcde");
*
*     (void) STRING_justify (&cadeia01, 7, '-', STRING_t_justify_left)  ;    // -->  "abcde--"
*     (void) STRING_justify (&cadeia02, 6, '-', STRING_t_justify_left)  ;    // -->  "abcde-"
*     (void) STRING_justify (&cadeia03, 5, '-', STRING_t_justify_left)  ;    // -->  "abcde"
*     (void) STRING_justify (&cadeia04, 4, '-', STRING_t_justify_left)  ;    // -->  "abcd"
*     (void) STRING_justify (&cadeia05, 3, '-', STRING_t_justify_left)  ;    // -->  "abc"
*     (void) STRING_justify (&cadeia06, 7, '-', STRING_t_justify_centre);    // -->  "-abcde-"
*     (void) STRING_justify (&cadeia07, 6, '-', STRING_t_justify_centre);    // -->  "-abcde"
*     (void) STRING_justify (&cadeia08, 5, '-', STRING_t_justify_centre);    // -->  "abcde"
*     (void) STRING_justify (&cadeia09, 4, '-', STRING_t_justify_centre);    // -->  "bcde"
*     (void) STRING_justify (&cadeia10, 3, '-', STRING_t_justify_centre);    // -->  "bcd"
*     (void) STRING_justify (&cadeia11, 7, '-', STRING_t_justify_right) ;    // -->  "--abcde"
*     (void) STRING_justify (&cadeia12, 6, '-', STRING_t_justify_right) ;    // -->  "-abcde"
*     (void) STRING_justify (&cadeia13, 5, '-', STRING_t_justify_right) ;    // -->  "abcde"
*     (void) STRING_justify (&cadeia14, 4, '-', STRING_t_justify_right) ;    // -->  "bcde"
*     (void) STRING_justify (&cadeia15, 3, '-', STRING_t_justify_right) ;    // -->  "cde"
*     (void) STRING_justify (&cadeia16, 2, '-', STRING_t_justify_right) ;    // -->  "de"
*     (void) STRING_justify (&cadeia17, 1, '-', STRING_t_justify_right) ;    // -->  "e"
*     (void) STRING_justify (&cadeia18, 0, '-', STRING_t_justify_right) ;    // -->  ""
*
*     printf ("\"%s\"\n", cadeia01);
*     printf ("\"%s\"\n", cadeia02);
*     printf ("\"%s\"\n", cadeia03);
*     printf ("\"%s\"\n", cadeia04);
*     printf ("\"%s\"\n", cadeia05);
*     printf ("\"%s\"\n", cadeia06);
*     printf ("\"%s\"\n", cadeia07);
*     printf ("\"%s\"\n", cadeia08);
*     printf ("\"%s\"\n", cadeia09);
*     printf ("\"%s\"\n", cadeia10);
*     printf ("\"%s\"\n", cadeia11);
*     printf ("\"%s\"\n", cadeia12);
*     printf ("\"%s\"\n", cadeia13);
*     printf ("\"%s\"\n", cadeia14);
*     printf ("\"%s\"\n", cadeia15);
*     printf ("\"%s\"\n", cadeia16);
*     printf ("\"%s\"\n", cadeia17);
*     printf ("\"%s\"\n", cadeia18);
*
*     STRING_release (&cadeia01);
*     STRING_release (&cadeia02);
*     STRING_release (&cadeia03);
*     STRING_release (&cadeia04);
*     STRING_release (&cadeia05);
*     STRING_release (&cadeia06);
*     STRING_release (&cadeia07);
*     STRING_release (&cadeia08);
*     STRING_release (&cadeia09);
*     STRING_release (&cadeia10);
*     STRING_release (&cadeia11);
*     STRING_release (&cadeia12);
*     STRING_release (&cadeia13);
*     STRING_release (&cadeia14);
*     STRING_release (&cadeia15);
*     STRING_release (&cadeia16);
*     STRING_release (&cadeia17);
*     STRING_release (&cadeia18);
*
* NOTE:
*
* The call to realloc() may cause the string to be moved to a different
* memory location. This change of string address must take effect
* on the calling environment, so the string MUST be passed by reference.
*---------------------------------------------------------------------------------
*/

char *STRING_justify (char **originalString, size_t finalLength, char filler, STRING_t_justification justification)
{
 size_t
   originalLength,
   resultStringMemBlockSize,
   leadingFilling,
   leadingChops,
   trailingFilling;
/*
 size_t
   trailingChops;
*/
 char
   *resultString = NULL;

 originalLength = strlen (*originalString);
 if (finalLength == originalLength)
   return (*originalString);
 resultStringMemBlockSize = (finalLength + 1) * sizeof(char);
 errno = 0;
 resultString = (char *) malloc (resultStringMemBlockSize);
 if (resultString == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
     "STRING_justify(\"%s\",%zd,'%c',%d): malloc(%zd) failed",
     *originalString, finalLength, filler, (int) justification, resultStringMemBlockSize);
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 switch (justification) {
   case (STRING_t_justify_left):
     if (finalLength > originalLength) {
       memcpy (resultString, *originalString, originalLength);
       memset (&(resultString[originalLength]), (int) filler, finalLength - originalLength);
     }
     else {
       memcpy (resultString, *originalString, finalLength);
     }
     resultString[finalLength] = 0;
     break;
   case (STRING_t_justify_right):
     if (finalLength > originalLength) {
       memset (resultString, (int) filler, finalLength - originalLength);
       memcpy (&(resultString[finalLength - originalLength]), *originalString, originalLength);
     }
     else {
       memcpy (resultString, &(*originalString[originalLength - finalLength]), finalLength);
     }
     resultString[finalLength] = 0;
     break;
   case (STRING_t_justify_centre):
     if (finalLength > originalLength) {
       leadingFilling = (size_t) ROUND ((finalLength - originalLength) / 2.0);
       trailingFilling = finalLength - originalLength - leadingFilling;
       memset (resultString, (int) filler, leadingFilling);
       memcpy (&(resultString[leadingFilling]), *originalString, originalLength);
       memset (&(resultString[leadingFilling+originalLength]), (int) filler, trailingFilling);
     }
     else {
       leadingChops = (size_t) ROUND ((originalLength - finalLength) / 2.0);
	   /*
       trailingChops = originalLength - finalLength - leadingChops;
	   */
       memcpy (resultString, &(*originalString)[leadingChops], finalLength);
     }
     resultString[finalLength] = 0;
     break;
   default:   /* This should never happen */
     errno = 0;
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknow justification type %d while processing string %s", justification, *originalString);
     ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 free (*originalString);
 *originalString = resultString;
 return (resultString);
}

