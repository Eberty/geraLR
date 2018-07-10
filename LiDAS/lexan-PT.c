/*
*---------------------------------------------------------------------
*
*   File         : lexan-PT.c
*   Created      : 1996-03-26
*   Last Modified: 2012-05-30
*
*   Simple lexical analyser - MENSAGENS DE ERRO EM PORTUGUES
*
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
*  INCLUDE FILES
*---------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "error.h"
#include "mystrings.h"
#include "lexan-PT.h"

/*
*---------------------------------------------------------------------
*
*  INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

char
  LEXAN_tokenTypeNames [12][26] = {
    {"LEXAN_token_single_str"},
    {"LEXAN_token_double_str"},
    {"LEXAN_token_iden"},
    {"LEXAN_token_resWord"},
    {"LEXAN_token_longInt"},
    {"LEXAN_token_double"},
    {"LEXAN_token_delim"},
    {"LEXAN_token_EOF"},
    {"LEXAN_token_NULL"},
    {"LEXAN_token_LINE_COMMENT"},
    {"LEXAN_token_OPEN_COMMENT"},
    {"LEXAN_token_CLOSE_COMMENT"}
  };

/*                     */
/* Function prototypes */
/*                     */

LEXAN_t_lexJobId LEXAN_start_job (
  char              *fileName,
  reg_syntax_t       regexSyntax,
  LEXAN_t_tokenCase  tokenCase,
  int                totTokenSpecs,
  LEXAN_t_tokenSpecs *tokenSpecs );

Bool LEXAN_get_token (
  LEXAN_t_lexJobId   jobId,
  LEXAN_t_tokenType *p_token,
  LEXAN_t_tokenVal  *p_tokenVal );

Bool LEXAN_get_subtoken (
  LEXAN_t_lexJobId  jobId,
  LEXAN_t_tokenType subToken,
  int               subGroupStart,
  int               subGroupEnd,
  LEXAN_t_tokenVal *p_subTokenVal );

LEXAN_t_tokenLocation  LEXAN_get_token_location (LEXAN_t_lexJobId jobId);
char                  *LEXAN_get_token_context  (LEXAN_t_lexJobId jobId);
char                  *LEXAN_get_token_regex    (LEXAN_t_lexJobId jobId);
char                  *LEXAN_get_error_descr    (LEXAN_t_lexJobId jobId);
int                    LEXAN_get_error_count    (LEXAN_t_lexJobId jobId);
Bool                   LEXAN_terminate_job      (LEXAN_t_lexJobId jobId);
Bool                   LEXAN_terminate_all_jobs (void);

/*
*---------------------------------------------------------------------
*
*  IMPLEMENTATION (not visible from other modules)
*
*---------------------------------------------------------------------
*/

/*                 */
/* A few constants */
/*                 */

#define c_EOF             0xFF     /* Char indicating end of file */
#define MAX_LINE_SIZE     1024     /* Text read from input files  */

/*                                                         */
/* Formal parameters of tokens & their regular expressions */
/*                                                         */

typedef struct {
  char                     *tk_regexStr;
  struct re_pattern_buffer  tk_regexBuffer;
  struct re_registers       tk_regexRegs;
  LEXAN_t_tokenType         tk_type;
  int                       tk_matchSize;
  int                       tk_firstGroup;
  int                       tk_lastGroup;
  int                       tk_tokenSize;
}
  t_tokenInfo;

/*                            */
/* Information on active jobs */
/*                            */

typedef struct {
 LEXAN_t_lexJobId   jobId;              /* Unique job identifier             */
 LEXAN_t_tokenCase  tokenCase;          /* Case transformation of input file */
 int                (* convProc)(int);  /* Ptr to case conversion procedure  */
 int                formalTokens;       /* Number of tokens to be recognized */
 t_tokenInfo       *tokenInfoVec;       /* Array of formal token parameters  */
 char              *fileName;           /* Name of file to be scanned        */
 FILE              *filePt;             /* File handle                       */
 char              *currLine;           /* Last line read from input file    */
 int                lineSize;           /* Its size                          */
 int                lineNum;            /* Its number                        */
 int                currChar;           /* Current char in currLine          */
 int                charPos;            /* Its position in currLine          */
 LEXAN_t_tokenType  currToken;          /* Current token                     */
 int                tokenIndex;         /* Its index in tokenInfoVec         */
 LEXAN_t_tokenVal   tokenVal;           /* Its value                         */
 Bool               b_bufFull;          /* Whether a char has been read      */
 Bool               b_midComment;       /* Whether in a multi-line comment   */
 int                openCommLineNum;    /* Line where comment was opened     */
 int                totErrors;          /* Number of lexical errors so far   */
 char              *lastError;          /* Descr of last error occurred      */
}
 t_jobInfo;

/*                                                */
/* Several files can be scanned simultaneously,   */
/* each one corresponding to a different "job".   */
/* Job info is kept in a dynamically allocated    */
/* array. This array grows when a new job is      */
/* created, and shrinks when a job is terminated. */
/*                                                */

static t_jobInfo
  *jobInfoVec = NULL;

static int
  activeJobs = 0,      /* Total jobs currently active */
  lastJobNum = 0;      /* Number of last job started  */

/*                                                                 */
/* Description of errors that don't apply to any job in particular */
/*                                                                 */

static char
  globalErrorDescr [ERROR_maxErrorMsgSize],
  *tokenStr       = NULL,
  *globalCurrLine = NULL;

/*                          */
/* Has it been initialized? */
/*                          */

Bool
  b_initialized_lexan = FALSE;

/*
*---------------------------------------------------------------------
*   Function prototypes
*---------------------------------------------------------------------
*/

static Bool       consume_input    (LEXAN_t_lexJobId jobId, int amount);
static t_jobInfo *get_job_info     (LEXAN_t_lexJobId jobId);
static void       initialize_lexan (void);
static char      *markErrorPos     (int errorPos);
static Bool       read_char        (LEXAN_t_lexJobId jobId);
static Bool       read_non_blank   (LEXAN_t_lexJobId jobId);

/*
*---------------------------------------------------------------------
*  Make sure a given job exists and return a
*  pointer to its entry in the "jobInfo" array.
*  Issue an error message otherwise.
*---------------------------------------------------------------------
*/

static t_jobInfo *get_job_info (LEXAN_t_lexJobId jobId)
{
 int
   c_jobIndex;
 t_jobInfo
   *p_jobInfo;

 for (c_jobIndex = 0, p_jobInfo = jobInfoVec;
        c_jobIndex < activeJobs;
          c_jobIndex++, p_jobInfo++ )
   if (p_jobInfo->jobId == jobId)
     break;

 /* Is there such a job running? */

 if (c_jobIndex == activeJobs) {
   snprintf (
     globalErrorDescr,
     ERROR_maxErrorMsgSize,
     "%s.c::get_job_info(%d): Invalid job identifier",
     __FILE__,
     jobId );
   return ((t_jobInfo *) NULL);
 }

 /* The job is active. Return a pointer to job's info */

 strcpy (globalErrorDescr, "");
 return (p_jobInfo);
}

/*
*---------------------------------------------------------------------
*  Create a string of "-" pointing to an error
*---------------------------------------------------------------------
*/

static char *markErrorPos (int errorPos)
{
 static char
   line [MAX_LINE_SIZE];

 memset (line, 0, (size_t) MAX_LINE_SIZE);
 memset (line, ' ', (size_t) 7);
 memset (&(line[7]), '-', (size_t) errorPos);
 line [errorPos+7] = '^';
 line [errorPos+8] = '\n';
 return (line);
}

/*
*---------------------------------------------------------------------
*  Read a character from the specified file
*---------------------------------------------------------------------
*/

