/*
*-----------------------------------------------------------------------
*
*   File         : report.c
*   Created      : 1995-08-14
*   Last Modified: 2012-06-10
*
*   Useful routines for producing reports.
*
*-----------------------------------------------------------------------
*/

/*
*-----------------------------------------------------------------------
*   LIBRARIES AND INCLUDE FILES
*-----------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "error.h"
#include "mytime.h"
#include "mystrings.h"
#include "report.h"

/*
*---------------------------------------------------------------------
*
*   INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

/*                     */
/* Function prototypes */
/*                     */

char REPORT_answer_to_question  (char *question, char *accept);
void REPORT_close_table         (FILE *filePt);
void REPORT_open_table          (FILE *fileP);
void REPORT_repeat_char         (FILE *filePt, char character, int times);
void REPORT_reset_header_frame  (void);
void REPORT_newLine             (FILE *filePt, int lines);
void REPORT_write_output_header (FILE *filePt, char *fileName, char *progName, char *versionLabel, char *versionString);

void REPORT_make_multiple_strings (
 char ***strVec,           /* Pointer to array of strings            */
 char   *prefix,           /* Common prefix to strings in the series */
 char   *suffix,           /* Common suffix to strings in the series */
 int     first,            /* First number in the series             */
 int     total,            /* Total numbers in the series            */
 int     digits );         /* Digits of numbers in the series        */

void REPORT_prepare_header (
 char *header,
 int   requested_size,
 STRING_t_justification
       header_pos,
 char **header_top_line,
 char **header_middle_line,
 char **header_blank_line,
 char **header_bottom_line,
 int   output_cols );

void REPORT_print_array (
 FILE *filePt,
 int  *array,
 int   from,
 int   to,
 char *array_name,
 char *print_format,
 int   output_cols );

/*
void REPORT_print_boolean_matrix (
 FILE          *filePt,
 unsigned char *p_matrix,
 int            rows,
 int            cols,
 char          *header,
 char          *rowLabel );
*/

void REPORT_print_integer_table (
 FILE *filePt,               /* Handle to output file                    */
 int  *table,                /* Bi-dimensional array of integers         */
 int   cols,                 /* Number of columns in array               */
 int   from_row,             /* Print from this row...                   */
 int   to_row,               /* ...to this one...                        */
 int   first_row_number,     /* ...numbering them from this value        */
 int   from_col,             /* Print from this column...                */
 int   to_col,               /* ...to this one                           */
 int   first_col_number,     /* ...numbering them from this value        */
 char *table_name,           /* String constant ie. not a print format   */
 char *row_header_string,    /* String constant above table rows         */
 char *row_label_format,     /* Print format for row labels              */
 char *col_label_format,     /* Print format for column labels           */
 int   header_gap,           /* Empty lines between header & table       */
 char *data_label_format,    /* Print format for labels of table entries */
 char *data_format );        /* Print format for table entries           */

void REPORT_print_command_line (
 FILE *filePt,
 char *leadStr,
 int   argc,
 char *argv[] );

void REPORT_set_header_frame (
 char ch_Vertical,
 char ch_Horizontal,
 char ch_TopLeft,
 char ch_TopRight,
 char ch_BottomLeft,
 char ch_BottomRight );

/*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION (not visible from other modules)
*
*---------------------------------------------------------------------
*/

/*                                                  */
/* Characters used to `draw' the border of a header */
/*                                                  */

#define DEFAULT_HEADER_VERTICAL      '|'
#define DEFAULT_HEADER_HORIZONTAL    '-'
#define DEFAULT_HEADER_TOP_LEFT      '+'
#define DEFAULT_HEADER_TOP_RIGHT     '+'
#define DEFAULT_HEADER_BOTTOM_LEFT   '+'
#define DEFAULT_HEADER_BOTTOM_RIGHT  '+'

