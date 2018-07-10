/*
*-----------------------------------------------------------------------
*
*   File         : geraLR.h
*   Created      : 2009-03-03
*   Last Modified: 2018-04-16
*
*
*   DESCRIPTION:
*
*   This program reads a text file containing a context free grammar
*   and generates a LR(0) or sLR(1) parse table, along with the
*   corresponding collection of canonical LR(0) items, DFA states
*   and state transitions, and FIRST and FOLLOW sets.
*
*   This is the last Compiler course assignment.
*   Support code is provided, to be completed by each student.
*
*   USAGE:
*   geraLR -h for help
*
*-----------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _GERA_LR_DOT_H_
#define _GERA_LR_DOT_H_

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

/*
*-----------------------------------------------------------------------
* Program version
*-----------------------------------------------------------------------
*/

#define VERSION_LABEL    "v."
#define VERSION_STRING   "graphviz-Eberty"

/*
*-----------------------------------------------------------------------
* Definitions for I/O operations
*-----------------------------------------------------------------------
*/

#define FILE_NAME_SIZE          255
#define OPEN_OUTPUT_COMMENT     "/*"
#define CLOSE_OUTPUT_COMMENT    "*/"

/*                        */
/* Output file extensions */
/*                        */

#define FILE_EXTENSION_SYMBOLS            ".sym"

#define FILE_EXTENSION_NFA_TEXT           "-NFA.txt"
#define FILE_EXTENSION_NFA_DOT            "-NFA.dot"
#define FILE_EXTENSION_NFA_SVG            "-NFA.svg"
#define FILE_EXTENSION_NFA_LDA            "-NFA.lda"

#define FILE_EXTENSION_DFA_TEXT           "-DFA.txt"
#define FILE_EXTENSION_DFA_DOT            "-DFA.dot"
#define FILE_EXTENSION_DFA_SVG            "-DFA.svg"
#define FILE_EXTENSION_DFA_LDA            "-DFA.lda"

#define FILE_EXTENSION_SETS               ".set"
#define FILE_EXTENSION_ANSWER_SHEET       ".gab"
#define FILE_EXTENSION_BINARY_LR0_TABLE   "-LR0.tbl"
#define FILE_EXTENSION_BINARY_SLR1_TABLE  "-sLR1.tbl"
#define FILE_EXTENSION_TEXT_TABLES        "-TXT.tbl"
#define FILE_EXTENSION_ONE_OUTPUT         ".out"

/*                        */
/* Program and file names */
/*                        */

extern char
  *progName,
  grammarFileName         [FILE_NAME_SIZE],
  symbolsFileName         [FILE_NAME_SIZE],
  nfaTextFileName         [FILE_NAME_SIZE],
  nfaDotFileName          [FILE_NAME_SIZE],
  nfaSvgFileName          [FILE_NAME_SIZE],
  nfaLdaFileName          [FILE_NAME_SIZE],
  dfaTextFileName         [FILE_NAME_SIZE],
  dfaDotFileName          [FILE_NAME_SIZE],
  dfaSvgFileName          [FILE_NAME_SIZE],
  dfaLdaFileName          [FILE_NAME_SIZE],
  configFileName          [FILE_NAME_SIZE],
  setsFileName            [FILE_NAME_SIZE],
  lr0binaryTableFileName  [FILE_NAME_SIZE],
  slr1binaryTableFileName [FILE_NAME_SIZE],
  textTablesFileName      [FILE_NAME_SIZE],
  answerSheetFileName     [FILE_NAME_SIZE],
  oneOutputFileName       [FILE_NAME_SIZE];

/*              */
/* File handles */
/*              */

extern FILE
  *grammarFilePt,
  *symbolsFilePt,
  *nfaTextFilePt,
  *nfaDotFilePt,
  *nfaSvgFilePt,
  *nfaLdaFilePt,
  *dfaTextFilePt,
  *dfaDotFilePt,
  *dfaSvgFilePt,
  *dfaLdaFilePt,
  *setsFilePt,
  *answerSheetFilePt,
  *lr0binaryTableFilePt,
  *slr1binaryTableFilePt,
  *textTablesFilePt,
  *outputFilePt;

extern bool
  b_one_output_file,
  b_output_started;

/*
*---------------------------------------------------------------------
*   Type definitions
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
* Function prototypes
*---------------------------------------------------------------------
*/

extern void print_output_header (FILE *filePt, char *fileName, int argc, char *argv[]);
extern void print_grammar_rules (FILE *filePt);

#endif /* ifndef _GERA_LR_DOT_H_ */
