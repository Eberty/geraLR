/*
*------------------------------------------------------------------------
*
*   File         : mystrings.h
*   Created      : 2012-05-30
*   Last Modified: 2012-05-31
*
*   Safe manipulation of malloc'ed strings
*
*------------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _MY_STRINGS_DOT_H_
#define _MY_STRINGS_DOT_H_

/*
*------------------------------------------------------------------------
*   INCLUDE FILES
*------------------------------------------------------------------------
*/

/*                         */
/* Where size_t is defined */
/*                         */

#include <stddef.h>

/*
*---------------------------------------------------------------------
*   Definition of constants and macros
*---------------------------------------------------------------------
*/

/*                                                    */
/* Macros for making string comparison more intuitive */
/*                                                    */

#define STRINGS_ARE_EQUAL(s1,s2)      (!strcmp ((s1),(s2)))
#define STRINGS_ARE_DIFFERENT(s1,s2)  ( strcmp ((s1),(s2)))

/* A safer alternative to the standard strcpy() function using strncpy()             */
/*                                                                                   */
/* PROTOTYPE:  char *strncpy (char *toStr, const char *fromStr, size_t maxChars);    */
/*                                                                                   */
/* IMPORTANT: Bear in mind the following facts about the strncpy() function:         */
/*                                                                                   */
/* - It copies at most maxChars bytes from the array pointed to by fromStr to the    */
/*   array pointed to by toStr.                                                      */
/* - Bytes that follow a null byte are not copied.                                   */
/* - If fromStr is shorter than maxChars bytes, null bytes are appended to the copy  */
/*   in toStr, until maxChars bytes in all are written.                              */
/* - If there is no null byte in the first maxChars bytes of the array pointed to by */
/*   fromStr, the result will not be null-terminated.                                */
/* - Character movement is performed differently in different implementations. Thus, */
/*   if copying takes place between objects that overlap, the behaviour is undefined */
/* - It returns the destination string.                                              */
/* - No return value is reserved to flag an error.                                   */
/*                                                                                   */
/* This macro guarantees that the destination string is NULL terminated.             */
/* Nothing is returned.                                                              */

#define STRING_SAFER_STRCPY(toString,fromString,availableBytes)                        \
  (void) strncpy ((char *)(toString), (const char *)(fromString), (availableBytes)-1); \
  *((toString)+(availableBytes)-1) = '\0';

/*
*---------------------------------------------------------------------
*   Type definitions
*---------------------------------------------------------------------
*/

typedef enum {
  STRING_t_justify_left,
  STRING_t_justify_centre,
  STRING_t_justify_right
}
  STRING_t_justification;

/*
*---------------------------------------------------------------------
*   Procedures defined in "mystrings.c"
*---------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------
* Allocate an empty string. Return a pointer to it. The malloc'ed string 
* should be freed at some point with a call to STRING_release().
*------------------------------------------------------------------------
*/

extern char *STRING_allocate (void);

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

extern char *STRING_clear (char **stringVar);

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

extern char *STRING_copy (char **destString, const char *sourceString);

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

extern char *STRING_n_copy (char **destString, const char *sourceString, size_t charsToCopy);

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

extern char *STRING_concatenate (char **destString, const char *sourceString);

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

extern char *STRING_set (char **stringVar, unsigned char charToSet, size_t timesToAdd);

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

extern char *STRING_extend (char **stringVar, unsigned char charToAdd, size_t timesToAdd);

/*
*-----------------------------------------------------------------------
* Convert a string to lowercase.
* Works with both satically and dinamically allocated strings.
*-----------------------------------------------------------------------
*/

extern void STRING_convert_lower_case (char *p_str);

/*
*-----------------------------------------------------------------------
* Convert a string to uppercase.
* Works with both satically and dinamically allocated strings.
*-----------------------------------------------------------------------
*/

extern void STRING_convert_upper_case (char *p_str);

/*
*---------------------------------------------------------------------------------------
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
*---------------------------------------------------------------------------------------
*/

extern char *STRING_justify (
 char                   **originalString,
 size_t                   finalLength,
 char                     filler,
 STRING_t_justification   justification );

/*
*------------------------------------------------------------------------
* Free the memory block previously allocated to a string.
* The string must be passed by reference.
*------------------------------------------------------------------------
*/

extern void STRING_release (char **stringToFree);

#endif /* ifndef _MY_STRINGS_DOT_H_ */