static char
  header_vertical     = DEFAULT_HEADER_VERTICAL,
  header_horizontal   = DEFAULT_HEADER_HORIZONTAL,
  header_top_left     = DEFAULT_HEADER_TOP_LEFT,
  header_top_right    = DEFAULT_HEADER_TOP_RIGHT,
  header_bottom_left  = DEFAULT_HEADER_BOTTOM_LEFT,
  header_bottom_right = DEFAULT_HEADER_BOTTOM_RIGHT;

/*                                           */
/* Offset of the bit in array pos [row][col] */
/*                                           */

#define bit_OFFSET(totCols,row,col)  ((row)*(totCols) + (col))

/*                                          */
/* For opening and closing numbered tables. */
/* See header file "report.def" for details.  */
/*                                          */

static int
  current_table = 0;

static Bool
  b_table_open = FALSE;

/*
*-----------------------------------------------------------------------
*   Repeatedly write a character to output stream
*-----------------------------------------------------------------------
*/

void REPORT_repeat_char (FILE *filePt, char character, int times)
{
 int
   i;

 for (i = 0; i < times; i++)
   fprintf (filePt, "%c", character);
}

/*
*-----------------------------------------------------------------------
*   Skip lines
*-----------------------------------------------------------------------
*/

void REPORT_newLine (FILE *filePt, int lines)
{
 int
   i;

 for (i = 0; i < lines; i++)
   fprintf (filePt, "\n");
}

/*
*-----------------------------------------------------------------------
*   Build a series of numbered strings.
*   Example:
*     Prefix: "net"
*     Suffix: ".spc"
*     First: 8
*     Total: 3
*     Digits: 3
*   Strings produced:
*     "net008.spc"
*     "net009.spc"
*     "net010.spc"
*   NOTE:
*     If "digits" = 0 then the number of digits will be just big
*     enough for the numbers in the series (ie. no leading 0's
*     if that can be avoided).
*-----------------------------------------------------------------------
*/

#ifdef  INFIX_FORMAT_SIZE
# undef INFIX_FORMAT_SIZE
#endif
#define INFIX_FORMAT_SIZE 5

void REPORT_make_multiple_strings (
 char ***strVec,    /* Pointer to array of strings            */
 char   *prefix,    /* Common prefix to strings in the series */
 char   *suffix,    /* Common suffix to strings in the series */
 int     first,     /* First number in the series             */
 int     total,     /* Total numbers in the series            */
 int     digits )   /* Digits of numbers in the series        */
{
 int
   c_str,
   digitsNeeded,   /* Minimum numbers of digits          */
   last;           /* Last number in the series          */
 size_t
   formatSize,     /* Length of print format for strings */
   strSize;        /* Length of composite strings        */
 char
   infixFormat [INFIX_FORMAT_SIZE],
   *printFormat;

 ERROR_check_range_int (
   (int)    first,
   (int)    0,
   (int)    INT_MAX,
   (char *) "first number",
   (char *) __func__,
   (char *) __FILE__ );
 ERROR_check_range_int (
   (int)    total,
   (int)    1,
   (int)    INT_MAX,
   (char *) "total numbers",
   (char *) __func__,
   (char *) __FILE__ );
 last = first + total - 1;
 digitsNeeded = 1 + (int) log10 ((double) last);
 if (digits == 0)
   digits = digitsNeeded;
 if (digits < digitsNeeded) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Bad number of digits (%d); must be >= %d",
     digits, digitsNeeded );
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 snprintf (infixFormat, INFIX_FORMAT_SIZE, "%%0%dd", digits);
 formatSize = strlen (prefix) + strlen (infixFormat) + strlen (suffix);
 strSize    = strlen (prefix) + digits               + strlen (suffix);
 errno = 0;
 if ((printFormat = (char *) malloc (formatSize + 1)) == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "printFormat");
 snprintf (printFormat, formatSize + 1, "%s%s%s", prefix, infixFormat, suffix);
 errno = 0;
 (*strVec) = (char **) realloc ((*strVec), total * sizeof(char *));
 if ((*strVec) == NULL) {
   ERROR_no_memory (errno, __FILE__, __func__, "*strVec");
 }
 for (c_str = 0; c_str < total; c_str++) {
   errno = 0;
   if (((*strVec)[c_str] = (char *) malloc (strSize + 1)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "(*strVec)[%d]", c_str);
     ERROR_no_memory (errno, __FILE__, __func__, ERROR_auxErrorMsg);
   }
   sprintf ((*strVec)[c_str], printFormat, first + c_str);
 }
 free (printFormat);
}

