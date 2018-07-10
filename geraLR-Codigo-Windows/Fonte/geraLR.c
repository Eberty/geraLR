/*
*-----------------------------------------------------------------------
*
*   File         : geraLR.c
*   Created      : 2006-07-11
*   Last Modified: 2018-05-21
*
*   DESCRIPTION:
*
*   This program reads a text file containing a context free grammar
*   and generates a LR(0) or sLR(1) parse table, along with the
*   corresponding collection of canonical LR(0) items, DFA states
*   and state transitions, and FIRST and FOLLOW sets.
*
*   USAGE:
*   geraLR -h for help
*
*-----------------------------------------------------------------------
*/

/*
*-----------------------------------------------------------------------
* INCLUDE FILES
*-----------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "common.h"
#include "datastructs.h"
#include "error.h"
#include "mystrings.h"
#include "report.h"
#include "lexan.h"
#include "commline.h"
#include "grammar.h"
#include "diagrams.h"
#include "geraLR.h"

/*
*-----------------------------------------------------------------------
* INTERFACE (visible from other modules)
*-----------------------------------------------------------------------
*/

/*                        */
/* Program and file names */
/*                        */

char
  *progName                                = NULL,
  grammarFileName         [FILE_NAME_SIZE] = "",
  symbolsFileName         [FILE_NAME_SIZE] = "",
  nfaTextFileName         [FILE_NAME_SIZE] = "",
  nfaDotFileName          [FILE_NAME_SIZE] = "",
  nfaSvgFileName          [FILE_NAME_SIZE] = "",
  nfaLdaFileName          [FILE_NAME_SIZE] = "",
  dfaTextFileName         [FILE_NAME_SIZE] = "",
  dfaDotFileName          [FILE_NAME_SIZE] = "",
  dfaSvgFileName          [FILE_NAME_SIZE] = "",
  dfaLdaFileName          [FILE_NAME_SIZE] = "",
  configFileName          [FILE_NAME_SIZE] = "",
  setsFileName            [FILE_NAME_SIZE] = "",
  lr0binaryTableFileName  [FILE_NAME_SIZE] = "",
  slr1binaryTableFileName [FILE_NAME_SIZE] = "",
  textTablesFileName      [FILE_NAME_SIZE] = "",
  answerSheetFileName     [FILE_NAME_SIZE] = "",
  oneOutputFileName       [FILE_NAME_SIZE] = "";

/*              */
/* File handles */
/*              */

FILE
  *grammarFilePt         = NULL,
  *symbolsFilePt         = NULL,
  *nfaTextFilePt         = NULL,
  *nfaDotFilePt          = NULL,
  *nfaSvgFilePt          = NULL,
  *nfaLdaFilePt          = NULL,
  *dfaTextFilePt         = NULL,
  *dfaDotFilePt          = NULL,
  *dfaSvgFilePt          = NULL,
  *dfaLdaFilePt          = NULL,
  *setsFilePt            = NULL,
  *lr0binaryTableFilePt  = NULL,
  *slr1binaryTableFilePt = NULL,
  *textTablesFilePt      = NULL,
  *answerSheetFilePt     = NULL,
  *outputFilePt          = NULL;

bool
  b_one_output_file = false,
  b_output_started  = false;

/*
*---------------------------------------------------------------------
* Function prototypes
*---------------------------------------------------------------------
*/

void print_output_header (FILE *filePt, char *fileName, int argc, char *argv[]);
void print_grammar_rules (FILE *filePt);

/* To prevent "implicit declaration" warnings */

int snprintf (char *str, size_t size, const char *format, ...);

/*
*-----------------------------------------------------------------------
* IMPLEMENTATION (invisible from other modules)
*-----------------------------------------------------------------------
*/

/*                                     */
/* Fine-tuning of parse table printout */
/*                                     */

/* Column width and print format for state numbers */

#define PARSE_TABLE_STATE_WIDTH            7
#define PARSE_TABLE_STATE_PLAIN_FORMAT     "%3d"
#define PARSE_TABLE_STATE_CONFLICT_FORMAT  "> %3d <"
#define PARSE_TABLE_STATE_BLANK_FORMAT     " "

/* Print format for symbol codes and strings */

#define PARSE_TABLE_SYMBOL_CODE_FORMAT     "%d"
#define PARSE_TABLE_SYMBOL_STRING_FORMAT   "%s"

/*-------------------------------------------------------------------*/
/* Default column width for symbol strings or codes                  */
/* Can be set with command line option -w                            */
/*-------------------------------------------------------------------*/
/* Width > 0                                                         */
/*   the actual column width; contents will be trimmed and justified */
/* Width = 0                                                         */
/*   auto column width (set to longest symbol print label)           */
/*   with no extra blanks on either margin                           */
/* Width < 0                                                         */
/*   auto column width (set to longest symbol print label)           */
/*   plus ABS(Width) blanks inside each margin                       */
/*-------------------------------------------------------------------*/

#define DEFAULT_PARSE_TABLE_SYMBOL_WIDTH   -1

/* Print formats for the various parse action entries */

#define PARSE_TABLE_STRING_SHIFT    "s%d"
#define PARSE_TABLE_STRING_REDUCE   "r%d"
#define PARSE_TABLE_STRING_GOTO     "%d"
#define PARSE_TABLE_STRING_ACCEPT   "ACC"
#define PARSE_TABLE_STRING_ERROR    " "


static bool
  b_stripoff_quotes         = false,
  b_print_symbols           = false,
  b_print_nfa_text          = false,
  b_print_nfa_svg           = false,
  b_print_dfa_text          = false,
  b_print_dfa_svg           = false,
  b_automatic_animation     = false, 
  b_set_cfg_file            = false,
  b_print_sets              = false,
  b_print_answer_sheet      = false,
  b_print_text_LR0_table    = false,
  b_print_text_sLR1_table   = false,
  b_write_binary_LR0_table  = false,
  b_write_binary_sLR1_table = false,
  b_print_symbol_codes      = false;

static int
  parse_table_symbol_width;

/*
*---------------------------------------------------------------------
* Function prototypes
*---------------------------------------------------------------------
*/

int       main                                 (int argc, char *argv[]);
void      process_commLine                     (int argc, char *argv[]);
void      print_grammar_data                   (int argc, char *argv[]);
void      print_grammar_symbols                (int argc, char *argv[]);
void      print_sets                           (int argc, char *argv[]);
void      print_answer_sheet                   (int argc, char *argv[]);
void      print_text_parse_table_report_header (int argc, char *argv[]);
void      print_text_parse_table               (t_parse_table_type parse_table_type);
void      write_parse_table_binary_file        (t_parse_table_type parse_table_type);
short int stateCode2maxParseActions            (t_parse_table_type parse_table_type, t_stateCode stateCode);
void write_var_to_parse_table_bin_file  (
  t_parse_table_type  parse_table_type,
  const void         *pVar,
  size_t              varSize,
  const char         *varName );
void write_string_var_to_parse_table_bin_file (
  t_parse_table_type  parse_table_type,
  const char         *string,
  size_t              stringSize,
  const char         *varName );
void write_parse_action_to_parse_table_bin_file (
  t_parse_table_type  parse_table_type,
  t_stateCode         stateCode,
  t_symbolCode        symbolCode,
  t_parseAction       parseAction );

/*
*---------------------------------------------------------------------
* Process options and arguments in the command line
*---------------------------------------------------------------------
*/

typedef enum {
  commLineOpt_help = 0,
  commLineOpt_usage,
  commLineOpt_stripquotes,
  commLineOpt_symbols,
  commLineOpt_nfatxt,
  commLineOpt_nfasvg,
  commLineOpt_dfatxt,
  commLineOpt_dfasvg,
  commLineOpt_autoanim,
  commLineOpt_config,
  commLineOpt_sets,
  commLineOpt_answersheet,
  commLineOpt_oneoutput,
  commLineOpt_lrtxt,
  commLineOpt_slrtxt,
  commLineOpt_lrbin,
  commLineOpt_slrbin,
  commLineOpt_codes,
  commLineOpt_symbolwidth
}
t_commLineOpts;

int
  commLine_totOptions = (int) commLineOpt_symbolwidth + 1;