static Bool read_char (LEXAN_t_lexJobId jobId)
{
 int
   c_char;
 char
   line [MAX_LINE_SIZE];
 char
   *ptr;
 t_jobInfo
   *p_jobInfo;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);
 if (p_jobInfo == NULL)
   return (FALSE);

 /* Have we reached this file's EOF? */

 if (p_jobInfo->currChar == c_EOF)
   return (TRUE);

 /* Read a new line if necessary. */

 if (p_jobInfo->charPos >= p_jobInfo->lineSize - 1) {
   memset (line, 0, MAX_LINE_SIZE);
   ptr = fgets (line, MAX_LINE_SIZE, p_jobInfo->filePt);
   if (ptr == NULL) {
     p_jobInfo->currChar = c_EOF;
     if (! feof (p_jobInfo->filePt)) {
       snprintf (
         p_jobInfo->lastError,
	 ERROR_maxErrorMsgSize,
         "%s::read_char(%d): Erro de leitura. Inserindo EOF em \"%s\" ",
         __FILE__,
         jobId,
         p_jobInfo->fileName );
       p_jobInfo->totErrors++;
       p_jobInfo->currToken = LEXAN_token_NULL;
       return (FALSE);
     }
     else
       return (TRUE);
   }

   memset (p_jobInfo->currLine, 0, MAX_LINE_SIZE);
   strcpy (p_jobInfo->currLine, line);
   p_jobInfo->lineSize = (int) strlen (line);
   p_jobInfo->lineNum++;
   p_jobInfo->charPos = -1;

   /* Change case of newly read characters if necessary */

   if (p_jobInfo->tokenCase != LEXAN_t_keepCase)
     for (c_char = 0; c_char < p_jobInfo->lineSize; c_char++)
       p_jobInfo->currLine[c_char] = (char)
         ((* p_jobInfo->convProc)((int) p_jobInfo->currLine[c_char]));
 }

 /* Update current char */

 p_jobInfo->currChar = p_jobInfo->currLine[++(p_jobInfo->charPos)];
 return (TRUE);
}

/*
*---------------------------------------------------------------------
*  Read one character skipping blanks (spaces, tabs and newlines)
*---------------------------------------------------------------------
*/

static Bool read_non_blank (LEXAN_t_lexJobId jobId)
{
 t_jobInfo
   *p_jobInfo;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);
 if (p_jobInfo == NULL)
   return (FALSE);

 /* Read one character */

 if (! p_jobInfo->b_bufFull) {
   if (! read_char (jobId))
     return (FALSE);
 }

 /* Keep reading until we find a non-blank */

 while ( (p_jobInfo->currChar == ' ' )  ||
         (p_jobInfo->currChar == '\t')  ||
         (p_jobInfo->currChar == '\f')  ||
         (p_jobInfo->currChar == '\n')  ||
         (p_jobInfo->currChar == '\r') ) {
   if (! read_char (jobId))
     return (FALSE);
 }
 p_jobInfo->b_bufFull = FALSE;
 return (TRUE);
}

/*
*---------------------------------------------------------------------
*  Eat up characters from input buffer
*---------------------------------------------------------------------
*/

static Bool consume_input (LEXAN_t_lexJobId jobId, int amount)
{
 int
   c_char;
 t_jobInfo
   *p_jobInfo;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);
 if (p_jobInfo == NULL)
   return (FALSE);

 /* Consume "amount" characters from input buffer */

 for (c_char = 0; c_char < amount; c_char++)
   if (! read_char (jobId))
     return (FALSE);
 p_jobInfo->b_bufFull = TRUE;
 return (TRUE);
}

/*
*---------------------------------------------------------------------
*  Initialize the lexical analyser
*---------------------------------------------------------------------
*/

static void initialize_lexan (void)
{
 /* Include any initializations here */

 globalCurrLine = STRING_allocate();

 b_initialized_lexan = TRUE;
}

/*
*---------------------------------------------------------------------
*  Start a new job
*---------------------------------------------------------------------
*/