/*
*-----------------------------------------------------------------------
*   Write the command line to an output file
*-----------------------------------------------------------------------
*/

void REPORT_print_command_line (FILE *filePt, char *leadStr, int argc, char *argv[])
{
 int
   c_arg;

 fprintf (filePt, "%sCOMMAND LINE\n", leadStr);
 fprintf (filePt, "%s------------\n", leadStr);
 fprintf (filePt, "%s%s ", leadStr, argv[0]);
 for (c_arg = 1; c_arg < argc; c_arg++)
   fprintf (filePt, "%s ", argv[c_arg]);
 REPORT_newLine (filePt, 1);
}

/*
*-----------------------------------------------------------------------
*   Reset characters used in the frame of headers to default values
*-----------------------------------------------------------------------
*/

void REPORT_reset_header_frame (void)
{
 header_vertical     = DEFAULT_HEADER_VERTICAL;
 header_horizontal   = DEFAULT_HEADER_HORIZONTAL;
 header_top_left     = DEFAULT_HEADER_TOP_LEFT;
 header_top_right    = DEFAULT_HEADER_TOP_RIGHT;
 header_bottom_left  = DEFAULT_HEADER_BOTTOM_LEFT;
 header_bottom_right = DEFAULT_HEADER_BOTTOM_RIGHT;
}

/*
*-----------------------------------------------------------------------
*   Set characters used in the frame of headers
*-----------------------------------------------------------------------
*/

void REPORT_set_header_frame (
 char ch_Vertical,
 char ch_Horizontal,
 char ch_TopLeft,
 char ch_TopRight,
 char ch_BottomLeft,
 char ch_BottomRight )
{
 header_vertical     = ch_Vertical;
 header_horizontal   = ch_Horizontal;
 header_top_left     = ch_TopLeft;
 header_top_right    = ch_TopRight;
 header_bottom_left  = ch_BottomLeft;
 header_bottom_right = ch_BottomRight;
}

/*
*-----------------------------------------------------------------------
*   Prepare four lines used in simple headers
*-----------------------------------------------------------------------
*/

