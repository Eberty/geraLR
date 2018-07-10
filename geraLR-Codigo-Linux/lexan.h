/*
*---------------------------------------------------------------------
*
*   File         : lexan.h
*   Created      : 1996-03-26
*   Last Modified: 2012-05-30
*
*   Simple lexical analyser - ERROR MESSAGES IN ENGLISH
*
*---------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _LEXAN_DOT_H_
#define _LEXAN_DOT_H_

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include "regex.h"

/*
*---------------------------------------------------------------------
*   Constants
*---------------------------------------------------------------------
*/

/*                                                   */
/* Default for regular expression syntax options.    */
/* See comments at the end of this file for details. */
/*                                                   */

#define LEXAN_REGEX_SYNTAX      \
  ( RE_CHAR_CLASSES          |  \
    RE_DOT_NEWLINE           |  \
    RE_DOT_NOT_NULL          |  \
    RE_INTERVALS             |  \
    RE_NO_EMPTY_RANGES       |  \
    RE_CONTEXT_INDEP_ANCHORS |  \
    RE_NO_BK_BRACES )

/*                                                                    */
/* Useful regular expressions:                                        */
/*   string literals, identifiers, integer and real numbers.          */
/*                                                                    */
/* The double backslashes are awkward but necessary:                  */
/* - the first one escapes the "(" , ")" and "|" operators in regular */
/*   expressions. If un-escaped, they are treated as literals, which  */
/*   allows us to use them elsewhere for other purposes, eg. "(*" for */
/*   beginning of multi-line comments and "|" for table separators.   */
/*   This behaviour is determined by LEXAN_REGEX_SYNTAX above.        */
/* - the second backslash escapes the first one! A backslash in a     */
/*   literal string is treated as part of an escape sequence, such as */
/*   "\n" for newline and so on. We don't want that, we want the      */
/*   backslash to be a backslash, so we have to escape it.            */
/*                                                                    */
/* This is what the regular expressions would look like without all   */
/* these backslashes:                                                 */
/*                                                                    */
/*   "\"\(.*\)\""                                                     */
/*   "\([[:alpha:]][[:alnum:]_]*\)"                                   */
/*   "\([-+]?[0-9]+\)"                                                */
/*   "\([-+]?[0-9]*\.[0-9]*\([eE][-+]?[0-9]+\)?\)"                    */
/*                                                                    */

#define LEXAN_REGEX_SINGLE_STR "\\('[^']*'\\)"
#define LEXAN_REGEX_DOUBLE_STR "\\(\"[^\"]*\"\\)"
#define LEXAN_REGEX_IDEN       "\\([[:alpha:]][[:alnum:]_]*\\)"
#define LEXAN_REGEX_INT        "\\([-+]?[0-9]+\\)"
#define LEXAN_REGEX_REAL       "\\([-+]?[0-9]*\\.[0-9]*\\([eE][-+]?[0-9]+\\)?\\)"

/*
*---------------------------------------------------------------------
*   Data structures and variables
*---------------------------------------------------------------------
*/