LEXAN_t_lexJobId LEXAN_start_job (
 char              *fileName,
 reg_syntax_t       regexSyntax,
 LEXAN_t_tokenCase  tokenCase,
 int                totTokenSpecs,
 LEXAN_t_tokenSpecs *tokenSpecs )
{
 int
   c_token;
 const char
   *compResult;
 char
   *line,
   *fastmap;
 Bool
   b_openCOMMENT_used,
   b_closeCOMMENT_used;
 size_t
   nbytes;
 FILE
   *filePt;
 t_jobInfo
   *p_jobInfo;
 LEXAN_t_lexJobId
   jobId;
 t_tokenInfo
   *p_tokenInfo;
 LEXAN_t_tokenSpecs
   *p_tokenSpecs;

 /* Make sure the lexical analyser has been initialized */

 if (! b_initialized_lexan)
   initialize_lexan();

 /* Open the input file for reading */

 errno = 0;
 filePt = fopen (fileName, "r");
 if (filePt == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro ao abrir \"%s\" para leitura", fileName);
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Allocate memory for this job's specs */

 activeJobs++;
 nbytes = activeJobs * sizeof (t_jobInfo);
 errno = 0;
 jobInfoVec = (t_jobInfo *) realloc (jobInfoVec, nbytes);
 if (jobInfoVec == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "jobInfoVec");

 /*                             */
 /* Initialize this job's specs */
 /*                             */

 p_jobInfo = (t_jobInfo *) (jobInfoVec + activeJobs - 1);

 /* Job identifier */

 p_jobInfo->jobId = jobId = ++lastJobNum;

 /* Case sensitivity of scanner */

 p_jobInfo->tokenCase = tokenCase;
 switch (tokenCase) {
   case (LEXAN_t_upperCase) : p_jobInfo->convProc = toupper; break;
   case (LEXAN_t_lowerCase) : p_jobInfo->convProc = tolower; break;
   case (LEXAN_t_keepCase)  : p_jobInfo->convProc = NULL;    break;
   default:
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "tokenCase %d invalido", tokenCase);
     ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Number of formal tokens */

 p_jobInfo->formalTokens = totTokenSpecs;

 /* Allocate memory for the formal definition of tokens */

 nbytes = p_jobInfo->formalTokens * sizeof (t_tokenInfo);
 errno = 0;
 p_tokenInfo = (t_tokenInfo *) malloc (nbytes);
 if (p_tokenInfo == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "jobInfoVec[%d].tokenInfoVec", activeJobs - 1);
   ERROR_no_memory (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 p_jobInfo->tokenInfoVec = p_tokenInfo;

 /* Process the formal definition of tokens */

 b_openCOMMENT_used = b_closeCOMMENT_used = FALSE;

 for (c_token = 0,
      p_tokenInfo  = p_jobInfo->tokenInfoVec,
      p_tokenSpecs = tokenSpecs;
        c_token < p_jobInfo->formalTokens;
          c_token++, p_tokenInfo++, p_tokenSpecs++) {

   /* If LEXAN_token_OPEN_COMMENT or LEXAN_token_CLOSE_COMMENT, */
   /* make sure it is used only once              */

   if (p_tokenSpecs->type == LEXAN_token_OPEN_COMMENT) {
     if (b_openCOMMENT_used)
       ERROR_fatal_error (0, __FILE__, __func__,
         "LEXAN_token_OPEN_COMMENT so pode ser usado uma vez" );
     b_openCOMMENT_used = TRUE;
   }

   if (p_tokenSpecs->type == LEXAN_token_CLOSE_COMMENT) {
     if (b_closeCOMMENT_used)
       ERROR_fatal_error (0, __FILE__, __func__,
         "LEXAN_token_CLOSE_COMMENT so pode ser usado uma vez" );
     b_closeCOMMENT_used = TRUE;
   }

   /* Token type and regular expression */

   errno = 0;
   p_tokenInfo->tk_regexStr = strdup (p_tokenSpecs->regex);
   if (p_tokenInfo->tk_regexStr == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em tokenInfo[%d].tk_regexStr=strdup(\"%s\")",
       c_token, p_tokenSpecs->regex );
     ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
   }
   p_tokenInfo->tk_type       = p_tokenSpecs->type;
   p_tokenInfo->tk_matchSize  = 0;
   p_tokenInfo->tk_firstGroup = p_tokenSpecs->firstGroup;
   p_tokenInfo->tk_lastGroup  = p_tokenSpecs->lastGroup;

   /* Clear regex pattern buffer and registers */

   memset (
     (void *) &(p_tokenInfo->tk_regexBuffer),
     (int)    0,
     (size_t) sizeof (struct re_pattern_buffer) );
   memset (
     (void *) &(p_tokenInfo->tk_regexRegs),
     (int)    0,
     (size_t) sizeof (struct re_registers) );

   /* Initialize regex pattern buffer */

   p_tokenInfo->tk_regexBuffer.buffer    = (unsigned char *)   NULL;
   p_tokenInfo->tk_regexBuffer.allocated = (unsigned long int) 0;
   p_tokenInfo->tk_regexBuffer.used      = (unsigned long int) 0;
   p_tokenInfo->tk_regexBuffer.syntax    = (reg_syntax_t)      regexSyntax;
   p_tokenInfo->tk_regexBuffer.fastmap   = (char *)            NULL;
   p_tokenInfo->tk_regexBuffer.translate = (unsigned char *)   NULL;
   p_tokenInfo->tk_regexBuffer.no_sub    = (unsigned int)      0;

   /* The regular expression of a LEXAN_token_CLOSE_COMMENT will */
   /* use a fastmap to speed up the matching process      */

   if (p_tokenSpecs->type == LEXAN_token_CLOSE_COMMENT) {
     errno = 0;
     fastmap = (char *) malloc (256);
     if (fastmap == NULL) {
       snprintf (
         ERROR_auxErrorMsg,
	       ERROR_maxErrorMsgSize,
         "jobInfoVec[%d].tokenInfoVec[%d].fastmap",
         activeJobs - 1,
         c_token );
       ERROR_no_memory (errno, __FILE__, __func__, ERROR_auxErrorMsg);
     }
     p_tokenInfo->tk_regexBuffer.fastmap = fastmap;
   }

   /* Set regular expression syntax options */

   re_syntax_options = regexSyntax;

   /* Compile regular expression */

   compResult = re_compile_pattern (
     p_tokenInfo->tk_regexStr,
     (int) strlen (p_tokenInfo->tk_regexStr),
     &(p_tokenInfo->tk_regexBuffer) );
   if (compResult) {
     snprintf (
       ERROR_auxErrorMsg,
       ERROR_maxErrorMsgSize,
       "Erro na compilacao de expressao regular (job #%d, token #%d): %s",
       activeJobs - 1,
       c_token,
       compResult );
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }

 /* If LEXAN_token_OPEN_COMMENT was used, make sure   */
 /* so was LEXAN_token_CLOSE_COMMENT (and vice-versa) */

 if (b_openCOMMENT_used && (! b_closeCOMMENT_used))
   ERROR_fatal_error (0, __FILE__, __func__, "LEXAN_token_CLOSE_COMMENT faltando");

 if (b_closeCOMMENT_used && (! b_openCOMMENT_used))
   ERROR_fatal_error (0, __FILE__, __func__, "LEXAN_token_OPEN_COMMENT faltando");

 /* Input file name, file handle, text line etc */

 errno = 0;
 p_jobInfo->fileName = strdup (fileName);
 if (p_jobInfo->fileName == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em jobInfo[%d].fileName = strdup (\"%s\")",
     activeJobs - 1,
     fileName );
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 p_jobInfo->filePt = filePt;
 nbytes = MAX_LINE_SIZE;
 errno = 0;
 p_jobInfo->currLine = (char *) malloc (nbytes);
 if (p_jobInfo->currLine == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "jobInfoVec[%d].currLine", activeJobs - 1);
   ERROR_no_memory (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 memset (p_jobInfo->currLine, 0, nbytes);
 p_jobInfo->lineSize        = 0;
 p_jobInfo->lineNum         = 0;
 p_jobInfo->currChar        = 0;
 p_jobInfo->charPos         = MAX_LINE_SIZE;
 p_jobInfo->currToken       = LEXAN_token_NULL;
 p_jobInfo->tokenIndex      = -1;
 memset (&(p_jobInfo->tokenVal), 0, sizeof (LEXAN_t_tokenVal));
 p_jobInfo->b_bufFull       = FALSE;
 p_jobInfo->b_midComment    = FALSE;
 p_jobInfo->openCommLineNum = 0;
 p_jobInfo->totErrors       = 0;
 nbytes = ERROR_maxErrorMsgSize;
 errno = 0;
 line = (char *) malloc (nbytes);
 if (line == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "jobInfoVec[%d].lastError", activeJobs - 1);
   ERROR_no_memory (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 memset (line, 0, nbytes);
 p_jobInfo->lastError = line;

 /* Return the new job's id */

 return (jobId);
}

/*
*---------------------------------------------------------------------
*  Run a job (ie. get a token from a file)
*---------------------------------------------------------------------
*/

Bool LEXAN_get_token (
 LEXAN_t_lexJobId   jobId,
 LEXAN_t_tokenType *p_token,
 LEXAN_t_tokenVal  *p_tokenVal )
{
 int
   c_token,
   commentSize,
   extendedSize,
   longestToken,
   matchSize,
   searchRange,
   tokenIndex,
   tokenStart,
   tokenSize;
 Bool
   b_convOK,
   b_foundMatch,
   b_consumedOK,
   b_nextScanOK;
 t_jobInfo
   *p_jobInfo;
 t_tokenInfo
   *p_tokenInfo;

 /* Clear return values */

 memset (p_tokenVal, 0, sizeof (LEXAN_t_tokenVal));
 *p_token = LEXAN_token_NULL;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);
 if (p_jobInfo == NULL)
   return (FALSE);

 /* Clear token value */

 p_jobInfo->currToken  = LEXAN_token_NULL;
 p_jobInfo->tokenIndex = -1;
 free ((char *) p_jobInfo->tokenVal.tokenStr);
 memset (&(p_jobInfo->tokenVal), 0, sizeof (LEXAN_t_tokenVal));

 /* Skip blanks */

 if (! read_non_blank (jobId))
   return (FALSE);

 /* Check for end of file */

 if (p_jobInfo->currChar == c_EOF)
   *p_token = p_jobInfo->currToken = LEXAN_token_EOF;

 /*                                  */
 /* Processing a multi-line comment? */
 /*                                  */

 if (p_jobInfo->b_midComment) {

   /* Error if end-of-file before comment was closed */

   if (p_jobInfo->currChar == c_EOF) {
     p_jobInfo->totErrors++;
     snprintf (
       p_jobInfo->lastError,
       ERROR_maxErrorMsgSize,
       "%s(): Comentario aberto e nao fechado na linha %d de \"%s\"",
       __func__,
       p_jobInfo->openCommLineNum,
       p_jobInfo->fileName );
     return (FALSE);
   }

   /* Search for close-comment in input text buffer */

   for (c_token = 0, p_tokenInfo = p_jobInfo->tokenInfoVec;
          c_token < p_jobInfo->formalTokens;
            c_token++, p_tokenInfo++ )
     if (p_tokenInfo->tk_type == LEXAN_token_CLOSE_COMMENT)
       break;

   if (c_token == p_jobInfo->formalTokens)
     ERROR_fatal_error (
       0,
       __FILE__,
       __func__,
       "Panico! LEXAN_token_CLOSE_COMMENT nao encontrado");

   searchRange = p_jobInfo->lineSize - p_jobInfo->charPos - 1;
   matchSize = re_search (
     &p_tokenInfo->tk_regexBuffer,    /* The pattern buffer               */
     p_jobInfo->currLine,             /* The current line                 */
     p_jobInfo->lineSize,             /* Size of current line             */
     p_jobInfo->charPos,              /* Starting position for the search */
     searchRange,                     /* Range of search                  */
     &p_tokenInfo->tk_regexRegs );    /* The registers                    */

   switch (matchSize) {

     /* re_search() internal error */

     case (-2):
       p_jobInfo->totErrors++;
       snprintf (
         p_jobInfo->lastError,
         ERROR_maxErrorMsgSize,
         "%s(): Erro interno em re_search(). Pulando linha %d de \"%s\"",
         __func__,
         p_jobInfo->openCommLineNum,
         p_jobInfo->fileName );
       (void) consume_input (jobId, searchRange + 1);
       return (FALSE);

     /* No match: the end-of-comment was not found. */
     /* In this case we have to...                  */
     /* - ignore (ie. skip) the rest of the line    */
     /* - scan the text once again                  */

     case (-1):
       b_consumedOK = consume_input (jobId, searchRange + 1);
       if (! b_consumedOK) {
         p_jobInfo->totErrors++;
         snprintf (
           p_jobInfo->lastError,
           ERROR_maxErrorMsgSize,
           "%s(): Erro ao processar comentarios em \"%s\"\n[%4d] %s%s",
           __func__,
           p_jobInfo->fileName,
           p_jobInfo->lineNum,
           p_jobInfo->currLine,
           markErrorPos (p_jobInfo->charPos) );
       }
       b_nextScanOK = LEXAN_get_token (jobId, p_token, p_tokenVal);
       return (b_consumedOK && b_nextScanOK);

     /* Match: the end-of-comment was found.                   */
     /* To close the comment we have to...                     */
     /* - set the "processing comment" boolean to FALSE        */
     /* - skip everything up to & including the end-of-comment */
     /* - scan the text once again                             */

     default:
       p_jobInfo->b_midComment = FALSE;
       p_jobInfo->openCommLineNum = 0;
       extendedSize = p_tokenInfo->tk_regexRegs.end[0] - p_jobInfo->charPos;
       b_consumedOK = consume_input (jobId, extendedSize);
       if (! b_consumedOK) {
         p_jobInfo->totErrors++;
         snprintf (
           p_jobInfo->lastError,
           ERROR_maxErrorMsgSize,
           "%s(): Erro ao processar marcador de fim de comentario de bloco em \"%s\"\n[%4d] %s%s",
           __func__,
           p_jobInfo->fileName,
           p_jobInfo->lineNum,
           p_jobInfo->currLine,
           markErrorPos (p_jobInfo->charPos) );
       }
       b_nextScanOK = LEXAN_get_token (jobId, p_token, p_tokenVal);
       return (b_consumedOK && b_nextScanOK);
   }
 }

 /*                                      */
 /* Not processing a multi-line comment  */
 /*                                      */

 /* Check for end of file */

 if (p_jobInfo->currChar == c_EOF) {
   *p_token = p_jobInfo->currToken = LEXAN_token_EOF;
   return (TRUE);
 }

 /*                                       */
 /* Try matching all regular expressions. */
 /*                                       */

 b_foundMatch = FALSE;
 longestToken = INT_MIN;
 tokenIndex   = -1;

 for (c_token = 0, p_tokenInfo = p_jobInfo->tokenInfoVec;
        c_token < p_jobInfo->formalTokens;
          c_token++, p_tokenInfo++ ) {

   /* The variables "matchSize" and "tokenSize" may or may not end up */
   /* with the same value. This depends on the regular expression and */
   /* the text in the input buffer,                                   */
   /* Eg. if the regular expression  "([:alpha:]*)[:space:]*" is      */
   /* matched against the string "Begin  program;" then we get        */
   /*                                                                 */
   /*   match = "Begin  "                                             */
   /*   matchSize = 7                                                 */
   /*                                                                 */
   /*   token = "Begin"                                               */
   /*   tokenSize = 5                                                 */

   p_tokenInfo->tk_matchSize = matchSize = re_match (
     &p_tokenInfo->tk_regexBuffer,
     p_jobInfo->currLine,
     p_jobInfo->lineSize,
     p_jobInfo->charPos,
     &p_tokenInfo->tk_regexRegs );

   b_foundMatch = (matchSize != -1);

   if (b_foundMatch)
     p_tokenInfo->tk_tokenSize = tokenSize =
       p_tokenInfo->tk_regexRegs.end  [p_tokenInfo->tk_lastGroup] -
       p_tokenInfo->tk_regexRegs.start[p_tokenInfo->tk_firstGroup];
   else
     p_tokenInfo->tk_tokenSize = tokenSize = -1;

   /*                                                                 */
   /* If a multi-line comment has been opened then all we are         */
   /* interested in is the corresponding end-of-comment.              */
   /* Otherwise more than one regular expressions may match the text. */
   /* How do we choose a match among all the successful ones?         */
   /* We could choose the one that gives the longest token, but       */
   /* there's a problem.                                              */
   /* Suppose two formal tokens were defined as follows:              */
   /*                                                                 */
   /*   Token 1:                                                      */
   /*     type  = LEXAN_token_iden                                    */
   /*     regex = ([a-zA-Z]*)                                         */
   /*                                                                 */
   /*   Token 2:                                                      */
   /*     type  = LEXAN_token_resWord                                 */
   /*     regex = (Begin)                                             */
   /*                                                                 */
   /* Now suppose the text being scanned is "Begin Program\n". Both   */
   /* regular expressions match the text, and the tokens are the      */
   /* same length. If we just take the longest one we might choose    */
   /* option 1 above, which is clearly wrong: a reserved word would   */
   /* be missed and an identifier would be detected instead.          */
   /*                                                                 */
   /* SOLUTION:                                                       */
   /*                                                                 */
   /* The token obtained at each pass of the loop will be the final   */
   /* choice if...                                                    */
   /* - it is an end-of-comment and it occurred in the middle of a    */
   /*   multi-line comment, or                                        */
   /* - it is the absolute longest of all tokens, or                  */
   /* - it is as long as the longest one AND it is a reserved word    */
   /*                                                                 */

   if (! b_foundMatch)
     continue;
   if ( (p_tokenInfo->tk_type == LEXAN_token_CLOSE_COMMENT) &&
        (p_jobInfo->b_midComment) ) {
     tokenIndex = c_token;
     break;
   }
   if (tokenSize < longestToken)
     continue;
   if ((tokenSize > longestToken) || (p_tokenInfo->tk_type == LEXAN_token_resWord)) {
     tokenIndex = c_token;
     longestToken = tokenSize;
   }
 }

 /* If there was no match then it's a lexical error */

 if (tokenIndex == -1) {
   p_jobInfo->totErrors++;
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s(): Erro lexico em \"%s\"\n[%4d] %s%s",
     __func__,
     p_jobInfo->fileName,
     p_jobInfo->lineNum,
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos) );
   *p_token = LEXAN_token_NULL;
   return (FALSE);
 }

 /* Choose the best match */

 p_jobInfo->tokenIndex = tokenIndex;

 p_tokenInfo  = (t_tokenInfo*) p_jobInfo->tokenInfoVec + tokenIndex;
 matchSize    = p_tokenInfo->tk_matchSize;
 tokenSize    = p_tokenInfo->tk_tokenSize;
 tokenStart   = p_tokenInfo->tk_regexRegs.start[p_tokenInfo->tk_firstGroup];
 extendedSize = p_tokenInfo->tk_regexRegs.end  [p_tokenInfo->tk_lastGroup] -
                p_tokenInfo->tk_regexRegs.start[0];

 /*                                                */
 /* Is this the beginning of a multi-line comment? */
 /*                                                */

 if ( (p_tokenInfo->tk_type == LEXAN_token_OPEN_COMMENT) &&
      (! p_jobInfo->b_midComment) ) {

   /* To process a multi-line comment we have to...  */
   /* - set the "processing comment" boolean to TRUE */
   /* - remember this line's number                  */
   /* - skip the open-comment characters             */
   /* - scan the text once again                     */

   p_jobInfo->b_midComment = TRUE;
   p_jobInfo->openCommLineNum = p_jobInfo->lineNum;
   b_consumedOK = consume_input (jobId, extendedSize);
   if (! b_consumedOK) {
     p_jobInfo->totErrors++;
     snprintf (
       p_jobInfo->lastError,
       ERROR_maxErrorMsgSize,
       "%s(): Erro ao processar inicio de comentario de bloco em \"%s\"\n[%4d] %s%s",
       __func__,
       p_jobInfo->fileName,
       p_jobInfo->lineNum,
       p_jobInfo->currLine,
       markErrorPos (p_jobInfo->charPos) );
   }
   b_nextScanOK = LEXAN_get_token (jobId, p_token, p_tokenVal);
   return (b_consumedOK && b_nextScanOK);
 }

 /*                                          */
 /* Is this the end of a multi-line comment? */
 /*                                          */

 if (p_tokenInfo->tk_type == LEXAN_token_CLOSE_COMMENT) {

   if (p_jobInfo->b_midComment)

     /* PANIC: This shouldn't happen!                    */
     /* If we are indeed processing a multi-line comment */
     /* then the call to re_search() should have found   */
     /* this LEXAN_token_CLOSE_COMMENT and acted on it!  */

     ERROR_fatal_error (0, __FILE__, __func__,
       "Panico! LEXAN_token_CLOSE_COMMENT nao encontrado");

   else {

     /* Trying to close a comment when none was opened */

     p_jobInfo->totErrors++;
     (void) consume_input (jobId, extendedSize);
     snprintf (
       p_jobInfo->lastError,
       ERROR_maxErrorMsgSize,
       "%s(): Tentando fechar comentario nao aberto em \"%s\"\n[%4d] %s%s",
       __func__,
       p_jobInfo->fileName,
       p_jobInfo->lineNum,
       p_jobInfo->currLine,
       markErrorPos (p_jobInfo->charPos) );
     return (FALSE);
   }

   /* To close the comment we have to...              */
   /* - set the "processing comment" boolean to FALSE */
   /* - skip the close-comment characters             */
   /* - scan the text once again                      */


   /*                                                                         */
   /*   W A R N I N G              W A R N I N G              W A R N I N G   */
   /*                                                                         */
   /*   The following code up to the return command will never be executed!   */
   /*                                                                         */


   p_jobInfo->b_midComment = FALSE;
   p_jobInfo->openCommLineNum = 0;
   b_consumedOK = consume_input (jobId, extendedSize);
   if (! b_consumedOK) {
     p_jobInfo->totErrors++;
     snprintf (
       p_jobInfo->lastError,
       ERROR_maxErrorMsgSize,
       "%s(): Erro ao processar fim de comentario de bloco em \"%s\"\n[%4d] %s%s",
       __func__,
       p_jobInfo->fileName,
       p_jobInfo->lineNum,
       p_jobInfo->currLine,
       markErrorPos (p_jobInfo->charPos) );
   }
   b_nextScanOK = LEXAN_get_token (jobId, p_token, p_tokenVal);
   return (b_consumedOK && b_nextScanOK);
 }

 /*                             */
 /* Is this a one-line comment? */
 /*                             */

 if (p_tokenInfo->tk_type == LEXAN_token_LINE_COMMENT) {

   /* To process a one-line comment we have to...           */
   /* - skip all characters right up to the end of the line */
   /* - scan the text once again                            */

   commentSize = p_jobInfo->lineSize - p_jobInfo->charPos;
   b_consumedOK = consume_input (jobId, commentSize);
   if (! b_consumedOK) {
     p_jobInfo->totErrors++;
     snprintf (
       p_jobInfo->lastError,
       ERROR_maxErrorMsgSize,
       "%s(): Erro ao processar comentario de linha em \"%s\"\n[%4d] %s%s",
       __func__,
       p_jobInfo->fileName,
       p_jobInfo->lineNum,
       p_jobInfo->currLine,
       markErrorPos (p_jobInfo->charPos) );
   }
   b_nextScanOK = LEXAN_get_token (jobId, p_token, p_tokenVal);
   return (b_consumedOK && b_nextScanOK);
 }

 /*                                                */
 /* Return token type and token value by reference */
 /*                                                */

 *p_token = p_jobInfo->currToken = p_tokenInfo->tk_type;

 errno = 0;
 if (tokenStr)
   tokenStr = (char *) realloc ((void *) tokenStr, (size_t) tokenSize+1);
 else
  tokenStr = (char *) malloc ((size_t) tokenSize+1);
 if (tokenStr == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "tokenStr");
 memset (tokenStr, 0, (size_t) tokenSize + 1);
 strncpy (tokenStr, &(p_jobInfo->currLine[tokenStart]), (size_t) tokenSize);

 switch (p_tokenInfo->tk_type) {

   case (LEXAN_token_single_str) :
   case (LEXAN_token_double_str) :

     /* Return literal string, including the */
     /* quotes, in p_tokenVal->tokenStr      */

     errno = 0;
     p_tokenVal->tokenStr =
       (char *) realloc ((void *) p_tokenVal->tokenStr, (size_t) tokenSize+1);
     if (p_tokenVal->tokenStr == NULL)
       ERROR_no_memory (errno, __FILE__, __func__, "p_tokenVal->tokenStr");
     memset (p_tokenVal->tokenStr, 0, (size_t) tokenSize + 1);
     strncpy (p_tokenVal->tokenStr, tokenStr, (size_t) tokenSize);

     /* Save token string in p_jobInfo->tokenVal->tokenStr */

     errno = 0;
     p_jobInfo->tokenVal.tokenStr = (char *) realloc (
       (void *) p_jobInfo->tokenVal.tokenStr,
       (size_t) tokenSize + 1 );
     if (p_jobInfo->tokenVal.tokenStr == NULL)
       ERROR_no_memory (errno, __FILE__, __func__, "p_jobInfo->tokenVal.tokenStr");
     memset (p_jobInfo->tokenVal.tokenStr, 0, (size_t) tokenSize + 1);
     strncpy (p_jobInfo->tokenVal.tokenStr, tokenStr, (size_t) tokenSize);
     break;

   case (LEXAN_token_resWord) :
   case (LEXAN_token_iden)    :

     /* Return token string in p_tokenVal->tokenStr */

     errno = 0;
     p_tokenVal->tokenStr =
       (char *) realloc ((void *) p_tokenVal->tokenStr, (size_t) tokenSize+1);
     if (p_tokenVal->tokenStr == NULL)
       ERROR_no_memory (errno, __FILE__, __func__, "p_tokenVal->tokenStr");
     memset (p_tokenVal->tokenStr, 0, (size_t) tokenSize + 1);
     strncpy (p_tokenVal->tokenStr, tokenStr, (size_t) tokenSize);

     /* Save token string in p_jobInfo->tokenVal->tokenStr */

     errno = 0;
     p_jobInfo->tokenVal.tokenStr = (char *) realloc (
       (void *) p_jobInfo->tokenVal.tokenStr,
       (size_t) tokenSize + 1 );
     if (p_jobInfo->tokenVal.tokenStr == NULL)
       ERROR_no_memory (errno, __FILE__, __func__, "p_jobInfo->tokenVal.tokenStr");
     memset (p_jobInfo->tokenVal.tokenStr, 0, (size_t) tokenSize + 1);
     strncpy (p_jobInfo->tokenVal.tokenStr, tokenStr, (size_t) tokenSize);
     break;

   case (LEXAN_token_longInt) :

     /* Convert and return token string in p_tokenVal->longIntVal */

     b_convOK = ERROR_str2all (
       tokenStr,
       ERROR_t_long_int,
       (void *) &(p_tokenVal->longIntVal) );
     if (! b_convOK) {
       p_jobInfo->totErrors++;
       *p_token = p_jobInfo->currToken = LEXAN_token_NULL;
       p_jobInfo->tokenIndex = -1;
       memset (p_tokenVal,             0, sizeof (LEXAN_t_tokenVal));
       memset (&(p_jobInfo->tokenVal), 0, sizeof (LEXAN_t_tokenVal));
       snprintf (
         p_jobInfo->lastError,
         ERROR_maxErrorMsgSize,
         "%s(): \n  %s em \"%s\"\n[%4d] %s%s",
         __func__,
         ERROR_auxErrorMsg,
         p_jobInfo->fileName,
         p_jobInfo->lineNum,
         p_jobInfo->currLine,
         markErrorPos (p_jobInfo->charPos) );
       (void) consume_input (jobId, extendedSize);
       return (FALSE);
     }

     /* Copy converted token string in p_jobInfo->tokenVal.longIntVal */

     p_jobInfo->tokenVal.longIntVal = p_tokenVal->longIntVal;
     break;

   case (LEXAN_token_double) :

     /* Convert and return token string in p_tokenVal->doubleVal */

     b_convOK = ERROR_str2all (
       tokenStr,
       ERROR_t_double,
       (void *) &(p_tokenVal->doubleVal) );
     if (! b_convOK) {
       p_jobInfo->totErrors++;
       *p_token = p_jobInfo->currToken = LEXAN_token_NULL;
       p_jobInfo->tokenIndex = -1;
       memset (p_tokenVal,             0, sizeof (LEXAN_t_tokenVal));
       memset (&(p_jobInfo->tokenVal), 0, sizeof (LEXAN_t_tokenVal));
       snprintf (
         p_jobInfo->lastError,
         ERROR_maxErrorMsgSize,
         "%s(): \n  %s em \"%s\"\n[%4d] %s%s",
         __func__,
	       ERROR_auxErrorMsg,
         p_jobInfo->fileName,
         p_jobInfo->lineNum,
         p_jobInfo->currLine,
         markErrorPos (p_jobInfo->charPos) );
       (void) consume_input (jobId, extendedSize);
       return (FALSE);
     }

     /* Copy converted token string in p_jobInfo->tokenVal.doubleVal */

     p_jobInfo->tokenVal.doubleVal = p_tokenVal->doubleVal;
     break;

   case (LEXAN_token_delim)   :

     /* Convert and return token string in p_tokenVal->delimChar */

     b_convOK = ERROR_str2all (
       tokenStr,
       ERROR_t_char,
       (void *) &(p_tokenVal->delimChar) );
     if (! b_convOK) {
       p_jobInfo->totErrors++;
       *p_token = p_jobInfo->currToken = LEXAN_token_NULL;
       p_jobInfo->tokenIndex = -1;
       memset (p_tokenVal,             0, sizeof (LEXAN_t_tokenVal));
       memset (&(p_jobInfo->tokenVal), 0, sizeof (LEXAN_t_tokenVal));
       snprintf (
         p_jobInfo->lastError,
         ERROR_maxErrorMsgSize,
         "%s(): \n  %s em \"%s\"\n[%4d] %s%s",
         __func__,
	       ERROR_auxErrorMsg,
         p_jobInfo->fileName,
         p_jobInfo->lineNum,
         p_jobInfo->currLine,
         markErrorPos (p_jobInfo->charPos) );
       (void) consume_input (jobId, extendedSize);
       return (FALSE);
     }

     /* Copy converted token string in p_jobInfo->tokenVal.delimChar */

     p_jobInfo->tokenVal.delimChar = p_tokenVal->delimChar;
     break;

   case (LEXAN_token_EOF)          :
   case (LEXAN_token_LINE_COMMENT) :
   case (LEXAN_token_OPEN_COMMENT) :
   case (LEXAN_token_CLOSE_COMMENT):
   case (LEXAN_token_NULL)         :
     break;
   default:
     p_jobInfo->totErrors++;
     snprintf (
       p_jobInfo->lastError,
       ERROR_maxErrorMsgSize,
       "%s(): Token %d desconhecido em \"%s\"\n[%4d] %s%s",
       __func__,
       p_tokenInfo->tk_type,
       p_jobInfo->fileName,
       p_jobInfo->lineNum,
       p_jobInfo->currLine,
       markErrorPos (p_jobInfo->charPos) );
     return (FALSE);
 }

 /* Eat up "extendedSize" characters from input buffer */

 if (! consume_input (jobId, extendedSize)) {
   p_jobInfo->totErrors++;
   *p_token = p_jobInfo->currToken = LEXAN_token_NULL;
   p_jobInfo->tokenIndex = -1;
   memset (p_tokenVal,             0, sizeof (LEXAN_t_tokenVal));
   memset (&(p_jobInfo->tokenVal), 0, sizeof (LEXAN_t_tokenVal));
   return (FALSE);
 }

 /* That's all */

 strcpy (p_jobInfo->lastError, "");
 return (TRUE);
}

/*
*---------------------------------------------------------------------
*  Obtain the value of a "subtoken".
*---------------------------------------------------------------------
*
*  Suppose the latest call to LEXAN_get_token() found a match
*  against this input buffer
*
*    "Unit43=999; etc."
*
*  and returned a token with the following specification:
*
*    type        = LEXAN_token_resWord
*    regex       = "(Unit)([:digit:]+)(=)([:digit:]+)"
*    firstGroup  = 1
*    lastGroup   = 2
*
*  This match results in
*
*    match = "Unit43=999"
*    matchSize = 7
*
*    token = LEXAN_token_resWord
*    token string =  "Unit43"
*    token size = 6
*
*    input buffer = "=999; etc."
*
*  Now, if we want to extract portions of a token string and
*  convert it to another token type, we use the procedure
*  LEXAN_get_subtoken(). Eg. to obtain the integer immediately
*  following the word "Unit" we do
*
*    LEXAN_t_tokenVal
*      subTokenVal;
*
*    b_subTokenOK = LEXAN_get_subtoken (
*      (LEXAN_t_lexJobId)   ...,
*      (LEXAN_t_tokenType)  LEXAN_token_longInt,
*      (int)          2,
*      (int)          2,
*      (LEXAN_t_tokenVal ) &subTokenVal );
*
*---------------------------------------------------------------------
*/

Bool LEXAN_get_subtoken (
  LEXAN_t_lexJobId  jobId,
  LEXAN_t_tokenType subToken,
  int               subGroupStart,
  int               subGroupEnd,
  LEXAN_t_tokenVal *p_subTokenVal )
{
 int
   subTokenStart,
   subTokenEnd,
   subTokenSize;
 Bool
   b_convOK;
 char
   *subTokenStr = NULL;
 int
   subExpr;
 t_jobInfo
   *p_jobInfo;
 t_tokenInfo
   *p_tokenInfo;

 /* Clear return values */

 memset (p_subTokenVal, 0, sizeof (LEXAN_t_tokenVal));

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);
 if (p_jobInfo == NULL)
   return (FALSE);

 /* Make sure the current token is a valid one */

 if (! ( (p_jobInfo->currToken == LEXAN_token_iden)       ||
         (p_jobInfo->currToken == LEXAN_token_resWord)    ||
         (p_jobInfo->currToken == LEXAN_token_single_str) ||
         (p_jobInfo->currToken == LEXAN_token_double_str) ||
         (p_jobInfo->currToken == LEXAN_token_delim)      ||
         (p_jobInfo->currToken == LEXAN_token_longInt)    ||
         (p_jobInfo->currToken == LEXAN_token_double) )) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s(): Token invalido em \"%s\"\n[%4d] %s%s",
     __func__,
     p_jobInfo->fileName,
     p_jobInfo->lineNum,
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos) );
   return (FALSE);
 }

 /* Make sure the current token index is a valid one */

 if (p_jobInfo->tokenIndex == -1) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s%s       Erro lexico na linha %d coluna %d: Caracter invalido '%c'\n",
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos),
     p_jobInfo->lineNum,
     p_jobInfo->charPos + 1,
     p_jobInfo->currLine[p_jobInfo->charPos] );
   return (FALSE);
 }

 /* Get a pointer to the original token's info */

 p_tokenInfo = (t_tokenInfo*) p_jobInfo->tokenInfoVec + p_jobInfo->tokenIndex;

 /* Make sure the starting and ending subgroup numbers are valid */

 if (subGroupStart > subGroupEnd) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s(): Ordem de groups invalida (%d,%d) em \"%s\"\n[%4d] %s%s",
     __func__,
     subGroupStart,
     subGroupEnd,
     p_jobInfo->fileName,
     p_jobInfo->lineNum,
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos) );
   return (FALSE);
 }

 subExpr = (int) p_tokenInfo->tk_regexRegs.num_regs;

 if (subGroupEnd > subExpr) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s(): Valor de grupo invalido (max %d) em \"%s\"\n[%4d] %s%s",
     __func__,
     subExpr,
     p_jobInfo->fileName,
     p_jobInfo->lineNum,
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos) );
   return (FALSE);
 }

 /* Subgroup numbers are ok    */
 /* Find location of subtoken  */

 subTokenStart = p_tokenInfo->tk_regexRegs.start[subGroupStart];
 subTokenEnd   = p_tokenInfo->tk_regexRegs.end  [subGroupEnd];
 subTokenSize  = subTokenEnd - subTokenStart;

 /* Put subtoken in its own string */

 errno = 0;
 subTokenStr = (char *) malloc ((size_t) subTokenSize +1 );
 if (subTokenStr == NULL)
   ERROR_no_memory (errno, __FILE__, __func__, "subTokenStr");
 memset (subTokenStr, 0, (size_t) subTokenSize + 1);
 strncpy (subTokenStr, &(p_jobInfo->currLine[subTokenStart]), (size_t) subTokenSize);

 switch (subToken) {
   case (LEXAN_token_resWord) :
   case (LEXAN_token_iden)    :

     /* Return subtoken string in p_subTokenVal->tokenStr */

     errno = 0;
     p_subTokenVal->tokenStr = (char *) realloc (
       (void *) p_subTokenVal->tokenStr,
       (size_t) subTokenSize + 1 );
     if (p_subTokenVal->tokenStr == NULL)
       ERROR_no_memory (errno, __FILE__, __func__, "p_subTokenVal->tokenStr");
     memset (p_subTokenVal->tokenStr, 0, (size_t) subTokenSize + 1);
     strncpy (p_subTokenVal->tokenStr, subTokenStr, (size_t) subTokenSize);
     break;

   case (LEXAN_token_longInt) :

     /* Convert and return subtoken string in p_subTokenVal->longIntVal */

     b_convOK = ERROR_str2all (
       subTokenStr,
       ERROR_t_long_int,
       (void *) &(p_subTokenVal->longIntVal) );
     if (! b_convOK) {
       memset (p_subTokenVal, 0, sizeof (LEXAN_t_tokenVal));
       snprintf (
         p_jobInfo->lastError,
         ERROR_maxErrorMsgSize,
         "%s(): \n  %s em \"%s\"\n[%4d] %s%s",  /* "%s(): \n  %s in \"%s\"\n[%4d] %s%s", */
         __func__,
	 ERROR_auxErrorMsg,
         p_jobInfo->fileName,
         p_jobInfo->lineNum,
         p_jobInfo->currLine,
         markErrorPos (p_jobInfo->charPos) );
         free ((char *) subTokenStr);
       return (FALSE);
     }
     break;

   case (LEXAN_token_double) :

     /* Convert and return subtoken string in p_subTokenVal->doubleVal */

     b_convOK = ERROR_str2all (
       subTokenStr,
       ERROR_t_double,
       (void *) &(p_subTokenVal->doubleVal) );
     if (! b_convOK) {
       memset (p_subTokenVal, 0, sizeof (LEXAN_t_tokenVal));
       snprintf (
         p_jobInfo->lastError,
         ERROR_maxErrorMsgSize,
         "%s(): \n  %s em \"%s\"\n[%4d] %s%s",  /* "%s(): \n  %s in \"%s\"\n[%4d] %s%s", */
         __func__,
	 ERROR_auxErrorMsg,
         p_jobInfo->fileName,
         p_jobInfo->lineNum,
         p_jobInfo->currLine,
         markErrorPos (p_jobInfo->charPos) );
         free ((char *) subTokenStr);
       return (FALSE);
     }
     break;

   case (LEXAN_token_delim)   :

     /* Convert and return subtoken string in p_subTokenVal->delimChar */

     b_convOK = ERROR_str2all (
       subTokenStr,
       ERROR_t_char,
       (void *) &(p_subTokenVal->delimChar) );
     if (! b_convOK) {
       memset (p_subTokenVal, 0, sizeof (LEXAN_t_tokenVal));
       snprintf (
         p_jobInfo->lastError,
         ERROR_maxErrorMsgSize,
         "%s(): \n  %s em \"%s\"\n[%4d] %s%s",  /* "%s(): \n  %s in \"%s\"\n[%4d] %s%s", */
         __func__,
	       ERROR_auxErrorMsg,
         p_jobInfo->fileName,
         p_jobInfo->lineNum,
         p_jobInfo->currLine,
         markErrorPos (p_jobInfo->charPos) );
         free ((char *) subTokenStr);
       return (FALSE);
     }
     break;

   case (LEXAN_token_double_str)   :
   case (LEXAN_token_single_str)   :
   case (LEXAN_token_EOF)          :
   case (LEXAN_token_NULL)         :
   case (LEXAN_token_LINE_COMMENT) :
   case (LEXAN_token_OPEN_COMMENT) :
   case (LEXAN_token_CLOSE_COMMENT):
     break;

   default:
     errno = 0;
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Tipo de meta-token desconhecido (%d)", subToken);
     ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* That's all */

 free ((char *) subTokenStr);
 strcpy (p_jobInfo->lastError, "");
 return (TRUE);
}