void REPORT_prepare_header (
 char *header_text,
 int   requested_size,
 STRING_t_justification
       header_pos,
 char **header_top_line,
 char **header_middle_line,
 char **header_blank_line,
 char **header_bottom_line,
 int   output_cols )
{
 int
   leading_blanks       = 0,
   trailing_blanks      = 0,
   total_blanks         = 0,
   real_header_length   = 0,
   usable_header_length = 0;
 static char
   *dashes      = NULL,
   *spaces      = NULL,        /* Eg.                   */
   *top_line    = NULL,        /*       +-----------+   */
   *middle_line = NULL,        /*       | Header    |   */
   *blank_line  = NULL,        /*       |           |   */
   *bottom_line = NULL;        /*       +-----------+   */

 /* Allocate memory for strings */

 dashes      = (char *) STRING_allocate();
 spaces      = (char *) STRING_allocate();
 top_line    = (char *) STRING_allocate();
 middle_line = (char *) STRING_allocate();
 blank_line  = (char *) STRING_allocate();
 bottom_line = (char *) STRING_allocate();

 /* Initialize a few things */

 STRING_extend (&dashes, (unsigned char) header_horizontal, (size_t) output_cols);
 STRING_extend (&spaces, (unsigned char) ' ', (size_t) output_cols);
 real_header_length = strlen (header_text);
 usable_header_length = SMALLEST (real_header_length, requested_size);
 usable_header_length = SMALLEST (usable_header_length, output_cols - 2);
 total_blanks = output_cols - usable_header_length - 2;
 if (total_blanks < 0)
   total_blanks = 0;
 switch (header_pos) {
   case (STRING_t_justify_left):
     leading_blanks  = 0;
     trailing_blanks = total_blanks;
     break;
   case (STRING_t_justify_centre):
     leading_blanks  = (int) (total_blanks / 2);
     trailing_blanks = total_blanks - leading_blanks;
     break;
   case (STRING_t_justify_right):
     leading_blanks  = total_blanks;
     trailing_blanks = 0;
     break;
   default:
     ERROR_fatal_error (0, __FILE__, __func__, "Invalid justification");
 }

 /* Make the strings that go in each header line */

 STRING_extend (&top_line, header_top_left, (size_t) 1);
 strncat (top_line, dashes, usable_header_length + total_blanks);
 STRING_extend (&top_line, header_top_right, (size_t) 1);
 strcat  (top_line, "\n");

 STRING_extend (&middle_line, header_vertical, (size_t) 1);
 strncat (middle_line, spaces, leading_blanks);
 strncat (middle_line, header_text, usable_header_length);
 strncat (middle_line, spaces, trailing_blanks);
 STRING_extend (&middle_line, header_vertical, (size_t) 1);
 strcat  (middle_line, "\n");

 STRING_extend (&blank_line, header_vertical, (size_t) 1);
 strncat (blank_line, spaces, usable_header_length + total_blanks);
 STRING_extend (&blank_line, header_vertical, (size_t) 1);
 strcat  (blank_line, "\n");

 STRING_extend (&bottom_line, header_bottom_left, (size_t) 1);
 strncat (bottom_line, dashes, usable_header_length + total_blanks);
 STRING_extend (&bottom_line, header_bottom_right, (size_t) 1);
 strcat  (bottom_line, "\n");

 /* Release the strings that are no longer needed */

 STRING_release (&dashes);
 STRING_release (&spaces);

 /* Return the remaining strings by reference */

 *header_top_line    = top_line;
 *header_middle_line = middle_line;
 *header_blank_line  = blank_line;
 *header_bottom_line = bottom_line;
}

/*
*---------------------------------------------------------------------
*   Write to output file an array of integers
*---------------------------------------------------------------------
*/

void REPORT_print_array (
 FILE *filePt,
 int  *array,
 int   from,
 int   to,
 char *array_name,
 char *print_format,
 int   output_cols )
{
 int
   c_col,
   incr_col,
   index;
 static char
   *auxStr = NULL;

 /* Allocate memory for aux string */

 errno = 0;
 if ((auxStr = (char *) realloc (auxStr, output_cols + 1)) == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "auxStr");

 snprintf (auxStr, output_cols, print_format, 0);
 incr_col = strlen (auxStr);
 if (array_name) {
   fprintf (filePt, "%s :\n", array_name);
   REPORT_repeat_char (filePt, '-', strlen (array_name));
 }
 REPORT_newLine (filePt,1);
 for (index = from, c_col = 0; index <= to; index++) {
   if ((c_col += incr_col) > output_cols) {
     REPORT_newLine (filePt,1);
     c_col = incr_col;
   }
   fprintf (filePt, print_format, array [index]);
 }
 REPORT_newLine (filePt,3);
 free (auxStr);
}

/*
*---------------------------------------------------------------------
*   Write to output file a table of integers.
*   This procedure differs from REPORT_print_array() in that here
*   the parameter "table" is treated as a bi-demensional array.
*---------------------------------------------------------------------
*/

