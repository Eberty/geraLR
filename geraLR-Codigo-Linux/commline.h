/*
*-----------------------------------------------------------------------
*
*   File         : commline.h
*   Created      : 1995-03-05
*   Last Modified: 2012-05-27
*
*   Management of command line arguments.
*
*-----------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _COMMAND_LINE_DOT_H_
#define _COMMAND_LINE_DOT_H_

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <limits.h>

/*
*---------------------------------------------------------------------
*
*   INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

/*                  */
/* Useful constants */
/*                  */

#ifndef bool
# define bool    int
# define false   0
# define true    1
#endif

#define COMMLINE_OPT_FREE_USE  INT_MAX

/*                                            */
/* Types of arguments of command line options */
/*                                            */

typedef enum {
  COMMLINE_opt_arg_none,
  COMMLINE_opt_arg_long_int,
  COMMLINE_opt_arg_ulong_int,
  COMMLINE_opt_arg_double,
  COMMLINE_opt_arg_char,
  COMMLINE_opt_arg_string
}
  COMMLINE_t_optArgType;

/*
*---------------------------------------------------------------------
*   Procedures defined in "commLine.c"
*---------------------------------------------------------------------
*/

extern void  COMMLINE_free_commLine_data     (void);
extern bool  COMMLINE_argPos2argVal          (int arg_pos, void *arg_val);
extern void  COMMLINE_display_usage          (void);
extern char *COMMLINE_get_commLine_error     (void);
extern char *COMMLINE_get_program_short_name (char *full_name);
extern bool  COMMLINE_optId2optUses          (int opt_ID,  int *uses);
extern bool  COMMLINE_optPos2optUse          (int opt_pos, int *opt_ID,  int  *opt_use);
extern bool  COMMLINE_optUse2optArg          (int opt_ID,  int  opt_use, void *opt_arg);
extern bool  COMMLINE_optUse2optPos          (int opt_ID,  int  opt_use, int  *opt_pos);
extern bool  COMMLINE_parse_commLine         (int argc, char *argv[], int totOpts,...);
extern int   COMMLINE_total_options          (void);
extern int   COMMLINE_total_non_opt_args     (void);

extern void COMMLINE_validate_boolean_option (
 bool   *p_var,
 int     optIndex,
 bool    defaultValue,
 char   *varName,
 char   *procName,
 char   *progName );

extern void COMMLINE_validate_int_option (
 int  *p_var,
 int   optIndex,
 int   defaultValue,
 int   minValue,
 int   maxValue,
 char *varName,
 char *procName,
 char *progName );

extern void COMMLINE_validate_short_int_option (
 short int *p_var,
 int        optIndex,
 short int  defaultValue,
 short int  minValue,
 short int  maxValue,
 char      *varName,
 char      *procName,
 char      *progName );

extern void COMMLINE_validate_double_option (
 double *p_var,
 int     optIndex,
 double  defaultValue,
 double  minValue,
 double  maxValue,
 char   *varName,
 char   *procName,
 char   *progName );

extern void COMMLINE_validate_string_option (
 char  **p_var,
 int     optIndex,
 char   *defaultValue,
 int     minLength,
 int     maxLength,
 char   *varName,
 char   *procName,
 char   *progName );