void process_commLine (int argc, char *argv[])
{
 bool
   commLineOK;
 int
   optUses;
 long int
   argLongInt;
 char
   *argStr = NULL;

 /* If there was an error in the command line */
 /* then display program usage info and abort */

 progName = COMMLINE_get_program_short_name (argv[0]);
 commLineOK = COMMLINE_parse_commLine (argc, argv,
   commLine_totOptions,
      commLineOpt_help,        'h', "help",        COMMLINE_opt_arg_none,     0, COMMLINE_OPT_FREE_USE, 0,
      commLineOpt_usage,       'u', "usage",       COMMLINE_opt_arg_none,     0, COMMLINE_OPT_FREE_USE, 0,
      commLineOpt_stripquotes, 'x', "stripquotes", COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_symbols,     'y', "symbols",     COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_nfatxt,      'n', "nfatxt",      COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_nfasvg,      'N', "nfasvg",      COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_dfatxt,      'd', "dfatxt",      COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_dfasvg,      'D', "dfasvg",      COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_autoanim,    'A', "autoanim",    COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_config,      'C', "configfile",  COMMLINE_opt_arg_string,   0, 1,            0,
      commLineOpt_sets,        'f', "sets",        COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_answersheet, 'a', "answersheet", COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_oneoutput,   'o', "oneoutput",   COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_lrtxt,       'l', "lrtxt",       COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_slrtxt,      's', "slrtxt",      COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_lrbin,       'L', "lrbin",       COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_slrbin,      'S', "slrbin",      COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_codes,       'c', "codes",       COMMLINE_opt_arg_none,     0, 1,            0,
      commLineOpt_symbolwidth, 'w', "symbolwidth", COMMLINE_opt_arg_long_int, 0, 1,            0,
   1,
      COMMLINE_opt_arg_string );

 if (! commLineOK) {
   COMMLINE_display_usage();
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 }

 /* If the user asked for help then provide it and exit */

 if (! COMMLINE_optId2optUses (commLineOpt_help, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (optUses > 0) {
   printf("\n");
   printf("This program takes as input a context free grammar and\n");
   printf("generates transition tables to drive different LR parsers.\n");
   printf("Usage:\n");
   printf("\n");
   printf("%s grammar_file [options]\n", progName);
   printf("\n");
   printf("+---------------------------------------------------------------------------+\n");
   printf("| Option     Purpose                                                Default |\n");
   printf("+---------------------------------------------------------------------------+\n");
   printf("| -h         Display this help message                                      |\n");
   printf("| -u         Display details on command line options                        |\n");
   printf("| -y         Print grammar symbols and their numeric codes               No |\n");
   printf("| -n         Print NFA states with LR(0) items in plain text format      No |\n");
   printf("| -N         Print NFA in DOT, convert to SVG format, and generate .lda  No |\n");
   printf("| -d         Print DFA states with LR(0) items in plain text format      No |\n");
   printf("| -D         Print DFA in DOT, convert to SVG format, and generate .lda  No |\n");
   printf("| -A         Generate LiDAS files for SVG animation move by itself       No |\n");
   printf("|              - Standard animation is by keyboard                          |\n");
   printf("| -C <file>  Set configuration file to SVG and animation attributes      No |\n");
   printf("| -f         Print FIRST and FOLLOW sets                                 No |\n");
   printf("| -a         Print exam paper answer sheet                               No |\n");
   printf("| -l         Generate LR(0) parse tables in text format                  No |\n");
   printf("| -s         Generate sLR(1) parse tables in text format                 No |\n");
   printf("| -L         Generate LR(0) parse tables in binary format                No |\n");
   printf("| -S         Generate sLR(1) parse tables in binary format               No |\n");
   printf("| -o         Send all output to a single file                            No |\n");
   printf("| -x         Strip off quotes from terminal symbol strings               No |\n");
   printf("| -c         Print symbol codes in parse tables, not strings             No |\n");
   printf("| -w N       Column width for symbols in parse tables                    -1 |\n");
   printf("|              N>0 : actual column width; text trimmed & left justified     |\n");
   printf("|              N=0 : auto column width, set to longest symbol string        |\n");
   printf("|                    and no extra blanks on either margin                   |\n");
   printf("|              N=-1: auto column width, set to longest symbol string        |\n");
   printf("|                    plus ABS(N) blanks inside each margin                  |\n");
   printf("+---------------------------------------------------------------------------+\n");
   printf(" \n");
   exit (EXIT_SUCCESS);
 }

 /* If the user asked for program usage info then provide it and exit */

 if (! COMMLINE_optId2optUses (commLineOpt_usage, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (optUses > 0) {
   COMMLINE_display_usage();
   exit (EXIT_SUCCESS);
 }

 /* Does the user want quotes stripped off symbol strings? */

 if (! COMMLINE_optId2optUses (commLineOpt_stripquotes, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_stripoff_quotes = (optUses > 0);

 /* Has the user asked for grammar symbols and their numeric codes? */

 if (! COMMLINE_optId2optUses (commLineOpt_symbols, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_symbols = (optUses > 0);

 /* Has the user asked for NFA states with LR(0) items in plain text format? */

 if (! COMMLINE_optId2optUses (commLineOpt_nfatxt, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_nfa_text = (optUses > 0);
 
 /* Has the user asked for NFA states with LR(0) items in SVG format? */

 if (! COMMLINE_optId2optUses (commLineOpt_nfasvg, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_nfa_svg = (optUses > 0);
 
 /* Has the user asked for DFA states with LR(0) items in plain text format? */

 if (! COMMLINE_optId2optUses (commLineOpt_dfatxt, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_dfa_text = (optUses > 0);

 /* Has the user asked for DFA states with LR(0) items in SVG format? */

 if (! COMMLINE_optId2optUses (commLineOpt_dfasvg, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_dfa_svg = (optUses > 0);
 
 /* Has the user asked for LiDAS files to animate the SVG automatically? */
 
 if (! COMMLINE_optId2optUses (commLineOpt_autoanim, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_automatic_animation = (optUses > 0);
 
 /* Has the user asked for set configuration file to SVG attributes? */

 if (! COMMLINE_optId2optUses (commLineOpt_config, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_set_cfg_file = (optUses > 0);
 if (optUses != 0) {
   if (! COMMLINE_optUse2optArg (commLineOpt_config, 1, &argStr))
     ERROR_short_fatal_error (COMMLINE_get_commLine_error());
   free (argStr);
   argStr = NULL;
 }
 
 /* Has the user asked for FIRST and FOLLOW sets? */

 if (! COMMLINE_optId2optUses (commLineOpt_sets, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_sets = (optUses > 0);

 /* Has the user asked for an answersheet printout? */

 if (! COMMLINE_optId2optUses (commLineOpt_answersheet, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_answer_sheet = (optUses > 0);

 /* Has the user asked for a single output file? */

 if (! COMMLINE_optId2optUses (commLineOpt_oneoutput, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_one_output_file = (optUses > 0);

 /* Has the user asked for generation of LR(0) parser table in text format? */

 if (! COMMLINE_optId2optUses (commLineOpt_lrtxt, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_text_LR0_table = (optUses > 0);

 /* Has the user asked for generation of SLR(1) parser table in text format? */

 if (! COMMLINE_optId2optUses (commLineOpt_slrtxt, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_text_sLR1_table = (optUses > 0);

 /* Has the user asked for generation of LR(0) parser table in binary format? */

 if (! COMMLINE_optId2optUses (commLineOpt_lrbin, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_write_binary_LR0_table = (optUses > 0);

 /* Has the user asked for generation of SLR(1) parser table in binary format? */

 if (! COMMLINE_optId2optUses (commLineOpt_slrbin, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_write_binary_sLR1_table = (optUses > 0);

 /* Print symbol codes in parse tables instead of symbol strings? */

 if (! COMMLINE_optId2optUses (commLineOpt_codes, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 b_print_symbol_codes = (optUses > 0);

 /* Column width for symbol codes or strings in parse tables */

 if (! COMMLINE_optId2optUses (commLineOpt_symbolwidth, &optUses))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 if (optUses == 0)
   parse_table_symbol_width = (int) DEFAULT_PARSE_TABLE_SYMBOL_WIDTH;
  else {
   if (! COMMLINE_optUse2optArg (commLineOpt_symbolwidth, 1, &argLongInt))
     ERROR_short_fatal_error (COMMLINE_get_commLine_error());
   parse_table_symbol_width = (int) argLongInt;
 }

 /* At least one type of output must be selected */

 if (! (b_print_symbols          ||
        b_print_nfa_text         ||
        b_print_nfa_svg          ||
        b_print_dfa_text         ||
        b_print_dfa_svg          ||
        b_print_sets             ||
        b_print_answer_sheet     ||
        b_print_text_sLR1_table  ||
        b_print_text_LR0_table   ||
        b_write_binary_LR0_table ||
        b_write_binary_sLR1_table  )) {
   printf ("\nNo valid output selected.\n\n");
   exit(0);
 }

 /* Check the input file name */
{
 if (! COMMLINE_argPos2argVal (1, &argStr))
   ERROR_short_fatal_error (COMMLINE_get_commLine_error());
 errno = 0;
 if ((strcpy (grammarFileName, argStr)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (grammarFileName, %s) failed", argStr);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
 free (argStr);
 argStr = NULL;
 errno = 0;
 if ((grammarFilePt = fopen (grammarFileName, "r")) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open file \"%s\" for reading", grammarFileName);
   ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
 }
}

 /* Build output file names and file handles */

 if (b_one_output_file) {
   errno = 0;
   if ((strcpy (oneOutputFileName, grammarFileName)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (oneOutputFileName ,\"%s\") failed", grammarFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((strcat(oneOutputFileName, FILE_EXTENSION_ONE_OUTPUT)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(oneOutputFileName ,\"%s\") failed", FILE_EXTENSION_ONE_OUTPUT);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((outputFilePt = fopen (oneOutputFileName, "w")) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", oneOutputFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
 }

 if (b_print_symbols) {
   if (b_one_output_file) {
     errno = 0;
     if ((strcpy (symbolsFileName, oneOutputFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (symbolsFileName ,\"%s\") failed", oneOutputFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     symbolsFilePt = outputFilePt;
   }
   else {
     errno = 0;
     if ((strcpy (symbolsFileName, grammarFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (symbolsFileName ,\"%s\") failed", grammarFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((strcat(symbolsFileName, FILE_EXTENSION_SYMBOLS)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(symbolsFileName ,\"%s\") failed", FILE_EXTENSION_SYMBOLS);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((symbolsFilePt = fopen (symbolsFileName, "w")) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", symbolsFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
   }
 }

 if (b_print_nfa_text) {
   if (b_one_output_file) {
     errno = 0;
     if ((strcpy (nfaTextFileName, oneOutputFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (nfaTextFileName ,\"%s\") failed", oneOutputFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     nfaTextFilePt = outputFilePt;
   }
   else {
     errno = 0;
     if ((strcpy (nfaTextFileName, grammarFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (nfaTextFileName ,\"%s\") failed", grammarFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((strcat(nfaTextFileName, FILE_EXTENSION_NFA_TEXT)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(nfaTextFileName ,\"%s\") failed", FILE_EXTENSION_NFA_TEXT);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((nfaTextFilePt = fopen (nfaTextFileName, "w")) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", nfaTextFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
   }
 }

 if (b_print_nfa_svg) {
   errno = 0;
   if ((strcpy (nfaDotFileName, grammarFileName)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (nfaDotFileName ,\"%s\") failed", grammarFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((strcat(nfaDotFileName, FILE_EXTENSION_NFA_DOT)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(nfaDotFileName ,\"%s\") failed", FILE_EXTENSION_NFA_DOT);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((nfaDotFilePt = fopen (nfaDotFileName, "w")) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", nfaDotFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
 }

 if (b_print_dfa_text) {
   if (b_one_output_file) {
     errno = 0;
     if ((strcpy (dfaTextFileName, oneOutputFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (dfaTextFileName ,\"%s\") failed", oneOutputFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     dfaTextFilePt = outputFilePt;
   }
   else {
     errno = 0;
     if ((strcpy (dfaTextFileName, grammarFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (dfaTextFileName ,\"%s\") failed", grammarFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((strcat(dfaTextFileName, FILE_EXTENSION_DFA_TEXT)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(dfaTextFileName ,\"%s\") failed", FILE_EXTENSION_DFA_TEXT);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((dfaTextFilePt = fopen (dfaTextFileName, "w")) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", dfaTextFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
   }
 }

 if (b_print_dfa_svg) {
   errno = 0;
   if ((strcpy (dfaDotFileName, grammarFileName)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (dfaDotFileName ,\"%s\") failed", grammarFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((strcat(dfaDotFileName, FILE_EXTENSION_DFA_DOT)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(dfaDotFileName ,\"%s\") failed", FILE_EXTENSION_DFA_DOT);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((dfaDotFilePt = fopen (dfaDotFileName, "w")) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", dfaDotFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
 }

 if (b_set_cfg_file) {
   if (! COMMLINE_optUse2optArg (commLineOpt_config, 1, &argStr))
     ERROR_short_fatal_error (COMMLINE_get_commLine_error());
   errno = 0;
   if ((strcpy (configFileName, argStr)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (configFileName, %s) failed", argStr);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   free (argStr);
   argStr = NULL;
   /*if ((configFilePt = fopen (configFileName, "r")) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open file \"%s\" for reading", configFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   fclose(configFilePt);
   */
 }
 
 if (b_print_sets) {
   if (b_one_output_file) {
     errno = 0;
     if ((strcpy (setsFileName, oneOutputFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (setsFileName ,\"%s\") failed", oneOutputFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     setsFilePt = outputFilePt;
   }
   else {
     errno = 0;
     if ((strcpy (setsFileName, grammarFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (setsFileName ,\"%s\") failed", grammarFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((strcat(setsFileName, FILE_EXTENSION_SETS)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(setsFileName ,\"%s\") failed", FILE_EXTENSION_SETS);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((setsFilePt = fopen (setsFileName, "w")) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", setsFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
   }
 }

 if (b_print_answer_sheet) {
   if (b_one_output_file) {
     errno = 0;
     if ((strcpy (answerSheetFileName, oneOutputFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (answerSheetFileName ,\"%s\") failed", oneOutputFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     answerSheetFilePt = outputFilePt;
   }
   else {
     errno = 0;
     if ((strcpy (answerSheetFileName, grammarFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (answerSheetFileName ,\"%s\") failed", grammarFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((strcat(answerSheetFileName, FILE_EXTENSION_ANSWER_SHEET)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(answerSheetFileName ,\"%s\") failed", FILE_EXTENSION_ANSWER_SHEET);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((answerSheetFilePt = fopen (answerSheetFileName, "w")) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", answerSheetFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
   }
 }

 if (b_print_text_LR0_table | b_print_text_sLR1_table) {
   if (b_one_output_file) {
     errno = 0;
     if ((strcpy (textTablesFileName, oneOutputFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (textTablesFileName ,\"%s\") failed", oneOutputFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     textTablesFilePt = outputFilePt;
   }
   else {
     errno = 0;
     if ((strcpy (textTablesFileName, grammarFileName)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (textTablesFileName ,\"%s\") failed", grammarFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((strcat(textTablesFileName, FILE_EXTENSION_TEXT_TABLES)) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(textTablesFileName ,\"%s\") failed", FILE_EXTENSION_TEXT_TABLES);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
     errno = 0;
     if ((textTablesFilePt = fopen (textTablesFileName, "w")) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", textTablesFileName);
       ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
     }
   }
 }

 if (b_write_binary_LR0_table) {
   errno = 0;
   if ((strcpy (lr0binaryTableFileName, grammarFileName)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (lr0binaryTableFileName ,\"%s\") failed", grammarFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((strcat(lr0binaryTableFileName, FILE_EXTENSION_BINARY_LR0_TABLE)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(lr0binaryTableFileName ,\"%s\") failed", FILE_EXTENSION_BINARY_LR0_TABLE);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((lr0binaryTableFilePt = fopen (lr0binaryTableFileName, "wb")) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", lr0binaryTableFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
 }

 if (b_write_binary_sLR1_table) {
   errno = 0;
   if ((strcpy (slr1binaryTableFileName, grammarFileName)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (slr1binaryTableFileName ,\"%s\") failed", grammarFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((strcat(slr1binaryTableFileName, FILE_EXTENSION_BINARY_SLR1_TABLE)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat(slr1binaryTableFileName ,\"%s\") failed", FILE_EXTENSION_BINARY_SLR1_TABLE);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
   errno = 0;
   if ((slr1binaryTableFilePt = fopen (slr1binaryTableFileName, "wb")) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", slr1binaryTableFileName);
     ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
   }
 }

 COMMLINE_free_commLine_data();
}

/*
*---------------------------------------------------------------------
* Print output file header
*---------------------------------------------------------------------
*/

void print_output_header (FILE *filePt, char *fileName, int argc, char *argv[])
{
   /* Header consists of:                         */
   /*   "File name", "Created by" and "Date/Time" */
   /*   "Command line"                            */
   /* Put the header inside a multi-line comment  */

   fprintf (filePt, "%s", OPEN_OUTPUT_COMMENT);
   REPORT_write_output_header (filePt, fileName, progName, VERSION_LABEL, VERSION_STRING);
   REPORT_newLine (filePt, 1);
   REPORT_print_command_line (filePt, "  ", argc, argv);
   REPORT_newLine (filePt, 1);
   fprintf (filePt, "%s\n", CLOSE_OUTPUT_COMMENT);
   REPORT_newLine (filePt, 1);
}

/*
*---------------------------------------------------------------------
* Print grammar rules to output file
*---------------------------------------------------------------------
*/

void print_grammar_rules (FILE *filePt)
{
 unsigned int
    iRule,
  iSymbol;

 /* Print list of rules */

 fprintf (filePt, "+------------+\n");
 fprintf (filePt, "| RULES: %3d |\n", (int) totRules);
 fprintf (filePt, "+------------+\n");
 REPORT_newLine (filePt, 1);

 for (iRule = 1; iRule <= totRules; iRule++) {
   fprintf (filePt, "%3d: %s ->", (int) iRule, symbolCode2symbolString (rulePos2symbolCode (iRule, 0)));
   for (iSymbol = 1; (int) iSymbol <= (int) ruleNumber2ruleSize(iRule); iSymbol++)
     fprintf (filePt, " %s", symbolCode2symbolString (rulePos2symbolCode(iRule, iSymbol)));
   REPORT_newLine (filePt, 1);
 }
 REPORT_newLine (filePt, 1);
}

/*
*---------------------------------------------------------------------
* Print debugging information (internal data structures)
*---------------------------------------------------------------------
*/

void print_grammar_data (int argc, char *argv[])
{
 unsigned int
   iSymbol,
   iUse,
   totUses;
 t_symbolCode
   symbolCode;

 /* First print the header and grammar rules, then the grammar symbol details */

 if (! b_one_output_file || ! b_output_started) {
   print_output_header (symbolsFilePt, symbolsFileName, argc, argv);
   print_grammar_rules (symbolsFilePt);
   b_output_started = true;
 }

 /* Print list of non-terminals */

 fprintf (symbolsFilePt, "+--------------------+\n");
 fprintf (symbolsFilePt, "| NON-TERMINALS: %3d |\n", (int) totNonTerminals);
 fprintf (symbolsFilePt, "+--------------------+\n");
 REPORT_newLine (symbolsFilePt, 1);

 for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   fprintf (symbolsFilePt, "Symbol %04d: %s\n", symbolCode, symbolCode2symbolString (symbolCode));
   totUses = symbolCode2totUses (symbolCode, t_lefthand);
   fprintf (symbolsFilePt, "In lefthand  side of %d rules: ", (int) totUses);
   for (iUse = 1; iUse <= totUses; iUse++)
     fprintf (symbolsFilePt, "%d  ", (int) symbolCode2use (symbolCode, t_lefthand, iUse));
   REPORT_newLine (symbolsFilePt, 1);
   totUses = symbolCode2totUses (symbolCode, t_righthand);
   fprintf (symbolsFilePt, "In righthand side of %d rules: ", (int) totUses);
   for (iUse = 1; iUse <= totUses; iUse++)
     fprintf (symbolsFilePt, "%d  ", (int) symbolCode2use (symbolCode, t_righthand, iUse));
   REPORT_newLine (symbolsFilePt, 2);
 }

 /* Print list of terminals */

 fprintf (symbolsFilePt, "+----------------+\n");
 fprintf (symbolsFilePt, "| TERMINALS: %3d |\n", (int) totTerminals);
 fprintf (symbolsFilePt, "+----------------+\n");
 REPORT_newLine (symbolsFilePt, 1);

 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   fprintf (symbolsFilePt, "Symbol %04d: %s", (int) symbolCode, symbolCode2symbolString (symbolCode));
   if (symbolCode == end_of_input_code)
     fprintf (symbolsFilePt, " (END_OF_INPUT)");
   totUses = symbolCode2totUses (symbolCode, t_righthand);
   fprintf (symbolsFilePt, "\nIn righthand side of %d rules: ", (int) totUses);
   for (iUse = 1; iUse <= totUses; iUse++)
     fprintf (symbolsFilePt, "%d  ", (int) symbolCode2use (symbolCode, t_righthand, iUse));
   REPORT_newLine (symbolsFilePt, 2);
 }

 /* Print epsilon usage */

 fprintf (symbolsFilePt, "+---------+\n");
 fprintf (symbolsFilePt, "| Epsilon |\n");
 fprintf (symbolsFilePt, "+---------+\n");
 REPORT_newLine (symbolsFilePt, 1);

 symbolCode = epsilon_code;
 fprintf (symbolsFilePt, "Symbol %04d: %s\n", (int) symbolCode, symbolCode2symbolString (symbolCode));
 totUses = symbolCode2totUses (symbolCode, t_righthand);
 fprintf (symbolsFilePt, "In righthand side of %d rules: ", (int) totUses);
 for (iUse = 1; iUse <= totUses; iUse++)
   fprintf (symbolsFilePt, "%d  ", (int) symbolCode2use (symbolCode, t_righthand, iUse));
 REPORT_newLine (symbolsFilePt, 2);
}

/*
*---------------------------------------------------------------------
* Print first and follow sets
*---------------------------------------------------------------------
*/

void print_sets (int argc, char *argv[])
{
 unsigned int
   iSymbol,
   iPosInSet,
   symbolsInSet;
 t_symbolCode
   symbolCode,
   symbolInSetCode;

 /* First print the header and grammar rules, then the FIRST and FOLLOW sets */

 if (! b_one_output_file || ! b_output_started) {
   print_output_header (setsFilePt, setsFileName, argc, argv);
   print_grammar_rules (setsFilePt);
   b_output_started = true;
 }

 /* Print FIRST set */

 fprintf (setsFilePt, "+------------+\n");
 fprintf (setsFilePt, "| FIRST sets |\n");
 fprintf (setsFilePt, "+------------+\n");
 REPORT_newLine (setsFilePt, 1);

 for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   fprintf (setsFilePt, "Symbol %04d: %s\n", symbolCode, symbolCode2symbolString (symbolCode));
   symbolsInSet = setSize (t_firstSet, symbolCode);
   for (iPosInSet = 1; iPosInSet <= symbolsInSet; iPosInSet++) {
     symbolInSetCode = getSymbolInSet (t_firstSet, symbolCode, iPosInSet);
     fprintf (setsFilePt, "  %04d: %s\n", symbolInSetCode, symbolCode2symbolString (symbolInSetCode));
   }
   REPORT_newLine (setsFilePt, 1);
 }

 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   fprintf (setsFilePt, "Symbol %04d: %s\n", symbolCode, symbolCode2symbolString (symbolCode));
   symbolsInSet = setSize (t_firstSet, symbolCode);
   for (iPosInSet = 1; iPosInSet <= symbolsInSet; iPosInSet++) {
     symbolInSetCode = getSymbolInSet (t_firstSet, symbolCode, iPosInSet);
     fprintf (setsFilePt, "  %04d: %s\n", symbolInSetCode, symbolCode2symbolString (symbolInSetCode));
   }
   REPORT_newLine (setsFilePt, 1);
 }

 /* Print FIRST set */

 fprintf (setsFilePt, "+-------------+\n");
 fprintf (setsFilePt, "| FOLLOW sets |\n");
 fprintf (setsFilePt, "+-------------+\n");
 REPORT_newLine (setsFilePt, 1);

 for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   fprintf (setsFilePt, "Symbol %04d: %s\n", symbolCode, symbolCode2symbolString (symbolCode));
   symbolsInSet = setSize (t_followSet, symbolCode);
   for (iPosInSet = 1; iPosInSet <= symbolsInSet; iPosInSet++) {
     symbolInSetCode = getSymbolInSet (t_followSet, symbolCode, iPosInSet);
     fprintf (setsFilePt, "  %04d: %s\n", symbolInSetCode, symbolCode2symbolString (symbolInSetCode));
   }
   REPORT_newLine (setsFilePt, 1);
 }

}

/*
*---------------------------------------------------------------------
* Print parse table report header
*---------------------------------------------------------------------
*/

void print_text_parse_table_report_header (int argc, char *argv[])
{
 if (! b_one_output_file || ! b_output_started) {
   print_output_header (textTablesFilePt, textTablesFileName, argc, argv);
   print_grammar_rules (textTablesFilePt);
   b_output_started = true;
 }
}

/*
*-----------------------------------------------------------------------------
* For a given state, return the maximum number of actions in the parse table
*-----------------------------------------------------------------------------
*/

short int stateCode2maxParseActions (t_parse_table_type parse_table_type, t_stateCode stateCode)
{
 unsigned int
   iSymbol,
   totParseActions = 0,
   maxActionsInState = 0;
 t_symbolCode
   symbolCode;

 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   totParseActions = parseTablePos2totParseActions (parse_table_type, stateCode, symbolCode);
   maxActionsInState = GREATEST(maxActionsInState,totParseActions);
 }
 for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   totParseActions = parseTablePos2totParseActions (parse_table_type, stateCode, symbolCode);
   maxActionsInState = GREATEST(maxActionsInState,totParseActions);
 }
 return ((short int) maxActionsInState);
}

/*
*---------------------------------------------------------------------
* Print parsing table
*---------------------------------------------------------------------
*/

#define AUX_STRING_LENGTH 1500

void print_text_parse_table (t_parse_table_type parse_table_type)
{
 unsigned int
   iState,
   iSymbol;
 int
   iAction,
   columnWidth,
   tableWidth,
   padding;
 t_stateCode
   stateCode;
 t_symbolCode
   symbolCode = 0;
 short int
   accParseActions = 0,
   totParseActions = 0,
   maxActionsInState = 0;
 t_parseAction
   parseAction;
 size_t
   auxStringLength;
 char
   *headerLine1 = NULL,
   *headerLine2 = NULL,
   *headerLine3 = NULL,
   *detailLine  = NULL,
   *auxString   = NULL,
   *dashes      = NULL;

 /* Initialize a few things */

 headerLine1 = (char *) STRING_allocate();
 headerLine2 = (char *) STRING_allocate();
 headerLine3 = (char *) STRING_allocate();
 detailLine  = (char *) STRING_allocate();
 auxString   = (char *) STRING_allocate();
 dashes      = (char *) STRING_allocate();

 auxStringLength = AUX_STRING_LENGTH;
 (void) STRING_set (&auxString, (unsigned char) 0, auxStringLength + 2);

 /* Adjust symbol column width */

 columnWidth = parse_table_symbol_width;
 padding = 0;
 if (columnWidth <= 0) {

   /* Auto column width with extra padding? */

   if (columnWidth < 0) {
     padding = -columnWidth;
     columnWidth = 0;
   }

   /* Auto column width: must check each symbol's printing label, */
   /* work out the widest one and set the column width to that    */

   for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
     if (b_print_symbol_codes)
       snprintf (auxString, auxStringLength, PARSE_TABLE_SYMBOL_CODE_FORMAT, symbolCode);
     else
       snprintf (auxString, auxStringLength, PARSE_TABLE_SYMBOL_STRING_FORMAT, symbolCode2symbolString (symbolCode));
     columnWidth = (int) GREATEST (columnWidth, (int) strlen (auxString));
   }
   for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
     if (b_print_symbol_codes)
       snprintf (auxString, auxStringLength, PARSE_TABLE_SYMBOL_CODE_FORMAT, symbolCode);
     else
       snprintf (auxString, auxStringLength, PARSE_TABLE_SYMBOL_STRING_FORMAT, symbolCode2symbolString (symbolCode));
     columnWidth = (int) GREATEST (columnWidth, (int) strlen (auxString));
   }

   /* Must also check parse actions printing labels and set column width accordingly */

   snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_SHIFT, (int) totDFAstates);
   columnWidth = (int) GREATEST (columnWidth, (int) strlen (auxString));
   snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_REDUCE, (int) totRules);
   columnWidth = (int) GREATEST (columnWidth, (int) strlen (auxString));
   snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_GOTO, (int) totDFAstates);
   columnWidth = (int) GREATEST (columnWidth, (int) strlen (auxString));
   snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_ACCEPT);
   columnWidth = (int) GREATEST (columnWidth, (int) strlen (auxString));
   snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_ERROR);
   columnWidth = (int) GREATEST (columnWidth, (int) strlen (auxString));
   columnWidth += 2 * padding;
 }

 tableWidth = (int) (PARSE_TABLE_STATE_WIDTH + (totTerminals+totNonTerminals-1)*(columnWidth+1)-1 + 3);

 /* Now we can allocate and start building the various strings */

 (void) STRING_clear (&headerLine1);
 (void) STRING_clear (&headerLine2);
 (void) STRING_clear (&headerLine3);
 (void) STRING_clear (&detailLine);
 (void) STRING_clear (&auxString);
 (void) STRING_clear (&dashes);

 (void) STRING_extend (&dashes, (unsigned char) '-', (size_t) columnWidth);
 (void) STRING_concatenate (&dashes, "+");

 /* Print the table header */

 (void) STRING_copy (&headerLine1, "+");
 (void) STRING_extend ((char **) &headerLine1, '-', (size_t) tableWidth-2);
 (void) STRING_concatenate (&headerLine1, "+");

 fprintf (textTablesFilePt, "%s\n", headerLine1);

 (void) STRING_copy (&headerLine1, "| ");
 switch (parse_table_type) {
   case (t_LR0_parse_table) : (void) STRING_copy (&auxString, "LR(0) Parse Table");  break;
   case (t_sLR1_parse_table): (void) STRING_copy (&auxString, "sLR(1) Parse Table"); break;
   case (t_diff_parse_table): (void) STRING_copy (&auxString, "LR(0) and sLR(1) table differences");
 }
 (void) STRING_justify (&auxString, tableWidth-3, ' ', STRING_t_justify_left);
 (void) STRING_concatenate (&headerLine1, auxString);
 (void) STRING_concatenate (&headerLine1, "|");

 fprintf (textTablesFilePt, "%s\n", headerLine1);

 (void) STRING_copy (&headerLine1, "+");
 (void) STRING_extend ((char **) &headerLine1, '-', (size_t) PARSE_TABLE_STATE_WIDTH);
 (void) STRING_concatenate (&headerLine1, "+");
 (void) STRING_extend ((char **) &headerLine1, '-', (size_t) (totTerminals)*(columnWidth+1)-1);
 (void) STRING_concatenate (&headerLine1, "+");
 (void) STRING_extend ((char **) &headerLine1, '-', (size_t) (totNonTerminals-1)*(columnWidth+1)-1);
 (void) STRING_concatenate (&headerLine1, "+");
 (void) STRING_concatenate (&headerLine1, "    +-----------------+");

 fprintf (textTablesFilePt, "%s\n", headerLine1);

 (void) STRING_copy (&headerLine1, "|");
 (void) STRING_extend ((char **) &headerLine1, ' ', (size_t) PARSE_TABLE_STATE_WIDTH);
 (void) STRING_concatenate (&headerLine1, "|");
 (void) STRING_copy (&auxString, "ACTION");
 (void) STRING_justify (&auxString, (size_t) (totTerminals)*(columnWidth+1)-1, ' ', STRING_t_justify_centre);
 (void) STRING_concatenate (&headerLine1, auxString);
 (void) STRING_concatenate (&headerLine1, "|");
 (void) STRING_copy (&auxString, "GOTO");
 (void) STRING_justify (&auxString, (size_t) (totNonTerminals-1)*(columnWidth+1)-1, ' ', STRING_t_justify_centre);
 (void) STRING_concatenate (&headerLine1, auxString);
 (void) STRING_concatenate (&headerLine1, "|    |  TOTAL ACTIONS  |");

 fprintf (textTablesFilePt, "%s\n", headerLine1);

 (void) STRING_copy (&headerLine1, "|");
 (void) STRING_copy (&auxString, "STATE");
 (void) STRING_justify (&auxString, PARSE_TABLE_STATE_WIDTH, ' ', STRING_t_justify_centre);
 (void) STRING_concatenate (&headerLine1, auxString);
 (void) STRING_concatenate (&headerLine1, "+");

 (void) STRING_copy (&headerLine2, "|");
 (void) STRING_extend ((char **) &headerLine2, ' ', (size_t) PARSE_TABLE_STATE_WIDTH);
 (void) STRING_concatenate (&headerLine2, "|");

 (void) STRING_copy (&headerLine3, "+");
 (void) STRING_extend ((char **) &headerLine3, '-', (size_t) PARSE_TABLE_STATE_WIDTH);
 (void) STRING_concatenate (&headerLine3, "+");

 auxStringLength = (size_t) (columnWidth + 1) * (totTerminals + totNonTerminals) + 50;

 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   if (b_print_symbol_codes)
     snprintf (auxString, auxStringLength, PARSE_TABLE_SYMBOL_CODE_FORMAT, (int) symbolCode);
   else
     snprintf (auxString, auxStringLength, PARSE_TABLE_SYMBOL_STRING_FORMAT, symbolCode2symbolString (symbolCode));
   (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
   (void) STRING_concatenate (&headerLine1, dashes);
   (void) STRING_concatenate (&headerLine2, auxString);
   (void) STRING_concatenate (&headerLine2, "|");
   (void) STRING_concatenate (&headerLine3, dashes);
 }
 for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   if (b_print_symbol_codes)
     snprintf (auxString, auxStringLength, PARSE_TABLE_SYMBOL_CODE_FORMAT, symbolCode);
   else
     snprintf (auxString, auxStringLength, PARSE_TABLE_SYMBOL_STRING_FORMAT, symbolCode2symbolString (symbolCode));
   (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
   (void) STRING_concatenate (&headerLine1, dashes);
   (void) STRING_concatenate (&headerLine2, auxString);
   (void) STRING_concatenate (&headerLine2, "|");
   (void) STRING_concatenate (&headerLine3, dashes);
 }

 (void) STRING_concatenate (&headerLine1, "    +-----+-----+-----+");
 (void) STRING_concatenate (&headerLine2, "    |shift| red | goto|");
 (void) STRING_concatenate (&headerLine3, "    +-----+-----+-----+");

 /* Print the table header */

 fprintf (textTablesFilePt, "%s\n", headerLine1);
 fprintf (textTablesFilePt, "%s\n", headerLine2);
 fprintf (textTablesFilePt, "%s\n", headerLine3);

 fflush(NULL);

 /* Print list of canonical LR(0) states and the corresponding actions */

 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   maxActionsInState = stateCode2maxParseActions (parse_table_type, stateCode);
   for (iAction = 1; iAction <= maxActionsInState; iAction++) {
     if (maxActionsInState == 1)
       snprintf (auxString, auxStringLength, PARSE_TABLE_STATE_PLAIN_FORMAT, stateCode);
     else if (iAction == 1)
       snprintf (auxString, auxStringLength, PARSE_TABLE_STATE_CONFLICT_FORMAT, stateCode);
     else
       snprintf (auxString, auxStringLength, PARSE_TABLE_STATE_BLANK_FORMAT);
     (void) STRING_justify (&auxString, PARSE_TABLE_STATE_WIDTH, ' ', STRING_t_justify_centre);
     (void) STRING_copy (&detailLine, "|");
     (void) STRING_concatenate (&detailLine, auxString);
     (void) STRING_concatenate (&detailLine, "|");
     (void) STRING_set (&auxString, 0, auxStringLength + 2);
     for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
       symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
       totParseActions = (short int) parseTablePos2totParseActions (parse_table_type, stateCode, symbolCode);
       if (iAction > totParseActions)
         snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_ERROR);
       else {
         parseAction = parseTablePos2parseAction (parse_table_type, stateCode, symbolCode, (unsigned int) iAction);
         switch (parseAction.parseActionType) {
           case (t_shift): {
             snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_SHIFT, parseAction.parseActionParam.nextState);
             break;
           }
           case (t_reduce): {
             snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_REDUCE, (int) parseAction.parseActionParam.reductionRule);
             break;
           }
           case (t_accept): {
             snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_ACCEPT);
             break;
           }
           case (t_goto): {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid parse action 'goto' for terminal %d in state %d\n", symbolCode, stateCode);
             ERROR_fatal_error (0, progName, __func__, ERROR_auxErrorMsg);
           }
           default: {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown parse action type %d in state %d\n", parseAction.parseActionType, stateCode);
             ERROR_fatal_error (0, progName, __func__, ERROR_auxErrorMsg);
           }
         }
       }
       (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
       (void) STRING_concatenate (&detailLine, auxString);
       (void) STRING_concatenate (&detailLine, "|");
     }
     (void) STRING_set (&auxString, 0, auxStringLength + 2);
     for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
       symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
       totParseActions = (short int) parseTablePos2totParseActions (parse_table_type, stateCode, symbolCode);
       if (iAction > totParseActions)
         snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_ERROR);
       else {
         parseAction = parseTablePos2parseAction (parse_table_type, stateCode, symbolCode, (unsigned int) iAction);
         switch (parseAction.parseActionType) {
           case (t_goto): {
             snprintf (auxString, auxStringLength, PARSE_TABLE_STRING_GOTO, parseAction.parseActionParam.nextState);
             break;
           }
           case (t_shift): {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid parse action 'shift' for non-terminal %d in state %d\n", symbolCode, stateCode);
             ERROR_fatal_error (0, progName, __func__, ERROR_auxErrorMsg);
           }
           case (t_reduce): {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid parse action 'reduce' for non-terminal %d in state %d\n", symbolCode, stateCode);
             ERROR_fatal_error (0, progName, __func__, ERROR_auxErrorMsg);
           }
           case (t_accept): {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid parse action 'accept' for non-terminal %d in state %d\n", symbolCode, stateCode);
             ERROR_fatal_error (0, progName, __func__, ERROR_auxErrorMsg);
           }
           default: {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown parse action type %d in state %d\n", parseAction.parseActionType, stateCode);
             ERROR_fatal_error (0, progName, __func__, ERROR_auxErrorMsg);
             break;
           }
         }
       }
       (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
       (void) STRING_concatenate (&detailLine, auxString);
       (void) STRING_concatenate (&detailLine, "|");
     }
     (void) STRING_set (&auxString, 0, auxStringLength + 2);
     if (iAction == 1)
       snprintf (auxString, auxStringLength, "    | %3d | %3d | %3d |",
         (int) parseTableSummaryCol2totParseActions (parse_table_type, t_shift,  stateCode),
         (int) parseTableSummaryCol2totParseActions (parse_table_type, t_reduce, stateCode),
         (int) parseTableSummaryCol2totParseActions (parse_table_type, t_goto,   stateCode) );
     else
       snprintf (auxString, auxStringLength, "    |     |     |     |");
     (void) STRING_concatenate (&detailLine, auxString);
     fprintf (textTablesFilePt, "%s\n", detailLine);
   }
 }

 fprintf (textTablesFilePt, "%s\n", headerLine3);
 REPORT_newLine (textTablesFilePt, 1);

 fflush(NULL);

 /*                                                                 */
 /* Print total actions per grammar symbol followed by grand totals */
 /*                                                                 */

 (void) STRING_copy (&headerLine1, "+");
 (void) STRING_extend ((char **) &headerLine1, '-', (size_t) tableWidth-2);
 (void) STRING_concatenate (&headerLine1, "+");
 fprintf (textTablesFilePt, "%s", headerLine1);
 REPORT_newLine (textTablesFilePt, 1);

 (void) STRING_copy (&auxString, "| TOTAL ACTIONS");
 (void) STRING_justify (&auxString, tableWidth-1, ' ', STRING_t_justify_left);
 (void) STRING_concatenate (&auxString, "|");
 fprintf (textTablesFilePt, "%s", auxString);
 REPORT_newLine (textTablesFilePt, 1);
 fprintf (textTablesFilePt, "%s", headerLine3);
 REPORT_newLine (textTablesFilePt, 1);

 fflush(NULL);

 /* Shifts */

 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 snprintf (auxString, auxStringLength, "|shift");
 (void) STRING_justify (&auxString, PARSE_TABLE_STATE_WIDTH, ' ', STRING_t_justify_left);
 (void) STRING_concatenate (&auxString, " |");
 fprintf (textTablesFilePt, "%s", auxString);
 accParseActions = 0;
 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   accParseActions += totParseActions = (short int) parseTableSummaryRow2totParseActions (parse_table_type, t_shift, symbolCode);
   snprintf (auxString, auxStringLength, "%3d", totParseActions);
   (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
   (void) STRING_concatenate (&auxString, "|");
   fprintf (textTablesFilePt, "%s", auxString);
 }
 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   accParseActions += totParseActions = (short int) parseTableSummaryRow2totParseActions (parse_table_type, t_shift, symbolCode);
   snprintf (auxString, auxStringLength, "%3d", totParseActions);
   (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
   (void) STRING_concatenate (&auxString, "|");
   fprintf (textTablesFilePt, "%s", auxString);
 }
 fprintf (textTablesFilePt, "    | %3d |     |     |", accParseActions);
 REPORT_newLine (textTablesFilePt, 1);
 fprintf (textTablesFilePt, "%s", headerLine3);
 REPORT_newLine (textTablesFilePt, 1);

 fflush(NULL);

 /* Reductions */

 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 snprintf (auxString, auxStringLength, "|reduce");
 (void) STRING_justify (&auxString, PARSE_TABLE_STATE_WIDTH, ' ', STRING_t_justify_left);
 (void) STRING_concatenate (&auxString, " |");
 fprintf (textTablesFilePt, "%s", auxString);
 accParseActions = 0;
 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   accParseActions += totParseActions = (short int) parseTableSummaryRow2totParseActions (parse_table_type, t_reduce, symbolCode);
   snprintf (auxString, auxStringLength, "%3d", totParseActions);
   (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
   (void) STRING_concatenate (&auxString, "|");
   fprintf (textTablesFilePt, "%s", auxString);
 }
 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   accParseActions += totParseActions = (short int) parseTableSummaryRow2totParseActions (parse_table_type, t_reduce, symbolCode);
   snprintf (auxString, auxStringLength, "%3d", totParseActions);
   (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
   (void) STRING_concatenate (&auxString, "|");
   fprintf (textTablesFilePt, "%s", auxString);
 }
 fprintf (textTablesFilePt, "    |     | %3d |     |", accParseActions);
 REPORT_newLine (textTablesFilePt, 1);
 fprintf (textTablesFilePt, "%s", headerLine3);
 REPORT_newLine (textTablesFilePt, 1);

 fflush(NULL);

 /* Goto */

 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 snprintf (auxString, auxStringLength, "|goto");
 (void) STRING_justify (&auxString, PARSE_TABLE_STATE_WIDTH, ' ', STRING_t_justify_left);
 (void) STRING_concatenate (&auxString, " |");
 fprintf (textTablesFilePt, "%s", auxString);
 accParseActions = 0;
 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   accParseActions += totParseActions = (short int) parseTableSummaryRow2totParseActions (parse_table_type, t_goto, symbolCode);
   snprintf (auxString, auxStringLength, "%3d", totParseActions);
   (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
   (void) STRING_concatenate (&auxString, "|");
   fprintf (textTablesFilePt, "%s", auxString);
 }
 (void) STRING_set (&auxString, 0, auxStringLength + 2);
 for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   accParseActions += totParseActions = (short int) parseTableSummaryRow2totParseActions (parse_table_type, t_goto, symbolCode);
   snprintf (auxString, auxStringLength, "%3d", totParseActions);
   (void) STRING_justify (&auxString, columnWidth, ' ', STRING_t_justify_centre);
   (void) STRING_concatenate (&auxString, "|");
   fprintf (textTablesFilePt, "%s", auxString);
 }
 fprintf (textTablesFilePt, "    |     |     | %3d |", accParseActions);
 REPORT_newLine (textTablesFilePt, 1);
 fprintf (textTablesFilePt, "%s", headerLine3);
 REPORT_newLine (textTablesFilePt, 3);

 STRING_release (&dashes);
 STRING_release (&auxString);
 STRING_release (&detailLine);
 STRING_release (&headerLine3);
 STRING_release (&headerLine2);
 STRING_release (&headerLine1);
}

/*
*--------------------------------------------------------------------------------
* Write a single variable (but NOT a string) to a parse table binary file
* THIS IS MEANT TO SIMPLIFY THE CODE BY ENCAPSULATING THE EXTRA OVERHEAD
*--------------------------------------------------------------------------------
*/

void write_var_to_parse_table_bin_file  (
  t_parse_table_type  parse_table_type,
  const void         *pVar,
  size_t              varSize,
  const char         *varName )
{
  FILE
    *parseTableBinFilePt;
  size_t
    bytesWritten;

  switch (parse_table_type) {
      case (t_LR0_parse_table) : parseTableBinFilePt = lr0binaryTableFilePt;  break;
      case (t_sLR1_parse_table): parseTableBinFilePt = slr1binaryTableFilePt; break;
      default: {
        snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid pase table type %d\n", parse_table_type);
        ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
      }
  }
  bytesWritten = fwrite (pVar, varSize, (size_t) 1, parseTableBinFilePt);
  if (bytesWritten == 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to write %s to parse table binary file\n", varName);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
  }
}

#define WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,varName) \
       (write_var_to_parse_table_bin_file(parse_table_type,&varName,sizeof(varName),#varName))

/*
*------------------------------------------------------------------------
* Write a single STRING variable to a parse table binary file
* THIS IS MEANT TO SIMPLIFY THE CODE BY ENCAPSULATING THE EXTRA OVERHEAD
*------------------------------------------------------------------------
*/

void write_string_var_to_parse_table_bin_file (
  t_parse_table_type  parse_table_type,
  const char         *string,
  size_t              stringSize,
  const char         *varName )
{
  FILE
    *parseTableBinFilePt;
  size_t
    bytesWritten;

  switch (parse_table_type) {
      case (t_LR0_parse_table) : parseTableBinFilePt = lr0binaryTableFilePt;  break;
      case (t_sLR1_parse_table): parseTableBinFilePt = slr1binaryTableFilePt; break;
      default: {
        snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid pase table type %d\n", parse_table_type);
        ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
      }
  }
  bytesWritten = fwrite (string, stringSize, (size_t) 1, parseTableBinFilePt);
  if (bytesWritten == 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to write %s to parse table binary file\n", varName);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
  }
}

#define WRITE_STRING_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,varName) \
       (write_string_var_to_parse_table_bin_file(parse_table_type,varName,1+strlen(varName),#varName))

/*
*------------------------------------------------------------------------
* Write a single parse table entry to a parse table binary file
* THIS IS MEANT TO SIMPLIFY THE CODE BY ENCAPSULATING THE EXTRA OVERHEAD
*------------------------------------------------------------------------
*/

void write_parse_action_to_parse_table_bin_file (
  t_parse_table_type  parse_table_type,
  t_stateCode         stateCode,
  t_symbolCode        symbolCode,
  t_parseAction       parseAction )
{
  FILE
    *parseTableBinFilePt;
  size_t
    bytesWritten;

  switch (parse_table_type) {
      case (t_LR0_parse_table) : parseTableBinFilePt = lr0binaryTableFilePt;  break;
      case (t_sLR1_parse_table): parseTableBinFilePt = slr1binaryTableFilePt; break;
      default: {
        snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid pase table type %d\n", parse_table_type);
        ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
      }
  }
  bytesWritten = fwrite (&parseAction, sizeof (parseAction), (size_t) 1, parseTableBinFilePt);
  if (bytesWritten == 0) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to write parseAction[%d][%d] to parse table binary file\n", stateCode, symbolCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
  }
}

#define WRITE_PARSE_ACTION_TO_PARSE_TABLE_BIN_FILE(parse_table_type,stateCode,symbolCode,parseAction) \
        (write_parse_action_to_parse_table_bin_file (parse_table_type, stateCode, symbolCode, parseAction))

/*
*---------------------------------------------------------------------
* Write the parse table to a binary file.
* Actually, a lot more is written to the file, namely:
*  - Parse table type: LR(0) or sLR(1)
*  - Terminals: numeric code and string
*  - Non-terminals: numeric code and string
*  - Grammar rules
*  - Parse table
*---------------------------------------------------------------------
*/

void write_parse_table_binary_file (t_parse_table_type parse_table_type) {
  unsigned int
    iSymbol;
  unsigned int
    iState,
    iRule,
    iParseAction,
    ruleSize;
  unsigned int
    parseActionsInState,
    totParseActions;
  t_stateCode
    stateCode;
  t_symbolCode
    symbolCode;
  t_parseAction
    parseAction;
  const char *
    symbolString;

  /* Parse table type */

  WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,parse_table_type);

  /* Terminals */

  WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,totTerminals);
  for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
    symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
    symbolString = symbolCode2symbolString (symbolCode);
    WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,symbolCode);          /* For each symbol write */
    WRITE_STRING_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,symbolString); /* the pair code/string  */
  }

  /* Non-terminals */

  WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,totNonTerminals);
  for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
    symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
    symbolString = symbolCode2symbolString (symbolCode);
    WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,symbolCode);          /* For each symbol write */
    WRITE_STRING_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,symbolString); /* the pair code/string  */
  }

  /* Grammar rules */

  WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,totRules);
  for (iRule = 1; iRule <= totRules; iRule++) {                     /* For each rule write the rule size    */
    ruleSize = ruleNumber2ruleSize(iRule);                          /* followed by the rule symbol codes.   */
    WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,ruleSize);   /* NOTE: the rule size does not include */
    symbolCode = rulePos2symbolCode (iRule, 0);                     /* the non-terminal to the left of the  */
    WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,symbolCode); /* rule arrow!                          */
    for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
      symbolCode = rulePos2symbolCode (iRule, iSymbol);
      WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,symbolCode);
    }
  }

  /* The actual parse table entries */

  WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,totDFAstates);
  for (iState = 1; iState <= totDFAstates; iState++) {
    stateCode = dfa_stateNumber2stateCode (iState);

    /* Work out and write the total number of parse actions in this state */

    totParseActions = 0;
    for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
      symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
      totParseActions += parseTablePos2totParseActions (parse_table_type, stateCode, symbolCode);
    }
    for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
      symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
      totParseActions += parseTablePos2totParseActions (parse_table_type, stateCode, symbolCode);
    }
    WRITE_VAR_TO_PARSE_TABLE_BIN_FILE(parse_table_type,totParseActions);

    /* Now write this many pairs <symbol,parseAction> */

    for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
      symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
      parseActionsInState = parseTablePos2totParseActions (parse_table_type, stateCode, symbolCode);
      for (iParseAction = 1; iParseAction <= parseActionsInState; iParseAction++) {
        parseAction = parseTablePos2parseAction (parse_table_type, stateCode, symbolCode, iParseAction);
        WRITE_PARSE_ACTION_TO_PARSE_TABLE_BIN_FILE(parse_table_type,stateCode,symbolCode,parseAction);
      }
    }
    for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
      symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
      parseActionsInState = parseTablePos2totParseActions (parse_table_type, stateCode, symbolCode);
      for (iParseAction = 1; iParseAction <= parseActionsInState; iParseAction++) {
        parseAction = parseTablePos2parseAction (parse_table_type, stateCode, symbolCode, iParseAction);
        WRITE_PARSE_ACTION_TO_PARSE_TABLE_BIN_FILE(parse_table_type,stateCode,symbolCode,parseAction);
      }
    }
  }
}

/*
*--------------------------------------------------------
* Imprime quadros de respostas da prova de Compiladores
*--------------------------------------------------------
*/

#define QUADRO_A_1_PONTOS_POR_CELULA  2
#define QUADRO_A_2_PONTOS_POR_LINHA   1
#define QUADRO_B_1_PONTOS_POR_LINHA   1
#define QUADRO_C_1_PONTOS_POR_LINHA   3
#define QUADRO_C_2_PONTOS_POR_CELULA  2
#define QUADRO_C_3_PONTOS_POR_LINHA   5
#define QUADRO_C_4_PONTOS_POR_LINHA   5
#define QUADRO_C_5_PONTOS_POR_LINHA   5
#define QUADRO_D_1_PONTOS_POR_LINHA   1
#define QUADRO_E_1_PONTOS_POR_LINHA   1

typedef struct {
  t_stateCode stateCode;
  int stateTransitions;
}
  t_stateAndTransitions;

static int compareStates (const void * state1, const void * state2)
{
   return ( (*(t_stateAndTransitions *) state1).stateTransitions - (*(t_stateAndTransitions *) state2).stateTransitions);
}

void print_answer_sheet (int argc, char *argv[])
{
 int
   iState,
   iShiftState,
   iRule,
   iSymbol,
   iTransition,
   transitions,
   maxSymbolLength,
   shifts,
   reductions,
   gotos;
 t_stateCode
   stateCode;
 t_symbolCode
   symbolCode;
 char
   *symbolString = NULL;
 t_stateAndTransitions
   statesAndTransitions[tot_DFA_shift_states];
int
  quadro_A_1_CELULAS = 0,
  quadro_A_2_LINHAS  = 0,
  quadro_B_1_LINHAS  = 0,
  quadro_C_1_LINHAS  = 0,
  quadro_C_2_CELULAS = 0,
  quadro_C_3_LINHAS  = 0,
  quadro_C_4_LINHAS  = 0,
  quadro_C_5_LINHAS  = 0,
  quadro_D_1_LINHAS  = 0,
  quadro_E_1_LINHAS  = 0;
int
  quadro_A_1_pontos = 0,
  quadro_A_2_pontos = 0,
  quadro_B_1_pontos = 0,
  quadro_C_1_pontos = 0,
  quadro_C_2_pontos = 0,
  quadro_C_3_pontos = 0,
  quadro_C_4_pontos = 0,
  quadro_C_5_pontos = 0,
  quadro_D_1_pontos = 0,
  quadro_E_1_pontos = 0;
int
  quadro_A_pontos = 0,
  quadro_C_pontos = 0;

 quadro_A_1_CELULAS = 2;
 quadro_A_2_LINHAS  = tot_NFA_non_deterministic_states;
 quadro_B_1_LINHAS  = totTerminals + totNonTerminals + 1;
 quadro_C_1_LINHAS  = tot_DFA_shift_states;
 quadro_C_2_CELULAS = tot_DFA_1_reduce_states;
 quadro_C_3_LINHAS  = tot_DFA_shift_1_reduce_states;
 quadro_C_4_LINHAS  = tot_DFA_N_reduce_states;
 quadro_C_5_LINHAS  = tot_DFA_shift_N_reduce_states;
 quadro_D_1_LINHAS  = totTerminals + totNonTerminals;
 quadro_E_1_LINHAS  = totTerminals + totNonTerminals;

 quadro_A_1_pontos = quadro_A_1_CELULAS * QUADRO_A_1_PONTOS_POR_CELULA;
 quadro_A_2_pontos = quadro_A_2_LINHAS  * QUADRO_A_2_PONTOS_POR_LINHA ;
 quadro_B_1_pontos = quadro_B_1_LINHAS  * QUADRO_B_1_PONTOS_POR_LINHA ;
 quadro_C_1_pontos = quadro_C_1_LINHAS  * QUADRO_C_1_PONTOS_POR_LINHA ;
 quadro_C_2_pontos = quadro_C_2_CELULAS * QUADRO_C_2_PONTOS_POR_CELULA;
 quadro_C_3_pontos = quadro_C_3_LINHAS  * QUADRO_C_3_PONTOS_POR_LINHA ;
 quadro_C_4_pontos = quadro_C_4_LINHAS  * QUADRO_C_4_PONTOS_POR_LINHA ;
 quadro_C_5_pontos = quadro_C_5_LINHAS  * QUADRO_C_5_PONTOS_POR_LINHA ;
 quadro_D_1_pontos = quadro_D_1_LINHAS  * QUADRO_D_1_PONTOS_POR_LINHA ;
 quadro_E_1_pontos = quadro_E_1_LINHAS  * QUADRO_E_1_PONTOS_POR_LINHA ;

 quadro_A_pontos = quadro_A_1_pontos + quadro_A_2_pontos;
 quadro_C_pontos = quadro_C_1_pontos + quadro_C_2_pontos + quadro_C_3_pontos + quadro_C_4_pontos + quadro_C_5_pontos;

 maxSymbolLength = GREATEST (maxTerminalLength, maxNonTerminalLength);
 fprintf (answerSheetFilePt, "+-------------------------------------+\n");
 fprintf (answerSheetFilePt, "|  GABARITO DA PROVA DE COMPILADORES  |\n");
 fprintf (answerSheetFilePt, "+-------------------------------------+\n");
 REPORT_newLine (answerSheetFilePt, 1);
 fprintf (answerSheetFilePt, "QUADRO A: AFND - Estados\n");
 fprintf (answerSheetFilePt, "(Total %d pontos)\n", quadro_A_pontos);
 fprintf (answerSheetFilePt, "   Deterministicos\n");
 fprintf (answerSheetFilePt, "   (%d celulas, %d pontos por celula, total %d pontos)\n", quadro_A_1_CELULAS, QUADRO_A_1_PONTOS_POR_CELULA, quadro_A_1_pontos);
 fprintf (answerSheetFilePt, "       De empilhamento:  %d\n", (int) tot_NFA_shift_states) ;
 fprintf (answerSheetFilePt, "       De reducao:       %d\n", (int) tot_NFA_reduce_states);
 fprintf (answerSheetFilePt, "   Nao-deterministicos:\n");
 fprintf (answerSheetFilePt, "   (%d linhas, %d pontos por linha, total %d pontos)\n", quadro_A_2_LINHAS, QUADRO_A_2_PONTOS_POR_LINHA, quadro_A_2_pontos);
 if (tot_NFA_non_deterministic_states > 0) {
   for (iState = 1; iState <= (int) totNFAstates; iState++) {
     stateCode = nfa_stateNumber2stateCode (iState);
     if (stateCode2stateType (stateCode) != t_NFA_non_deterministic_state)
       continue;
     symbolCode = nfa_stateCode2transitionSymbol (stateCode, 1);
     transitions = (int) nfa_stateCode2totTransitionsFromState (stateCode);
     (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
     (void) STRING_justify ((char **) &symbolString, maxNonTerminalLength, ' ', STRING_t_justify_left);
     fprintf (answerSheetFilePt, "       %s  %d\n", symbolString, transitions - 1);
   }
 }
 fflush(NULL);
 REPORT_newLine (answerSheetFilePt, 1);
 fprintf (answerSheetFilePt, "QUADRO B: AFND - Transicoes\n");
 fprintf (answerSheetFilePt, "(%d linhas, %d pontos por linha, total %d pontos)\n", quadro_B_1_LINHAS, QUADRO_B_1_PONTOS_POR_LINHA, quadro_B_1_pontos);
 fprintf (answerSheetFilePt, "   Com terminais\n");
 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   transitions = (int) nfa_symbolCode2totTransitionsWithSymbol (symbolCode);
   (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
   (void) STRING_justify ((char **) &symbolString, maxTerminalLength, ' ', STRING_t_justify_left);
   fprintf (answerSheetFilePt, "       %s  %d\n", symbolString, transitions);
 }
 fprintf (answerSheetFilePt, "   Com nao-terminais\n");
 for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   transitions = (int) nfa_symbolCode2totTransitionsWithSymbol (symbolCode);
   (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
   (void) STRING_justify ((char **) &symbolString, maxTerminalLength, ' ', STRING_t_justify_left);
   fprintf (answerSheetFilePt, "       %s  %d\n", symbolString, transitions);
 }
 symbolCode = epsilon_code;
 transitions = (int) nfa_symbolCode2totTransitionsWithSymbol (symbolCode);
 fprintf (answerSheetFilePt, "   Com epsilon: %d\n", transitions);
 fflush(NULL);
 REPORT_newLine (answerSheetFilePt, 1);
 fprintf (answerSheetFilePt, "QUADRO C: AFD - Estados\n");
 fprintf (answerSheetFilePt, "(Total %d pontos)\n", quadro_C_pontos);
 fprintf (answerSheetFilePt, "   Sem conflitos\n");
 fprintf (answerSheetFilePt, "     De empilhamento\n");
 fprintf (answerSheetFilePt, "     (%d linhas, %d pontos por linha, total %d pontos)\n", quadro_C_1_LINHAS, QUADRO_C_1_PONTOS_POR_LINHA, quadro_C_1_pontos);
 if (tot_DFA_shift_states > 0) {
   iShiftState = 0;
   for (iState = 1; iState <= (int) totDFAstates; iState++) {
     stateCode = dfa_stateNumber2stateCode (iState);
     if (stateCode2stateType (stateCode) != t_DFA_shift_state)
       continue;
     statesAndTransitions[iShiftState].stateCode = stateCode;
     statesAndTransitions[iShiftState].stateTransitions = (int) dfa_stateCode2totTransitionsFromState (stateCode);
     iShiftState++;
   }
   qsort ((void *) statesAndTransitions, (size_t) tot_DFA_shift_states, sizeof(t_stateAndTransitions), compareStates);
   for (iShiftState = 0; iShiftState < (int) tot_DFA_shift_states; iShiftState++) {
     stateCode = statesAndTransitions[iShiftState].stateCode;
     transitions = statesAndTransitions[iShiftState].stateTransitions;
     fprintf (answerSheetFilePt, "       %2d transicoes:  ", transitions);
     for (iTransition = 1; iTransition <= transitions; iTransition++) {
       symbolCode = dfa_stateCode2transitionSymbol (stateCode, iTransition);
       (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
       (void) STRING_justify ((char **) &symbolString, maxSymbolLength, ' ', STRING_t_justify_left);
       fprintf (answerSheetFilePt, "%s  ", symbolString);
     }
     REPORT_newLine (answerSheetFilePt, 1);
   }
 }
 fflush(NULL);
 fprintf (answerSheetFilePt, "     De reducao:\n");
 fprintf (answerSheetFilePt, "     (%d celulas, %d pontos por celula, total %d pontos)\n", quadro_C_2_CELULAS, QUADRO_C_2_PONTOS_POR_CELULA, quadro_C_2_pontos);
 if (tot_DFA_1_reduce_states > 0) {
   fprintf (answerSheetFilePt, "        ");
   for (iState = 1; iState <= (int) totDFAstates; iState++) {
     stateCode = dfa_stateNumber2stateCode (iState);
     if (stateCode2stateType (stateCode) != t_DFA_1_reduce_state)
       continue;
     fprintf (answerSheetFilePt, "%d  ", dfa_stateCode2reductionRule (stateCode, 1));
   }
   REPORT_newLine (answerSheetFilePt, 1);
 }
 fflush(NULL);
 fprintf (answerSheetFilePt, "   Com conflitos\n");
 fprintf (answerSheetFilePt, "     Empilha-reduz:\n");
 fprintf (answerSheetFilePt, "     (%d linhas, %d pontos por linha, total %d pontos)\n", quadro_C_3_LINHAS, QUADRO_C_3_PONTOS_POR_LINHA, quadro_C_3_pontos);
 if (tot_DFA_shift_1_reduce_states > 0) {
   for (iState = 1; iState <= (int) totDFAstates; iState++) {
     stateCode = dfa_stateNumber2stateCode (iState);
     if (stateCode2stateType (stateCode) != t_DFA_shift_1_reduce_state)
       continue;
     fprintf (answerSheetFilePt, "        Regra %3d - Transicoes:  ", dfa_stateCode2reductionRule (stateCode, 1));
     for (iTransition = 1; iTransition <= dfa_stateCode2totTransitionsFromState (stateCode); iTransition++) {
       symbolCode = dfa_stateCode2transitionSymbol (stateCode, iTransition);
       (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
       (void) STRING_justify ((char **) &symbolString, maxSymbolLength, ' ', STRING_t_justify_left);
       fprintf (answerSheetFilePt, "%s  ", symbolString);
     }
     REPORT_newLine (answerSheetFilePt, 1);
   }
 }
 fflush(NULL);
 fprintf (answerSheetFilePt, "     Reduz-reduz:\n");
 fprintf (answerSheetFilePt, "     (%d linhas, %d pontos por linha, total %d pontos)\n", quadro_C_4_LINHAS, QUADRO_C_4_PONTOS_POR_LINHA, quadro_C_4_pontos);
 if (tot_DFA_N_reduce_states > 0) {
   for (iState = 1; iState <= (int) totDFAstates; iState++) {
     stateCode = dfa_stateNumber2stateCode (iState);
     if (stateCode2stateType (stateCode) != t_DFA_N_reduce_state)
       continue;
     fprintf (answerSheetFilePt, "        Regras: ");
     for (iRule = 1; iRule <= dfa_stateCode2totReductions (stateCode); iRule++)
       fprintf (answerSheetFilePt, "%3d ", dfa_stateCode2reductionRule (stateCode, iRule));
     REPORT_newLine (answerSheetFilePt, 1);
   }
 }
 fflush(NULL);
 fprintf (answerSheetFilePt, "     Empilha-reduz-reduz:\n");
 fprintf (answerSheetFilePt, "     (%d linhas, %d pontos por linha, total %d pontos)\n", quadro_C_5_LINHAS, QUADRO_C_5_PONTOS_POR_LINHA, quadro_C_5_pontos);
 if (tot_DFA_shift_N_reduce_states > 0) {
   for (iState = 1; iState <= (int) totDFAstates; iState++) {
     stateCode = dfa_stateNumber2stateCode (iState);
     if (stateCode2stateType (stateCode) != t_DFA_shift_N_reduce_state)
       continue;
     fprintf (answerSheetFilePt, "        Regras: ");
     for (iRule = 1; iRule <= dfa_stateCode2totReductions (stateCode); iRule++)
       fprintf (answerSheetFilePt, "%3d ", dfa_stateCode2reductionRule (stateCode, iRule));
     fprintf (answerSheetFilePt, "  -  Transicoes:  ");
     for (iTransition = 1; iTransition <= dfa_stateCode2totTransitionsFromState (stateCode); iTransition++) {
       symbolCode = dfa_stateCode2transitionSymbol (stateCode, iTransition);
       (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
       (void) STRING_justify ((char **) &symbolString, maxSymbolLength, ' ', STRING_t_justify_left);
       fprintf (answerSheetFilePt, "%s  ", symbolString);
     }
     REPORT_newLine (answerSheetFilePt, 1);
   }
 }
 fflush(NULL);
 REPORT_newLine (answerSheetFilePt, 1);
 fprintf (answerSheetFilePt, "QUADRO D: AFD - Transicoes\n");
 fprintf (answerSheetFilePt, "(%d linhas, %d pontos por linha, total %d pontos)\n", quadro_D_1_LINHAS, QUADRO_D_1_PONTOS_POR_LINHA, quadro_D_1_pontos);
 fprintf (answerSheetFilePt, "   Com terminais\n");
 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   transitions = (int) dfa_symbolCode2totTransitionsWithSymbol (symbolCode);
   (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
   (void) STRING_justify ((char **) &symbolString, maxTerminalLength, ' ', STRING_t_justify_left);
   fprintf (answerSheetFilePt, "       %s  %d\n", symbolString, transitions);
 }
 fprintf (answerSheetFilePt, "   Com nao-terminais\n");
 for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   transitions = (int) dfa_symbolCode2totTransitionsWithSymbol (symbolCode);
   (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
   (void) STRING_justify ((char **) &symbolString, maxTerminalLength, ' ', STRING_t_justify_left);
   fprintf (answerSheetFilePt, "       %s  %d\n", symbolString, transitions);
 }
 fflush(NULL);
 REPORT_newLine (answerSheetFilePt, 1);
 fprintf (answerSheetFilePt, "QUADRO E: Tabela de parse LR(0)\n");
 fprintf (answerSheetFilePt, "(%d linhas, %d pontos por linha, total %d pontos)\n", quadro_E_1_LINHAS, QUADRO_E_1_PONTOS_POR_LINHA, quadro_E_1_pontos);
 fprintf (answerSheetFilePt, "   Terminais\n");
 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   shifts     = (int) parseTableSummaryRow2totParseActions (t_LR0_parser, t_shift,  symbolCode);
   reductions = (int) parseTableSummaryRow2totParseActions (t_LR0_parser, t_reduce, symbolCode);
   (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
   (void) STRING_justify ((char **) &symbolString, maxTerminalLength, ' ', STRING_t_justify_left);
   fprintf (answerSheetFilePt, "       %s - %3d empilhamentos e %3d reducoes\n", symbolString, shifts, reductions);
 }
 fprintf (answerSheetFilePt, "   Não-terminais\n");
 for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   gotos = (int) parseTableSummaryRow2totParseActions (t_LR0_parser, t_goto, symbolCode);
   (void) STRING_copy ((char **) &symbolString, symbolCode2symbolString (symbolCode));
   (void) STRING_justify ((char **) &symbolString, maxTerminalLength, ' ', STRING_t_justify_left);
   fprintf (answerSheetFilePt, "       %s - %3d desvios\n", symbolString, gotos);
 }
 fflush(NULL);
}

/*
*---------------------------------------------------------------------
* Main body of the program
*---------------------------------------------------------------------
*/

int main (int argc, char *argv[]) {
 process_commLine (argc, argv);
 read_grammar (grammarFileName, progName, b_stripoff_quotes);

 if (b_print_symbols)
   print_grammar_data (argc, argv);
 build_LR0_items_NFA_and_DFA();
 
 if (b_set_cfg_file)
   initialize_svg_attributes();
 
 if (b_print_nfa_text)
   print_nfa_txt (argc, argv);
 
 if (b_print_nfa_svg) {
   print_nfa_dot();
   fclose (nfaDotFilePt);
   print_nfa_svg();
   if (! b_automatic_animation)
     print_nfa_lda();
   else
     print_nfa_lda2();
     /* compile_lda(nfaLdaFileName, nfaSvgFileName);*/
 }
 if (b_print_dfa_text)
   print_dfa_txt (argc, argv);
 if (b_print_dfa_svg) {
   print_dfa_dot ();
   fclose (dfaDotFilePt);
   print_dfa_svg ();
   if (!b_automatic_animation)
	   print_dfa_lda();
   else
     print_dfa_lda2();
    /* compile_lda(dfaLdaFileName, dfaSvgFileName);*/
 }
   
 build_first_sets();
 build_follow_sets();
 if (b_print_sets)
   print_sets (argc, argv);

 if (b_print_text_LR0_table || b_write_binary_LR0_table)
   build_LR0_parse_table();
 if (b_print_text_sLR1_table || b_write_binary_sLR1_table)
   build_sLR1_parse_table();
 if (b_print_text_LR0_table || b_print_text_sLR1_table)
   print_text_parse_table_report_header (argc, argv);
 if (b_print_text_LR0_table)
   print_text_parse_table (t_LR0_parse_table);
 if (b_print_text_sLR1_table)
   print_text_parse_table(t_sLR1_parse_table);
 if (b_print_text_LR0_table && b_print_text_sLR1_table) {
   build_diff_parse_table();
   print_text_parse_table (t_diff_parse_table);
 }
 if (b_print_answer_sheet)
   print_answer_sheet (argc, argv);
 if (b_write_binary_LR0_table)
   write_parse_table_binary_file(t_LR0_parse_table);
 if (b_write_binary_sLR1_table)
   write_parse_table_binary_file(t_sLR1_parse_table);

 /* That's all */

 if (DATA_is_module_initialized())
   DATA_destroy_all();
 free_LR0_parse_table_memory();
 free_sLR1_parse_table_memory();
 free_diff_parse_table_memory(); 
 
 if (b_set_cfg_file)
   free_svg_diagrams_data ();
 
 if (grammarFilePt)
   fclose (grammarFilePt);
 if (b_one_output_file)
   fclose (outputFilePt);
 else {
   if (b_print_symbols)
    fclose (symbolsFilePt);
   if (b_print_nfa_text)
    fclose (nfaTextFilePt);
   if (b_print_dfa_text) 
    fclose (dfaTextFilePt);
   if (b_print_sets) 
    fclose (setsFilePt);
   if (b_print_text_LR0_table | b_print_text_sLR1_table) 
    fclose (textTablesFilePt);
   if (b_print_answer_sheet) 
    fclose (answerSheetFilePt);
 }
 if (lr0binaryTableFilePt)
   fclose (lr0binaryTableFilePt);
 if (slr1binaryTableFilePt)
   fclose (slr1binaryTableFilePt);
 return (EXIT_SUCCESS);
}