/*
*---------------------------------------------------------------------
*  Obtain the regular expression of the current token
*---------------------------------------------------------------------
*/

char *LEXAN_get_token_regex (LEXAN_t_lexJobId jobId)
{
 static char
   *tokenRegex = NULL;
 t_jobInfo
   *p_jobInfo;
 t_tokenInfo
   *p_tokenInfo;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);
 if (p_jobInfo == NULL)
   return (NULL);

 /* Make sure the current token is a valid one */

 if (! ( (p_jobInfo->currToken == LEXAN_token_iden)       ||
         (p_jobInfo->currToken == LEXAN_token_resWord)    ||
         (p_jobInfo->currToken == LEXAN_token_single_str) ||
         (p_jobInfo->currToken == LEXAN_token_double_str) ||
         (p_jobInfo->currToken == LEXAN_token_delim)      ||
         (p_jobInfo->currToken == LEXAN_token_longInt)    ||
         (p_jobInfo->currToken == LEXAN_token_double) )) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s(): Token invalido em \"%s\"\n[%4d] %s%s",
     __func__,
     p_jobInfo->fileName,
     p_jobInfo->lineNum,
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos) );
   return (NULL);
 }

 /* Make sure the current token index is a valid one */

 if (p_jobInfo->tokenIndex == -1) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s%s       Erro lexico na linha %d coluna %d: Caracter invalido '%c'\n",
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos),
     p_jobInfo->lineNum,
     p_jobInfo->charPos + 1,
     p_jobInfo->currLine[p_jobInfo->charPos] );
   return (NULL);
 }

 /* Get a pointer to the current token's info */

 p_tokenInfo = (t_tokenInfo*) p_jobInfo->tokenInfoVec + p_jobInfo->tokenIndex;

 /* Put token's regular expression in its own static string */

 errno = 0;
 tokenRegex = strdup (p_tokenInfo->tk_regexStr);
 if (tokenRegex == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Erro em tokenRegex = strdup (\"%s\")",
     p_tokenInfo->tk_regexStr );
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* That's all */

 return (tokenRegex);
}