/*                                                                              */
/* The scanner can recognize 10 types of tokens. Suggested usage is             */
/* listed below, with examples of the corresponding regular expressions.        */
/* These are suggestions only. For instance, it is perfectly valid to           */
/* ignore "LEXAN_token_resWord" and use "LEXAN_token_iden" instead,             */
/* both for special keywords and identifiers in general.                        */
/*                                                                              */
/* LEXAN_token_single_str                                                       */
/*   String literals enclosed in single quotes.                                 */
/*   Regex: LEXAN_REGEX_SINGLE_STR                                              */
/*                                                                              */
/* LEXAN_token_double_str                                                       */
/*   String literals enclosed in double quotes.                                 */
/*   Regex: LEXAN_REGEX_DOUBLE_STR                                              */
/*                                                                              */
/* LEXAN_token_iden                                                             */
/*   Any identifier such as variable names.                                     */
/*   Regex: LEXAN_REGEX_IDEN                                                    */
/*                                                                              */
/* LEXAN_token_resWord                                                          */
/*   Special identifiers such as reserved words.                                */
/*   Regex: LEXAN_REGEX_IDEN                                                    */
/*                                                                              */
/* LEXAN_token_longInt                                                          */
/*   Long integers.                                                             */
/*   Regex: LEXAN_REGEX_INT                                                     */
/*                                                                              */
/* LEXAN_token_double                                                           */
/*   Double-precision floating-point numbers.                                   */
/*   Regex: LEXAN_REGEX_DOUBLE                                                  */
/*                                                                              */
/* LEXAN_token_delim                                                            */
/*   Single characters, eg. '<' '>' ';' ':' '.' '$'                             */
/*   Regex: "([<>;\\:\\.\\$])"                                                  */
/*                                                                              */
/* LEXAN_token_EOF                                                              */
/*   End of file.                                                               */
/*   Returned by the scanner when it reaches the end of the file.               */
/*   Cannot be used in the parameter list to lexan_get_token().                 */
/*                                                                              */
/* LEXAN_token_NULL                                                             */
/*   Lexical error, ie. unrecognized character or string.                       */
/*   Cannot be used in the parameter list to lexan_get_token().                 */
/*                                                                              */
/* LEXAN_token_LINE_COMMENT                                                     */
/*   One-line comments.                                                         */
/*   Not exactly a token; it is never returned by the scanner.                  */
/*   When the scanner finds one in the input file, it ignores it                */
/*   as well as the rest of the current line, and resumes scanning              */
/*   at the beginning of the next line. Useful for specifying                   */
/*   character(s) marking one-line comments, eg. "#" or "//".                   */
/*   Regex: "(#|//)"                                                            */
/*                                                                              */
/* LEXAN_token_OPEN_COMMENT                                                     */
/*   Opens multi-line comments.                                                 */
/*   Can only be used once in the parameter list to lexan_get_token().          */
/*   Not exactly a token; it is never returned by the scanner.                  */
/*   When the scanner finds one in the input file, it ignores all               */
/*   text until a LEXAN_token_CLOSE_COMMENT is found, and resumes scanning      */
/*   at that point. If the scanner reaches the end of the file and              */
/*   a LEXAN_token_CLOSE_COMMENT is not found, it returns LEXAN_token_NULL      */
/*   and produces an error message.                                             */
/*   Regex: "/\\*"  (for C-style comments).                                     */
/*                                                                              */
/* LEXAN_token_CLOSE_COMMENT                                                    */
/*   Closes multi-line comments. To be used in conjunction with                 */
/*   LEXAN_token_OPEN_COMMENT. If the scanner finds a LEXAN_token_CLOSE_COMMENT */
/*   before a LEXAN_token_CLOSE_COMMENT, it returns LEXAN_token_NULL and        */
/*   produces an error message.                                                 */
/*   Can only be used in the parameter list to lexan_get_token() if             */
/*   LEXAN_token_OPEN_COMMENT is also used.                                     */
/*   Regex: "\\* /"  (for C-style comments).                                    */
/*                   (WITHOUT THE SPACE between * and /)                        */
/*                                                                              */

typedef enum {
  LEXAN_token_single_str,
  LEXAN_token_double_str,
  LEXAN_token_iden,
  LEXAN_token_resWord,
  LEXAN_token_longInt,
  LEXAN_token_double,
  LEXAN_token_delim,
  LEXAN_token_EOF,
  LEXAN_token_NULL,
  LEXAN_token_LINE_COMMENT,
  LEXAN_token_OPEN_COMMENT,
  LEXAN_token_CLOSE_COMMENT
}
  LEXAN_t_tokenType;

extern char
  LEXAN_tokenTypeNames [12][26];

/*                             */
/* Case sensitivity of scanner */
/*                             */

typedef enum {
  LEXAN_t_upperCase,
  LEXAN_t_lowerCase,
  LEXAN_t_keepCase
}
  LEXAN_t_tokenCase;