extern void COMMLINE_validate_oneOf_string_options (
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
*   EXTRACTED FROM THE GNU INFO FILE ON PROGRAM ARGUMENTS:
*   [File: libc.info, Node: Argument Syntax, Up: Program Arguments]
*---------------------------------------------------------------------
*
*   Program Argument Syntax Conventions
*   -----------------------------------
*
*   POSIX recommends these conventions for command line arguments.
*   `getopt' makes it easy to implement them.
*
*   * Arguments are options if they begin with a hyphen delimiter (`-').
*   * Multiple options may follow a hyphen delimiter in a single token if
*     the options do not take arguments.  Thus, `-abc' is equivalent to
*     `-a -b -c'.
*   * Option names are single alphanumeric characters (as for `isalnum').
*   * Certain options require an argument.  For example, the `-o' command
*     of the `ld' command requires an argument--an output file name.
*   * An option and its argument may or may not appear as separate
*     tokens.  (In other words, the whitespace separating them is
*     optional.)  Thus, `-o foo' and `-ofoo' are equivalent.
*   * Options typically precede other non-option arguments.
*     The implementation of `getopt' in the GNU C library normally makes
*     it appear as if all the option arguments were specified before all
*     the non-option arguments for the purposes of parsing, even if the
*     user of your program intermixed option and non-option arguments.
*     It does this by reordering the elements of the ARGV array.  This
*     behavior is nonstandard; if you want to suppress it, define the
*     `_POSIX_OPTION_ORDER' environment variable.
*   * The argument `--' terminates all options; any following arguments
*     are treated as non-option arguments, even if they begin with a
*     hyphen.
*   * A token consisting of a single hyphen character is interpreted as
*     an ordinary non-option argument.  By convention, it is used to
*     specify input from or output to the standard input and output
*     streams.
*   * Options may be supplied in any order, or appear multiple times.
*     The interpretation is left up to the particular application
*     program.
*
*   GNU adds "long options" to these conventions.  Long options consist
*   of `--' followed by a name made of alphanumeric characters and dashes.
*   Option names are typically one to three words long, with hyphens to
*   separate words.  Users can abbreviate the option names as long as the
*   abbreviations are unique.
*      To specify an argument for a long option, write `--NAME=VALUE'.
*   This syntax enables a long option to accept an argument that is itself
*   optional.
*
*---------------------------------------------------------------------
*   PROCEDURE: COMMLINE_parse_commLine()
*---------------------------------------------------------------------
*
*   Following the POSIX/GNU convention, this procedure assumes that the
*   command line contains 0 or more options, with or without arguments,
*   followed by 0 or more non-option arguments.
*   Here we make a distinction between *FORMAL* and *ACTUAL* options:
*
*   - FORMAL options are those which the program should be able to
*     take via the command line. Their specification details must
*     be passed to this procedure as variable parameters.
*
*   - ACTUAL options are those provided to the program at run time
*     by the user via the command line. They are contained in the
*     "(char *) ARGV[]" variable, as received by "main()".
*
*   The specification details of a formal option include:
*
*   - identifier:
*     an integer used to uniquely identify the option; used in the
*     specification of mutually exclusive options (see below).
*
*   - short name:
*     a single character. Each option must have a unique character
*     for its short name.
*
*   - long name:
*     a character string. Each option must have a unique string
*     for its long name (or, alternatively, an empty string).
*
*   - argument type:
*     one of the following values
*       COMMLINE_opt_arg_none ....................... no argument
*       COMMLINE_opt_arg_long_int ................... long integer
*       COMMLINE_opt_arg_ulong_int .................. unsigned long integer
*       COMMLINE_opt_arg_double ..................... double
*       COMMLINE_opt_arg_char ....................... single character
*       COMMLINE_opt_arg_string ..................... character string
*
*   - minimum uses:
*     minimum number of actual uses in the command line;
*     0 indicates that the option may be omitted.
*
*   - maximum uses:
*     maximum number of actual uses in the command line.
*     The macro COMMLINE_OPT_FREE_USE indicates that there is no
*     limit to the number of times the option may be used.
*
*   - total exclusions:
*     number of options that are mutually exclusive with this one.
*     If any two options are declared as mutually exclusive, they
*     cannot be both used in the command line. An option may be
*     declared as mutually exclusive with any number of other ones.
*
*   - exclusion list:
*     list of identifiers indicating the options with which this one
*     is mutually exclusive.
*
*---------------------------------------------------------------------
*   PARAMETERS:
*---------------------------------------------------------------------
*
*   This procedure takes some fixed and some variable parameters,
*   which are described below.
*   The ones marked [F] are fixed; all the others are variable.
*
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
*       COMMLINE_t_optArgType ............... option argument type
*       int  ....................... minimum # of uses in command line
*       int  ....................... maximum # of uses in command line
*       int  ....................... totExcls (size of option excl list)
*
*         if totExcls > 0, then for each option in the exclusion list:
*
*         int .................... option identifier
*
*     int .............[F].......... nonOptArgs
*
*       if nonOptArgs > 0, then for each argument:
*
*       COMMLINE_t_optArgType ............... option argument type
*
*---------------------------------------------------------------------
*   USAGE:
*---------------------------------------------------------------------
*
*   commLineOK = COMMLINE_parse_commLine (
*      argc,
*      argv,
*      7,
*        1, 'H', "help",      COMMLINE_opt_arg_none,      0, COMMLINE_OPT_FREE_USE, 0,
*        2, 'U', "usage",     COMMLINE_opt_arg_none,      0, COMMLINE_OPT_FREE_USE, 0,
*        3, 'o', "output",    COMMLINE_opt_arg_string,    0, 1,            0,
*        4, 's', "samples",   COMMLINE_opt_arg_ulong_int, 1, 1,            0,
*        5, 'u', "uniform",   COMMLINE_opt_arg_none,      0, 1,            2, 6, 7,
*        6, 'n', "normal",    COMMLINE_opt_arg_none,      0, 1,            0,
*        7, 'd', "deviation", COMMLINE_opt_arg_double,    0, 1,            0,
*      2,
*        COMMLINE_opt_arg_string,
*        COMMLINE_opt_arg_ulong_int );
*
*---------------------------------------------------------------------
*/

#endif /* ifndef _COMMAND_LINE_DOT_H_ */