/*
*---------------------------------------------------------------------
*  Obtain the location of the current token in the input file
*---------------------------------------------------------------------
*/

LEXAN_t_tokenLocation LEXAN_get_token_location (LEXAN_t_lexJobId jobId)
{
 LEXAN_t_tokenLocation
   tokenLocation,
   emptyTokenLocation;
 t_jobInfo
   *p_jobInfo;
 t_tokenInfo
   *p_tokenInfo;

 emptyTokenLocation.lineNumber = emptyTokenLocation.columnNumber = 0;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);
 if (p_jobInfo == NULL)
   return (emptyTokenLocation);

 /* Make sure the current token is a valid one

 if (! ( (p_jobInfo->currToken == LEXAN_token_iden)       ||
         (p_jobInfo->currToken == LEXAN_token_resWord)    ||
         (p_jobInfo->currToken == LEXAN_token_single_str) ||
         (p_jobInfo->currToken == LEXAN_token_double_str) ||
         (p_jobInfo->currToken == LEXAN_token_delim)      ||
         (p_jobInfo->currToken == LEXAN_token_longInt)    ||
         (p_jobInfo->currToken == LEXAN_token_double) )) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s(): Invalid token in \"%s\"\n[%4d] %s%s",
     __func__,
     p_jobInfo->fileName,
     p_jobInfo->lineNum,
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos) );
   return (emptyTokenLocation);
 }

*/

 /* Make sure the current token index is a valid one */

 if (p_jobInfo->tokenIndex == -1) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s%s       Erro lexico na linha %d coluna %d: Caracter invalido '%c'\n",
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos),
     p_jobInfo->lineNum,
     p_jobInfo->charPos + 1,
     p_jobInfo->currLine[p_jobInfo->charPos] );
   tokenLocation.lineNumber   = p_jobInfo->lineNum;
   tokenLocation.columnNumber = p_jobInfo->charPos;
   return (tokenLocation);
 }

 /* Get a pointer to the current token's info */

 p_tokenInfo = (t_tokenInfo*) p_jobInfo->tokenInfoVec + p_jobInfo->tokenIndex;

 /* Put token's location in its own static variable */

 errno = 0;
 tokenLocation.lineNumber   = p_jobInfo->lineNum;
 tokenLocation.columnNumber = p_jobInfo->charPos - p_tokenInfo->tk_tokenSize + 1;

 /* That's all */

 return (tokenLocation);
}