/*                                                       */
/* Values associated with a token, depending on its type */
/*                                                       */

typedef struct {
  char    *tokenStr;
  long int longIntVal;
  double   doubleVal;
  char     delimChar;
}
  LEXAN_t_tokenVal;

/*                                         */
/* Location of the token in the input file */
/*                                         */

typedef struct {
  int lineNumber;
  int columnNumber;
}
  LEXAN_t_tokenLocation;

/*                                                               */
/* Formal specification of tokens                                */
/*                                                               */
/* type:                                                         */
/*   The type of the token                                       */
/* regex:                                                        */
/*   Regular expression defining the token                       */
/* firstGroup:                                                   */
/* lastGroup:                                                    */
/*   Limits the token to this portion of the regular expression. */
/*   Allows matches longer than actual tokens.                   */
/*   Useful for providing a context for tokens.                  */
/*   Only the input text leading up to the token is consumed;    */
/*   the part of the match coming after the token is preserved.  */
/*   These fields are ignored if "type" is one of the three      */
/*   comment tokens, as described above.                         */
/*                                                               */

typedef struct {
  LEXAN_t_tokenType  type;
  char              *regex;
  int                firstGroup;
  int                lastGroup;
}
  LEXAN_t_tokenSpecs;

/*                                  */
/* Id code of lexical analyser jobs */
/*                                  */

typedef int
  LEXAN_t_lexJobId;

/*
*---------------------------------------------------------------------
*   Procedures defined in "lexan.c"
*---------------------------------------------------------------------
*/

extern LEXAN_t_lexJobId LEXAN_start_job (
  char               *fileName,
  reg_syntax_t        regexSyntax,
  LEXAN_t_tokenCase   tokenCase,
  int                 totTokenSpecs,
  LEXAN_t_tokenSpecs *tokenSpecs );

extern bool LEXAN_get_token (
  LEXAN_t_lexJobId   jobId,
  LEXAN_t_tokenType *p_token,
  LEXAN_t_tokenVal  *p_tokenVal );

extern bool LEXAN_get_subtoken (
  LEXAN_t_lexJobId  jobId,
  LEXAN_t_tokenType subToken,
  int               subGroupStart,
  int               subGroupEnd,
  LEXAN_t_tokenVal *p_subTokenVal );

extern LEXAN_t_tokenLocation  LEXAN_get_token_location (LEXAN_t_lexJobId jobId);
extern char                  *LEXAN_get_token_context  (LEXAN_t_lexJobId jobId);
extern char                  *LEXAN_get_token_regex    (LEXAN_t_lexJobId jobId);
extern char                  *LEXAN_get_error_descr    (LEXAN_t_lexJobId jobId);
extern int                    LEXAN_get_error_count    (LEXAN_t_lexJobId jobId);
extern bool                   LEXAN_terminate_job      (LEXAN_t_lexJobId jobId);
extern bool                   LEXAN_terminate_all_jobs (void);