void REPORT_print_integer_table (
 FILE *filePt,               /* Handle to output file                    */
 int  *table,                /* Bi-dimensional array of integers         */
 int   cols,                 /* Number of columns in array               */
 int   from_row,             /* Print from this row...                   */
 int   to_row,               /* ...to this one...                        */
 int   first_row_number,     /* ...numbering them from this value        */
 int   from_col,             /* Print from this column...                */
 int   to_col,               /* ...to this one                           */
 int   first_col_number,     /* ...numbering them from this value        */
 char *table_name,           /* String constant ie. not a print format   */
 char *row_header_string,    /* String constant above table rows         */
 char *row_label_format,     /* Print format for row labels              */
 char *col_label_format,     /* Print format for column labels           */
 int   header_gap,           /* Empty lines between header & table       */
 char *data_label_format,    /* Print format for labels of table entries */
 char *data_format )         /* Print format for table entries           */
{
 int
   c_col,
   c_row,
   index,
   rowNumber,
   colNumber;
 char
   *auxStr = NULL;

 /* Allocate memory for aux string */

 errno = 0;
 if ((auxStr = (char *) malloc (256)) == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "auxStr");

 snprintf (auxStr, 255, data_format, 0);

 /* Print table name */

 if (table_name) {
   fprintf (filePt, "%s\n", table_name);
   REPORT_newLine (filePt,1);
 }

 /*                    */
 /* Print table header */
 /*                    */

 /* First, the string that goes above table rows... */

 if (row_header_string)
   fprintf (filePt, "%s", row_header_string);

 /* ...then the column labels */

 if (col_label_format) {

   /* Indentation due to row labels */

   if (row_label_format && (! row_header_string)) {
     snprintf (auxStr, 255, row_label_format, 0);
     REPORT_repeat_char (filePt, ' ', strlen (auxStr));
   }

   /* Each column's label */

   if (col_label_format) {
     for (c_col = from_col, colNumber = first_col_number;
            c_col <= to_col;
              c_col++, colNumber++ )
       fprintf (filePt, col_label_format, colNumber);
     REPORT_newLine (filePt,1);
   }
 }

 /* Gap between table header and table contents */

 REPORT_newLine (filePt, header_gap);

 /* Print table contents */

 for (c_row = from_row, rowNumber = first_row_number;
        c_row <= to_row;
          c_row++, rowNumber++ ) {
   if (row_label_format)
     fprintf (filePt, row_label_format, rowNumber);
   for (c_col = from_col, colNumber = first_col_number;
          c_col <= to_col;
            c_col++, colNumber++ ) {
     if (data_label_format)
       fprintf (filePt, data_label_format, colNumber);
     index = c_row * cols + c_col;
     fprintf (filePt, data_format, table[index]);
   }
   REPORT_newLine (filePt,1);
 }
 REPORT_newLine (filePt,1);
 free (auxStr);
}

/*
*---------------------------------------------------------------------
*   Write to output file a matrix of booleans
*---------------------------------------------------------------------


void REPORT_print_boolean_matrix (
 FILE          *filePt,
 unsigned char *p_matrix,
 int            rows,
 int            cols,
 char          *header,
 char          *rowLabel )
{
 int
   c_dash,
   c_row,
   c_col;

 REPORT_newLine (filePt,1);
 fprintf (filePt, "%s:\n", header);
 for (c_dash = 0; c_dash < (int) strlen (header); c_dash++)
   fprintf (filePt, "-");
 REPORT_newLine (filePt,1);
 for (c_row = 0; c_row < rows; c_row++) {
   fprintf (filePt, rowLabel, c_row+1);
   for (c_col = 0; c_col < cols; c_col++)
     fprintf (filePt, "%1d ",
       get_bit (p_matrix, bit_OFFSET(cols,c_row,c_col)));
   REPORT_newLine (filePt,1);
 }
}
*/

/*
*---------------------------------------------------------------------
*   Write to output file a simple header containing
*   - the name of the output file
*   - the name and version of the calling program
*   - the current time & date
*---------------------------------------------------------------------
*/