/*
*-------------------------------------------------------------------------------------
*  Return pointer to a static copy of the input line where the current token occurred
*-------------------------------------------------------------------------------------
*/

char *LEXAN_get_token_context (LEXAN_t_lexJobId jobId)
{
 t_jobInfo
   *p_jobInfo;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);
 if (p_jobInfo == NULL)
   return (NULL);

 /* Make sure the current token is a valid one

 if (! ( (p_jobInfo->currToken == LEXAN_token_iden)       ||
         (p_jobInfo->currToken == LEXAN_token_resWord)    ||
         (p_jobInfo->currToken == LEXAN_token_single_str) ||
         (p_jobInfo->currToken == LEXAN_token_double_str) ||
         (p_jobInfo->currToken == LEXAN_token_delim)      ||
         (p_jobInfo->currToken == LEXAN_token_longInt)    ||
         (p_jobInfo->currToken == LEXAN_token_double) )) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s(): Invalid token in \"%s\"\n[%4d] %s%s",
     __func__,
     p_jobInfo->fileName,
     p_jobInfo->lineNum,
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos) );
   return (NULL);
 }

*/

 /* Make sure the current token index is a valid one

 if (p_jobInfo->tokenIndex == -1) {
   snprintf (
     p_jobInfo->lastError,
     ERROR_maxErrorMsgSize,
     "%s%s       %s(): Invalid token index in line %d column %d of \"%s\"\n",
     p_jobInfo->currLine,
     markErrorPos (p_jobInfo->charPos),
     __func__,
     p_jobInfo->lineNum,
     p_jobInfo->charPos,
     p_jobInfo->fileName );
   return (NULL);
 }

*/

 /* Put a pointer to the current line in its own static variable */

 (void) STRING_copy (&globalCurrLine, p_jobInfo->currLine);

 return (globalCurrLine);
}