/*
*---------------------------------------------------------------------
*   Example
*---------------------------------------------------------------------
*
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <ctype.h>

    #include "error.h"
    #include "lexan.h"

    #define REGEX_RES         "(Begin|End)([[:space:]]|$)"
    #define REGEX_COMM        "(#|//)"
    #define REGEX_OPEN_COMM   "<<"
    #define REGEX_CLOSE_COMM  ">>"

    #define totTokenSpecs  7

    LEXAN_t_tokenSpecs
      tokenSpecs [totTokenSpecs] = {
        {LEXAN_token_longInt,       LEXAN_REGEX_INT,   1, 1},
        {LEXAN_token_double,        LEXAN_REGEX_REAL,  1, 1},
        {LEXAN_token_iden,          LEXAN_REGEX_IDEN,  1, 1},
        {LEXAN_token_resWord,       REGEX_RES,         1, 1},
        {LEXAN_token_LINE_COMMENT,  REGEX_COMM,        0, 0},
        {LEXAN_token_OPEN_COMMENT,  REGEX_OPEN_COMM,   0, 0},
        {LEXAN_token_CLOSE_COMMENT, REGEX_CLOSE_COMM,  0, 0}
      };


    void main (void)
    {
     char
       *fileName = "sample.spc";
     bool
       tokenOK;
     LEXAN_t_lexJobId
       jobId;
     LEXAN_t_tokenType
       tokenType;
     LEXAN_t_tokenVal
       tokenVal;

     jobId = LEXAN_start_job (
       (char *)               fileName,
       (reg_syntax_t)         LEXAN_REGEX_SYNTAX,
       (LEXAN_t_tokenCase)    LEXAN_t_keepCase,
       (int)                  totTokenSpecs,
       (LEXAN_t_tokenSpecs *) tokenSpecs );

     tokenOK = LEXAN_get_token (
      (LEXAN_t_lexJobId)    jobId,
      (LEXAN_t_tokenType *) &tokenType,
      (LEXAN_t_tokenVal *)  &tokenVal );

     while (token != LEXAN_token_EOF) {
       switch (tokenType) {
         case (LEXAN_token_longInt) :
           printf ("<Long int> %ld \n", tokenVal.longIntVal);
           break;
         case (LEXAN_token_double) :
           printf ("<Double> %f \n", tokenVal.doubleVal);
           break;
         case (LEXAN_token_iden) :
           printf ("<Iden> %s \n", tokenVal.tokenStr);
           break;
         case (LEXAN_token_resWord) :
           printf ("<ResWord> %s \n", tokenVal.tokenStr);
           break;
         default:
           printf ("Error: %s \n", LEXAN_get_error_descr (jobId));
           break;
       }
       tokenOK = LEXAN_get_token (
        (LEXAN_t_lexJobId)     jobId,
        (LEXAN_t_tokenType *) &tokenType,
        (LEXAN_t_tokenVal *)  &tokenVal );
     }
     if (! tokenOK)
       printf ("%s \n", LEXAN_get_error_descr (jobId));

     LEXAN_terminate_job (jobId);
    }
*
*---------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------
*   EXTRACTED FROM  /usr/include/rx.h
*---------------------------------------------------------------------

   The following bits are used to determine the regexp syntax we
   recognize.  The set/not-set meanings are chosen so that Emacs syntax
   remains the value 0.  The bits are given in alphabetical order, and
   the definitions shifted by one from the previous bit; thus, when we
   add or remove a bit, only one other definition need change.

   If this bit is not set, then \ inside a bracket expression is literal.
   If set, then such a \ quotes the following character.

#define RE_BACKSLASH_ESCAPE_IN_LISTS (1)

   If this bit is not set, then + and ? are operators, and \+ and \? are
     literals.
   If set, then \+ and \? are operators and + and ? are literals.

#define RE_BK_PLUS_QM (RE_BACKSLASH_ESCAPE_IN_LISTS << 1)

   If this bit is set, then character classes are supported.  They are:
     [:alpha:], [:upper:], [:lower:],  [:digit:], [:alnum:], [:xdigit:],
     [:space:], [:print:], [:punct:], [:graph:], and [:cntrl:].
   If not set, then character classes are not supported.

#define RE_CHAR_CLASSES (RE_BK_PLUS_QM << 1)

   If this bit is set, then ^ and $ are always anchors (outside bracket
     expressions, of course).
   If this bit is not set, then it depends:
        ^  is an anchor if it is at the beginning of a regular
           expression or after an open-group or an alternation operator;
        $  is an anchor if it is at the end of a regular expression, or
           before a close-group or an alternation operator.

   This bit could be (re)combined with RE_CONTEXT_INDEP_OPS, because
   POSIX draft 11.2 says that * etc. in leading positions is undefined.
   We already implemented a previous draft which made those constructs
   invalid, though, so we haven't changed the code back.

#define RE_CONTEXT_INDEP_ANCHORS (RE_CHAR_CLASSES << 1)

   If this bit is set, then special characters are always special
     regardless of where they are in the pattern.
   If this bit is not set, then special characters are special only in
     some contexts; otherwise they are ordinary.  Specifically,
     * + ? and intervals are only special when not after the beginning,
     open-group, or alternation operator.

#define RE_CONTEXT_INDEP_OPS (RE_CONTEXT_INDEP_ANCHORS << 1)

   If this bit is set, then *, +, ?, and { cannot be first in an re or
     immediately after an alternation or begin-group operator.

#define RE_CONTEXT_INVALID_OPS (RE_CONTEXT_INDEP_OPS << 1)

   If this bit is set, then . matches newline.
   If not set, then it doesn't.

#define RE_DOT_NEWLINE (RE_CONTEXT_INVALID_OPS << 1)

   If this bit is set, then . doesn't match NUL.
   If not set, then it does.

#define RE_DOT_NOT_NULL (RE_DOT_NEWLINE << 1)

   If this bit is set, nonmatching lists [^...] do not match newline.
   If not set, they do.

#define RE_HAT_LISTS_NOT_NEWLINE (RE_DOT_NOT_NULL << 1)

   If this bit is set, either \{...\} or {...} defines an
     interval, depending on RE_NO_BK_BRACES.
   If not set, \{, \}, {, and } are literals.

#define RE_INTERVALS (RE_HAT_LISTS_NOT_NEWLINE << 1)

   If this bit is set, +, ? and | aren't recognized as operators.
   If not set, they are.

#define RE_LIMITED_OPS (RE_INTERVALS << 1)

   If this bit is set, newline is an alternation operator.
   If not set, newline is literal.

#define RE_NEWLINE_ALT (RE_LIMITED_OPS << 1)

   If this bit is set, then `{...}' defines an interval, and \{ and \}
     are literals.
  If not set, then `\{...\}' defines an interval.

#define RE_NO_BK_BRACES (RE_NEWLINE_ALT << 1)

   If this bit is set, (...) defines a group, and \( and \) are literals.
   If not set, \(...\) defines a group, and ( and ) are literals.

#define RE_NO_BK_PARENS (RE_NO_BK_BRACES << 1)

   If this bit is set, then \<digit> matches <digit>.
   If not set, then \<digit> is a back-reference.

#define RE_NO_BK_REFS (RE_NO_BK_PARENS << 1)

   If this bit is set, then | is an alternation operator, and \| is literal.
   If not set, then \| is an alternation operator, and | is literal.

#define RE_NO_BK_VBAR (RE_NO_BK_REFS << 1)

   If this bit is set, then an ending range point collating higher
     than the starting range point, as in [z-a], is invalid.
   If not set, then when ending range point collates higher than the
     starting range point, the range is ignored.

#define RE_NO_EMPTY_RANGES (RE_NO_BK_VBAR << 1)

   If this bit is set, then an unmatched ) is ordinary.
   If not set, then an unmatched ) is invalid.

#define RE_UNMATCHED_RIGHT_PAREN_ORD (RE_NO_EMPTY_RANGES << 1)

   If this bit is set, do not process the GNU regex operators.
   IF not set, then the GNU regex operators are recognized.

#define RE_NO_GNU_OPS (RE_UNMATCHED_RIGHT_PAREN_ORD << 1)

   This global variable defines the particular regexp syntax to use (for
   some interfaces).  When a regexp is compiled, the syntax used is
   stored in the pattern buffer, so changing this does not affect
   already-compiled regexps.

extern reg_syntax_t re_syntax_options;

   Define combinations of the above bits for the standard possibilities.
   (The [[[ comments delimit what gets put into the Texinfo file, so
   don't delete them!)

   [[[begin syntaxes]]]

#define RE_SYNTAX_EMACS 0

#define RE_SYNTAX_AWK                                                   \
  (RE_BACKSLASH_ESCAPE_IN_LISTS | RE_DOT_NOT_NULL                       \
   | RE_NO_BK_PARENS            | RE_NO_BK_REFS                         \
   | RE_NO_BK_VBAR              | RE_NO_EMPTY_RANGES                    \
   | RE_UNMATCHED_RIGHT_PAREN_ORD)

#define RE_SYNTAX_POSIX_AWK                                             \
  (RE_SYNTAX_POSIX_EXTENDED | RE_BACKSLASH_ESCAPE_IN_LISTS)

#define RE_SYNTAX_GREP                                                  \
  (RE_BK_PLUS_QM              | RE_CHAR_CLASSES                         \
   | RE_HAT_LISTS_NOT_NEWLINE | RE_INTERVALS                            \
   | RE_NEWLINE_ALT)

#define RE_SYNTAX_EGREP                                                 \
  (RE_CHAR_CLASSES        | RE_CONTEXT_INDEP_ANCHORS                    \
   | RE_CONTEXT_INDEP_OPS | RE_HAT_LISTS_NOT_NEWLINE                    \
   | RE_NEWLINE_ALT       | RE_NO_BK_PARENS                             \
   | RE_NO_BK_VBAR)

#define RE_SYNTAX_POSIX_EGREP                                           \
  (RE_SYNTAX_EGREP | RE_INTERVALS | RE_NO_BK_BRACES)

#define RE_SYNTAX_SED RE_SYNTAX_POSIX_BASIC

   Syntax bits common to both basic and extended POSIX regex syntax.

#define _RE_SYNTAX_POSIX_COMMON                                         \
  (RE_CHAR_CLASSES | RE_DOT_NEWLINE      | RE_DOT_NOT_NULL              \
   | RE_INTERVALS  | RE_NO_EMPTY_RANGES)

#define RE_SYNTAX_POSIX_BASIC                                           \
  (_RE_SYNTAX_POSIX_COMMON | RE_BK_PLUS_QM)

   Differs from ..._POSIX_BASIC only in that RE_BK_PLUS_QM becomes
   RE_LIMITED_OPS, i.e., \? \+ \| are not recognized.  Actually, this
   isn't minimal, since other operators, such as \`, aren't disabled.

#define RE_SYNTAX_POSIX_MINIMAL_BASIC                                   \
  (_RE_SYNTAX_POSIX_COMMON | RE_LIMITED_OPS)

#define RE_SYNTAX_POSIX_EXTENDED                                        \
  (_RE_SYNTAX_POSIX_COMMON | RE_CONTEXT_INDEP_ANCHORS                   \
   | RE_CONTEXT_INDEP_OPS  | RE_NO_BK_BRACES                            \
   | RE_NO_BK_PARENS       | RE_NO_BK_VBAR                              \
   | RE_UNMATCHED_RIGHT_PAREN_ORD)

   Differs from ..._POSIX_EXTENDED in that RE_CONTEXT_INVALID_OPS
   replaces RE_CONTEXT_INDEP_OPS and RE_NO_BK_REFS is added.

#define RE_SYNTAX_POSIX_MINIMAL_EXTENDED                                \
  (_RE_SYNTAX_POSIX_COMMON  | RE_CONTEXT_INDEP_ANCHORS                  \
   | RE_CONTEXT_INVALID_OPS | RE_NO_BK_BRACES                           \
   | RE_NO_BK_PARENS        | RE_NO_BK_REFS                             \
   | RE_NO_BK_VBAR          | RE_UNMATCHED_RIGHT_PAREN_ORD)
   [[[end syntaxes]]]

   Maximum number of duplicates an interval can allow.  Some systems
   (erroneously) define this in other header files, but we want our
   value, so remove any previous define.

#ifdef RE_DUP_MAX
#undef RE_DUP_MAX
#endif
#define RE_DUP_MAX ((1 << 15) - 1)

*---------------------------------------------------------------------
*/

#endif /* ifndef _LEXAN_DOT_H_ */