void REPORT_write_output_header (FILE *filePt, char *fileName, char *progName, char *versionLabel, char *versionString)
{
 int
   l1, l2, l3,
   longest,
   blanks,
   dashes,
   c_blank,
   c_dash;
 char
   *timeStr;

 l1 = strlen (progName) + 1 + strlen (versionLabel) + strlen (versionString);  /* Ex: "geraLR v.22" */
 timeStr = TIME_getCurrentTimeStr();
 l2 = strlen (timeStr);
 l3 = strlen (fileName);
 longest = GREATEST (l1, l2);
 longest = GREATEST (l3, longest);
 dashes = longest + 16;

 REPORT_newLine (filePt, 1);
 fprintf (filePt, "%c", header_top_left);
 for (c_dash = 0; c_dash < dashes; c_dash++)
   fprintf (filePt, "%c", header_horizontal);
 fprintf (filePt, "%c\n", header_top_right);

 fprintf (filePt, "%c  File name : %s", header_vertical, fileName);
 blanks = longest - l3;
 for (c_blank = 0; c_blank < blanks; c_blank++)
   fprintf (filePt, " ");
 fprintf (filePt, "  %c\n", header_vertical);

 fprintf (filePt, "%c  Created by: %s %s%s", header_vertical, progName, versionLabel, versionString);
 blanks = longest - l1;
 for (c_blank = 0; c_blank < blanks; c_blank++)
   fprintf (filePt, " ");
 fprintf (filePt, "  %c\n", header_vertical);

 fprintf (filePt, "%c  Date/Time : %s", header_vertical, timeStr);
 blanks = longest - l2;
 for (c_blank = 0; c_blank < blanks; c_blank++)
   fprintf (filePt, " ");
 fprintf (filePt, "  %c\n", header_vertical);

 fprintf (filePt, "%c", header_bottom_left);
 for (c_dash = 0; c_dash < dashes; c_dash++)
   fprintf (filePt, "%c", header_horizontal);
 fprintf (filePt, "%c\n", header_bottom_right);

 fflush (NULL);
}

/*
*---------------------------------------------------------------------
*   Write to output file a string immediately followed by a table
*---------------------------------------------------------------------
*/

void REPORT_open_table (FILE *filePt)
{
 if (b_table_open) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Close table %d first", current_table);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 REPORT_newLine (filePt, 1);
 fprintf (filePt, REPORT_OPENING_TABLE_FORMAT, ++current_table);
 REPORT_newLine (filePt, 1);
 fflush (NULL);
 b_table_open = TRUE;
}

/*
*---------------------------------------------------------------------
*   Write to output file a string immediately following a table
*---------------------------------------------------------------------
*/

void REPORT_close_table (FILE *filePt)
{
 if (! b_table_open)
   ERROR_fatal_error (0, __FILE__, __func__, "Open table first");
 fprintf (filePt, REPORT_CLOSING_TABLE_FORMAT, current_table);
 REPORT_newLine (filePt, 1);
 fflush (NULL);
 b_table_open = FALSE;
}

/*
*---------------------------------------------------------------------
*   Ask the user a question and return the answer
*---------------------------------------------------------------------
*/

char REPORT_answer_to_question (char *question, char *accept)
{
  int
    c_char,
    lastPos;
  char
    answer;
  Bool
    answerOK;

  /* Ask the question */

  printf ("%s", question);

  /* Print acceptable characters */

  printf (" [");
  lastPos = strlen (accept) - 1;
  for (c_char = 0; c_char < lastPos; c_char++)
    printf ("%c ", accept [c_char]);
  printf ("%c] ? \n", accept [lastPos]);

  /* Get an answer */

  do {
    answer = (char) fgetc (stdin);
    fflush (NULL);
    answer = tolower (answer);
    answerOK = (strchr (accept, answer) ? TRUE : FALSE);
   }
    while (! answerOK);
  return (answer);
}