/*
*---------------------------------------------------------------------
*  Terminate an active job
*---------------------------------------------------------------------
*/

Bool LEXAN_terminate_job (LEXAN_t_lexJobId jobId)
{
 int
   c_token,
   kill_this;
 size_t
   nbytes;
 t_jobInfo
   *p_jobInfo;
 t_tokenInfo
   *p_tokenInfo;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);
 if (p_jobInfo == NULL)
   return (FALSE);

 /* To remove a job:                                               */
 /* - free memory dynamically allocated to its component fields:   */
 /*     (char *) p_jobInfo->fileName                               */
 /*     (char *) p_jobInfo->currLine                               */
 /*     (char *) p_jobInfo->lastError                              */
 /*     (char *) p_jobInfo->tokenVal.tokenStr                      */
 /*     for each formal token definition for the job:              */
 /*       (char *) p_jobInfo->tokenInfoVec[c_token].tk_regexStr    */
 /*       (char *) p_jobInfo->tokenInfoVec[c_token].tk_regexBuffer */
 /*     (t_tokenInfo *) tokenInfoVec                               */
 /* - close the corresponding input file                           */
 /* - if the job is not the last one in "jobInfoVec"               */
 /*   then the last one should be moved to take its place          */
 /* - realloc "jobInfoVec" with a smaller size                     */
 /* - update "activeJobs"                                          */
 /* - return TRUE                                                  */

 /* Freeing memory... */

 free (tokenStr);
 tokenStr = NULL;
 free (p_jobInfo->fileName);
 free (p_jobInfo->currLine);
 free (p_jobInfo->lastError);
 free (p_jobInfo->tokenVal.tokenStr);

 for (c_token = 0, p_tokenInfo = p_jobInfo->tokenInfoVec;
        c_token < p_jobInfo->formalTokens;
          c_token++, p_tokenInfo++ ) {
   free (p_tokenInfo->tk_regexStr);
   free (p_tokenInfo->tk_regexRegs.start);
   free (p_tokenInfo->tk_regexRegs.end);
   regfree ((regex_t *) &(p_tokenInfo->tk_regexBuffer));
 }
 free (p_jobInfo->tokenInfoVec);

 fclose (p_jobInfo->filePt);

 /* Filling in the space occupied by the job being terminated */

 for (kill_this = 0; kill_this < activeJobs; kill_this++)
   if (jobInfoVec[kill_this].jobId == jobId)
     break;
 if (kill_this < activeJobs-1)
   memcpy (&(jobInfoVec[kill_this]), &(jobInfoVec[activeJobs-1]), sizeof (t_jobInfo));

 /* Clearing the space occupied by the job that was at the top of the stack */

 memset (&(jobInfoVec[activeJobs-1]), 0, sizeof (t_jobInfo));
 activeJobs--;

 /* Resizing "jobInfoVec" */

 nbytes = activeJobs * sizeof (t_jobInfo);
 errno = 0;
 jobInfoVec = (t_jobInfo *) realloc (jobInfoVec, nbytes);
 if ((jobInfoVec == NULL) && (errno != 0))
   ERROR_no_memory (errno, __FILE__, __func__, "jobInfoVec");

 /* If there are no more active jobs, release dynamically allocated string */

 if (activeJobs == 0)
   STRING_release (&globalCurrLine);

 /* That's all */

 return (TRUE);
}

/*
*---------------------------------------------------------------------
*  Terminate all active jobs
*---------------------------------------------------------------------
*/

Bool LEXAN_terminate_all_jobs (void)
{
 Bool
   b_termOK = TRUE;

 while (activeJobs > 0)
   b_termOK = b_termOK && LEXAN_terminate_job (jobInfoVec[activeJobs-1].jobId);
 return (b_termOK);
}

/*
*---------------------------------------------------------------------
*  Return error description.
*---------------------------------------------------------------------
*/

char *LEXAN_get_error_descr (LEXAN_t_lexJobId jobId)
{
 t_jobInfo
   *p_jobInfo;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);

 return ((p_jobInfo == NULL) ? globalErrorDescr : p_jobInfo->lastError);
}

/*
*---------------------------------------------------------------------
*  Return error count.
*---------------------------------------------------------------------
*/

int LEXAN_get_error_count (LEXAN_t_lexJobId jobId)
{
 t_jobInfo
   *p_jobInfo;

 /* Obtain pointer to file information */

 p_jobInfo = get_job_info (jobId);

 return ((p_jobInfo == NULL) ? -1 : p_jobInfo->totErrors);
}
