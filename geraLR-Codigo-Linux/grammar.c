/*
*-----------------------------------------------------------------------
*
*   File         : grammar.c
*   Created      : 2006-08-01
*   Last Modified: 2018-05-19
*
*   DESCRIPTION:
*   Processing of context-free grammars, LR(0) items, NFAs and DFAs,
*   LR(0) and sLR(1) parsers
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

#include "common.h"
#include "error.h"
#include "lexan.h"
#include "datastructs.h"
#include "grammar.h"

/*
*-----------------------------------------------------------------------
* INTERFACE (visible from other modules)
*-----------------------------------------------------------------------
*/

/*
*-----------------------------------------------------------------------
* Definitions for grammar symbols and rules
*-----------------------------------------------------------------------
*/

unsigned int
  totRules               = 0,  /* Total number of grammar rules                           */
  totTerminals           = 0,  /* Total number of terminals                               */
  totNonTerminals        = 0,  /* Total number of non-terminals                           */
  totGrammarSymbols      = 0,  /* totTerminals + totNonTerminals                          */
  totFSAsymbols          = 0;  /* totGrammarSymbols + 3 (epsilon, rule arrow and the dot) */

size_t
  maxTerminalLength      = 0,  /* Length of longest terminal string       */
  maxNonTerminalLength   = 0,  /* Length of longest non-terminal string   */
  maxGrammarSymbolLength = 0;  /* Length of longest grammar symbol string */

float
  avgTerminalLength      = 0,  /* Average length of all terminal strings       */
  avgNonTerminalLength   = 0,  /* Average length of all non-terminal strings   */
  avgGrammarSymbolLength = 0;  /* Average length of all grammar symbol strings */

t_symbolCode
  end_of_input_code,
  epsilon_code,
  rule_arrow_code,
  item_dot_code,
  unknown_symbol_code;

/*
*-----------------------------------------------------------------------
* Definitions for LR(0) items, DFA states and transitions
*-----------------------------------------------------------------------
*/

unsigned int
  tot_NFA_shift_states             = 0,
  tot_NFA_reduce_states            = 0,
  tot_NFA_non_deterministic_states = 0,
  tot_DFA_shift_states             = 0,
  tot_DFA_1_reduce_states          = 0,
  tot_DFA_N_reduce_states          = 0,
  tot_DFA_shift_1_reduce_states    = 0,
  tot_DFA_shift_N_reduce_states    = 0;

unsigned int
  totLR0items       = 0,  /* Total number of LR(0) items                                                    */
  totLR0itemSymbols = 0,  /* Total number of symbols in all LR(0) items including rule arrows and item dots */
  totNFAstates      = 0,  /* Total number of NFA states                                                     */
  totNFAitems       = 0,  /* Total number of NFA items, equal to the number of LR(0) items                  */
  totNFAtransitions = 0,  /* Total number of NFA state transitions                                          */
  totDFAstates      = 0,  /* Total number of DFA states                                                     */
  totDFAitems       = 0,  /* Total number of DFA items, probably greater than the number of LR(0) items     */
  totDFAtransitions = 0;  /* Total number of DFA state transitions                                          */

/*
*---------------------------------------------------------------------
* Function prototypes
*---------------------------------------------------------------------
*/

void read_grammar (char *grammarFileName, char *progName, bool b_stripoff_quotes);

/* Methods for grammar symbols and rules */

t_symbolCode  symbolNumber2symbolCode (unsigned int symbolNumber, t_symbolType symbolType);
t_symbolType  symbolCode2symbolType   (t_symbolCode symbolCode);
const char   *symbolCode2symbolString (t_symbolCode symbolCode);
unsigned int  ruleNumber2ruleSize     (t_ruleNumber ruleNumber);
t_symbolCode  rulePos2symbolCode      (t_ruleNumber ruleNumber, unsigned int rulePos);
unsigned int  symbolCode2totUses      (t_symbolCode symbolCode, t_whereInRule whereInRule);
t_ruleNumber  symbolCode2use          (t_symbolCode symbolCode, t_whereInRule whereInRule, unsigned int useNumber);
bool          isEpsilonRule           (t_ruleNumber ruleNumber);

/* Methods for LR(0) items */

bool          isReductionItem                (t_itemCode itemCode);
t_itemCode    itemNumber2itemCode            (unsigned int itemNumber);
t_ruleNumber  itemCode2ruleNumber            (t_itemCode itemCode);
t_dotPosition itemCode2dotPosition           (t_itemCode itemCode);
t_symbolCode  itemCode2transitionSymbol      (t_itemCode itemCode);
unsigned int  itemCode2transitionNumber      (t_itemCode itemCode, unsigned int whichTransition);
t_itemCode    ruleNumberDotPosition2itemCode (t_ruleNumber ruleNumber, t_dotPosition dotPosition);

/* Methods for NFA and DFA states and transitions */

t_stateType   stateCode2stateType       (t_stateCode stateCode);
const char   *stateType2stateTypeString (t_stateType stateType);

t_stateCode   nfa_stateNumber2stateCode   (unsigned int stateNumber);
unsigned int  nfa_stateCode2stateNumber   (t_stateCode stateCode);
t_itemCode    nfa_stateCode2itemCode      (t_stateCode stateCode);
t_ruleNumber  nfa_stateCode2reductionRule (t_stateCode stateCode);
bool          nfa_isReductionState        (t_stateCode stateCode);

t_stateCode   dfa_stateNumber2stateCode   (unsigned int stateNumber);
unsigned int  dfa_stateCode2stateNumber   (t_stateCode stateCode);
unsigned int  dfa_stateCode2totItems      (t_stateCode stateCode);
t_itemCode    dfa_stateCode2itemCode      (t_stateCode stateCode, unsigned int itemNumber);
unsigned int  dfa_stateCode2totReductions (t_stateCode stateCode);
t_ruleNumber  dfa_stateCode2reductionRule (t_stateCode stateCode, unsigned int reductionNumber);
bool          dfa_isReductionState        (t_stateCode stateCode);
t_stateCode   dfa_lookupNextState         (t_stateCode currStateCode, t_symbolCode transitionSymbol);

void          nfa_sortTransitions                     (t_transitionSortKey sortKey);
t_stateCode   nfa_transitionNumber2originState        (unsigned int transitionNumber);
t_symbolCode  nfa_transitionNumber2symbol             (unsigned int transitionNumber);
t_stateCode   nfa_transitionNumber2destState          (unsigned int transitionNumber);
unsigned int  nfa_symbolCode2totTransitionsWithSymbol (t_symbolCode symbolCode);
unsigned int  nfa_stateCode2totTransitionsFromState   (t_stateCode stateCode);
unsigned int  nfa_stateCode2totTransitionsToState     (t_stateCode stateCode);
t_symbolCode  nfa_stateCode2transitionSymbol          (t_stateCode stateCode, unsigned int transitionNumber);

void          dfa_sortTransitions                            (t_transitionSortKey sortKey);
t_stateCode   dfa_transitionNumber2originState               (unsigned int transitionNumber);
t_symbolCode  dfa_transitionNumber2symbol                    (unsigned int transitionNumber);
t_stateCode   dfa_transitionNumber2destState                 (unsigned int transitionNumber);
unsigned int  dfa_symbolCode2totTransitionsWithSymbol        (t_symbolCode symbolCode);
unsigned int  dfa_stateCode2totTransitionsFromStateFromState (t_stateCode stateCode);
unsigned int  dfa_stateCode2totTransitionsFromStateToState   (t_stateCode stateCode);
t_symbolCode  dfa_stateCode2transitionSymbol                 (t_stateCode stateCode, unsigned int transitionNumber);

void build_LR0_items_NFA_and_DFA (void);

/* Methods for FIRST and FOLLOW sets */

void          build_first_sets  (void);
void          build_follow_sets (void);
unsigned int  setSize           (t_setType setType, t_symbolCode symbolCode);
t_symbolCode  getSymbolInSet    (t_setType setType, t_symbolCode symbolCode, unsigned int posInSet);

/* Methods for LR(0) and SLR(1) parser tables */

void build_LR0_parse_table  (void);
void build_sLR1_parse_table (void);
void build_diff_parse_table (void);

void free_LR0_parse_table_memory  (void);
void free_sLR1_parse_table_memory (void);
void free_diff_parse_table_memory (void);

unsigned int parseTablePos2totParseActions (
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode );

t_parseAction parseTablePos2parseAction (
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode,
  unsigned int       actionNumber );

unsigned int parseTableSummaryCol2totParseActions (
  t_parse_table_type parse_table_type,
  t_parseActionType  parseActionType,
  t_stateCode        stateCode );

unsigned int parseTableSummaryRow2totParseActions (
  t_parse_table_type parse_table_type,
  t_parseActionType  parseActionType,
  t_symbolCode       symbolCode );

/* To prevent "implicit declaration" warnings */

int snprintf (char *str, size_t size, const char *format, ...);

/*
*-----------------------------------------------------------------------
* IMPLEMENTATION (invisible from other modules)
*-----------------------------------------------------------------------
*/

/*
*-----------------------------------------------------------------------
* Definitions for grammar file processing routines
*-----------------------------------------------------------------------
*/

#define MAX_CHARS_EACH_SYMBOL       50
#define MAX_CHARS_ALL_SYMBOLS       20000
#define MAX_SYMBOL_LEFTHAND_USES    100
#define MAX_SYMBOL_RIGHTHAND_USES   MAX_RULES
#define MAX_SYMBOLS_ALL_RIGHTHANDS  ((MAX_RULES)*(MAX_SYMBOLS_EACH_RIGHTHAND))

/* Some pre-defined symbol codes */

#define UNKNOWN_SYMBOL_CODE        -5
#define ALPHA_CODE                 -4  /* alpha as in X -> alpha Y beta */
#define RULE_ARROW_CODE            -3
#define DOT_CODE                   -2
/*
#define ENDOFINPUT_CODE            -1
*/
#define EPSILON_CODE                0
#define TERMINAL_START_CODE         1
#define NON_TERMINAL_START_CODE  1001

#define ENDOFINPUT_STR             "$"
#define EPSILON_STR                "_epsilon_"
#define INITIAL_SYMBOL_PREFIX      ""
#define INITIAL_SYMBOL_SUFFIX      "'"

                                /* How grammar rules are stored             */
typedef struct {                /* Example:  E -> T + E                     */
  t_symbolCode lefthandSymbol;  /*   Code of E: 1001 or similar             */
  unsigned int righthandSize;   /*   Number of symbols in righthand side: 3 */
  unsigned int posFirstSymbol;  /*   Position of T in array righthandSides  */
}
  t_ruleData;

static t_ruleData
  grammarRules [MAX_RULES];

typedef struct {
  t_symbolCode symbolCode;
  unsigned int posFirstChar;                               /* Position in array symbolNames of symbol's 1st char        */
  unsigned int totLefthandUses;                            /* Number of rules with this symbol in the lefthand side...  */
  unsigned int lefthandUses [MAX_SYMBOL_LEFTHAND_USES];    /* ...and these are the rules above                          */
  unsigned int totRighthandUses;                           /* Number of rules with this symbol in the righthand side... */
  unsigned int righthandUses [MAX_SYMBOL_RIGHTHAND_USES];  /* ...and these are the rules above                          */
  DATA_t_set_code   firstSet;                              /* Numeric code of the FIRST set                             */
  DATA_t_set_code   followSet;                             /* Numeric code of the FOLLOW set                            */
  unsigned int totNFAtransitionsWithSymbol;                /* Number of NFA and DFA transitions...                      */
  unsigned int totDFAtransitionsWithSymbol;                /* ...with this symbol                                       */
}                                                          /*                                                           */
  t_symbolData;                                            /* One entry for each terminal and non-terminal              */

static t_symbolData
  terminals    [MAX_TERMINALS],
  nonTerminals [MAX_NON_TERMINALS],
  endOfInputUsage,
  epsilonUsage;

static t_symbolCode                             /* Righthand sides of all  */
  righthandSides [MAX_SYMBOLS_ALL_RIGHTHANDS],  /* rules appended together */
  initialSymbolCode;

static char                              /* Strings of all terminals and    */
  symbolNames [MAX_CHARS_ALL_SYMBOLS];   /* non-terminals appended together */

static unsigned int
  nextSymbolChar   = 0,   /* Next available position in array symbolNames */
  nextRighthandPos = 0;   /* Total number of terminals                    */

static char
  epsilonString       [MAX_CHARS_EACH_SYMBOL],
  endOfInputString    [MAX_CHARS_EACH_SYMBOL],
  initialSymbolString [MAX_CHARS_EACH_SYMBOL];

static size_t
  totTerminalLength,
  totNonTerminalLength,
  totGrammarSymbolLength;

/*
*-----------------------------------------------------------------------
* Definitions for LR(0) items, NFA and DFA states and transitions
*-----------------------------------------------------------------------
*/

/* The total number of LR(0) items is the same in both types of automata */
/* but in the DFA the same item may appear in more than one state        */

#define MAX_NFA_TRANSITIONS            (MAX_LR0_ITEMS)
#define MAX_ITEMS_PER_NFA_STATE        1              /* This is an exact number */
#define MAX_TRANSITIONS_PER_NFA_STATE  (MAX_RULES)

#define MAX_DFA_TRANSITIONS            (MAX_LR0_ITEMS)
#define MAX_ITEMS_PER_DFA_STATE        (MAX_RULES)    /* This is an estimate */
#define MAX_TRANSITIONS_PER_DFA_STATE  (MAX_ITEMS_PER_DFA_STATE)

#define UNKNOWN_TRANSITION_INDEX      -2
#define UNKNOWN_ITEM_INDEX            -1
#define UNKNOWN_ITEM_CODE              0
#define ITEM_START_CODE                1
#define UNKNOWN_STATE_CODE             0
#define DFA_STATE_START_CODE           1
#define NFA_STATE_START_CODE           10001  /* ((MAX_DFA_STATES)+1) */

/* Short strings describing each type of NFA and DFA state */


#define FIRST_STATE_TYPE    t_NFA_shift_state
#define LAST_STATE_TYPE     t_DFA_shift_N_reduce_state
#define TOTAL_STATE_TYPES   ((LAST_STATE_TYPE)-(FIRST_STATE_TYPE)+1)

#define NFA_SHIFT_STATE_STR               "Single shift on terminal"
#define NFA_REDUCE_STATE_STR              "Single reduction"
#define NFA_NON_DETERMINISTIC_STATE_STR   "Single shift on non-terminal & epsilon transition(s)"
#define DFA_SHIFT_STATE_STR               "Shift(s) only"
#define DFA_1_REDUCE_STATE_STR            "Single reduction only"
#define DFA_N_REDUCE_STATE_STR            "Multiple reductions only"
#define DFA_SHIFT_1_REDUCE_STATE_STR      "Shift(s) & single reduction"
#define DFA_SHIFT_N_REDUCE_STATE_STR      "Shift(s) & multiple reductions"

static char
  stateTypeString[TOTAL_STATE_TYPES][65] = {
    NFA_SHIFT_STATE_STR            ,
    NFA_REDUCE_STATE_STR           ,
    NFA_NON_DETERMINISTIC_STATE_STR,
    DFA_SHIFT_STATE_STR            ,
    DFA_1_REDUCE_STATE_STR         ,
    DFA_N_REDUCE_STATE_STR         ,
    DFA_SHIFT_1_REDUCE_STATE_STR   ,
    DFA_SHIFT_N_REDUCE_STATE_STR   };

/* Each LR(0) item is stored exactly once, though it may */
/* occur in several states both in the NFA and the DFA   */

typedef struct {              /* How LR(0) items are stored: */
  t_ruleNumber  ruleNumber;   /*   the rule number and       */
  t_dotPosition dotPosition;  /*   the position of the dot   */
}
  t_LR0item;

static t_LR0item
  LR0items [MAX_LR0_ITEMS];

/* In a NFA each state contains exactly one item, but possibly several transitions     */
/* (at most one with a terminal or non-terminal and maybe several epsilon transitions) */

typedef struct {                       /* How NFA states are stored:              */
  t_stateType  stateType;              /*   shift, reduce or non_deterministic    */
  int          itemIndex;              /*   position of item in LR0items[]        */
  unsigned int totInwardTransitions;   /*   number of transitions into this state */
  unsigned int totOutwardTransitions;  /*   number of transitions from this state */
}
  t_NFAstate;

static t_NFAstate
  NFAstates [MAX_NFA_STATES];

/* In a DFA each state may contain several items, so    */
/* the automaton requires a more complex data structure */

typedef struct {                                            /* How DFA states are stored:                             */
  t_stateType  stateType;                                   /*   shift, reduce, shift_reduce or reduce_reduce         */
  unsigned int itemsInState;                                /*   number of LR(0) items                                */
  int          itemIndex [MAX_ITEMS_PER_DFA_STATE];         /*   positions of items in LR0items[]                     */
  unsigned int totInwardTransitions;                        /*   number of transitions into this state                */
  unsigned int totTransitionSymbols;                        /*   number of distinct symbols immediately after the dot */
  t_symbolCode transitionSymbols [MAX_ITEMS_PER_DFA_STATE]; /*   list of symbol codes, as above                       */
}
  t_DFAstate;

static t_DFAstate
  DFAstates [MAX_DFA_STATES];

/* State transitions are represented in the same way in both types of automata */

typedef struct {              /* How state transitions are stored:       */
  t_stateCode  fromState;     /*   the numeric code of state of origin   */
  t_symbolCode withSymbol;    /*   the numeric code of transition symbol */
  t_stateCode  toState;       /*   the numeric code of destination state */
}
  t_stateTransition;

static t_stateTransition
  NFAtransitions [MAX_NFA_TRANSITIONS],
  DFAtransitions [MAX_DFA_TRANSITIONS];

static unsigned int
  nextLR0item       = 0,   /* Next available position in array LR0items       */
  nfa_nextState     = 0,   /* Next available position in array NFAstates      */
  dfa_nextState     = 0,   /* Next available position in array DFAstates      */
  nextNFAtransition = 0,   /* Next available position in array NFAtransitions */
  nextDFAtransition = 0;   /* Next available position in array DFAtransitions */

/*
*-----------------------------------------------------------------------
* Definitions for first & follow sets
*-----------------------------------------------------------------------
*/

typedef enum {
  t_first_set,
  t_follow_set,
  t_first_alpha_set   /* This really means FIRST (alpha) */
}
  t_extSetType;

                       /* Only the following fields will be actually used: */
static t_symbolData    /*   int          symbolsInFirstSet;                */
  alphaUsage;          /*   t_symbolCode firstSet [MAX_TERMINALS];         */

size_t
  setElementSize = sizeof (t_symbolCode);

/*
*-----------------------------------------------------------------------
* Definitions for LR(0), sLR(1) and differences parse tables
*-----------------------------------------------------------------------
*/

struct t_parseActionList {
  t_parseAction parseAction;
  struct t_parseActionList *p_nextParseAction;
};

struct t_parseTableEntry {
  unsigned int totParseActions;
  struct t_parseActionList *p_parseActionList;
};

#define MAX_TABLE_SYMBOLS   ((MAX_TERMINALS) + (MAX_NON_TERMINALS) + 1)
#define MAX_TABLE_STATES    MAX_DFA_STATES

struct t_parseTableEntry
   LR0parseTable [MAX_TABLE_STATES][MAX_TABLE_SYMBOLS],
  sLR1parseTable [MAX_TABLE_STATES][MAX_TABLE_SYMBOLS],
  diffParseTable [MAX_TABLE_STATES][MAX_TABLE_SYMBOLS];

/*
*-----------------------------------------------------------------------
* Definitions for LR(0) and sLR(1) parse tables summary (statistics)
*-----------------------------------------------------------------------
*/

typedef struct {
  unsigned int totShifts;
  unsigned int totReductions;
  unsigned int totGotos;
}
  t_parseTableSummary;

t_parseTableSummary
   LR0parseTableSummaryCol [MAX_TABLE_STATES],
   LR0parseTableSummaryRow [MAX_TABLE_SYMBOLS],
  sLR1parseTableSummaryCol [MAX_TABLE_STATES],
  sLR1parseTableSummaryRow [MAX_TABLE_SYMBOLS],
  diffParseTableSummaryCol [MAX_TABLE_STATES],
  diffParseTableSummaryRow [MAX_TABLE_SYMBOLS];

/*
*---------------------------------------------------------------------
* Function prototypes
*---------------------------------------------------------------------
*/

static void set_string_lengths (void);

static void validateToken (
 LEXAN_t_tokenType tokenExpected,
 LEXAN_t_tokenVal  valueExpected,
 LEXAN_t_tokenType tokenFound,
 LEXAN_t_tokenVal  valueFound );

/* Methods for grammar symbols and rules */

static int          symbolCode2symbolNumber (t_symbolCode symbolCode);
static t_symbolCode symbolStr2Code          (char *symbolStr);

static t_symbolCode add_terminal (
 LEXAN_t_tokenVal    tokenVal,
 t_whereInRule whereInRule,
 unsigned int  ruleNumber,
 bool          b_stripoff_quotes );

static t_symbolCode add_nonTerminal (
 char         *symbolStr,
 t_whereInRule whereInRule,
 unsigned int  ruleNumber );

static void add_rule (
 unsigned int ruleNumber,
 t_symbolCode lefthandSymbol,
 unsigned int righthandSize,
 unsigned int posFirstSymbol );

/* Methods for LR(0) items, NFA and DFA states and transitions */

static t_itemCode   newItem                 (t_ruleNumber ruleNumber, t_dotPosition dotPosition);
static t_FSA_type   stateCode2fsaType       (t_stateCode stateCode);
static void         validate_state_FSA_type (t_stateCode stateCode, t_FSA_type expected_fsaType, char *ERROR_errorMsg);

static t_stateCode  nfa_newEmptyState                (void);
static void         nfa_newTransition                (t_stateCode fromState, t_symbolCode withSymbol, t_stateCode toState);
static bool         nfa_addItemToState               (t_itemCode itemCode, t_stateCode stateCode);
static bool         nfa_isEmptyState                 (t_stateCode stateCode);

static t_stateCode  dfa_newEmptyState                (void);
static void         dfa_newTransition                (t_stateCode fromState, t_symbolCode withSymbol, t_stateCode toState);
static bool         dfa_addItemToState               (t_itemCode itemCode, t_stateCode stateCode);
static bool         dfa_isEmptyState                 (t_stateCode stateCode);
static bool         dfa_isDuplicateState             (t_stateCode stateCode);
static t_stateCode  dfa_stateCode2duplicateStateCode (t_stateCode stateCode);
static void         dfa_removeState                  (t_stateCode stateCode);
static t_stateCode  dfa_gotoState                    (t_stateCode currStateCode, t_symbolCode transitionSymbol);
static void         dfa_closure                      (t_stateCode stateCode);

static int compare_symbolCodes                 (const void *p1, const void *p2);
/*
static int compare_LR0items                    (const void *p1, const void *p2);
*/
static int compare_transitions_key_origin      (const void *p1, const void *p2);
static int compare_transitions_key_symbol      (const void *p1, const void *p2);
static int compare_transitions_key_destination (const void *p1, const void *p2);

/* Methods for FIRST and FOLLOW sets */

static DATA_t_set_code get_set_code    (t_extSetType setType, t_symbolCode symbolCode);
static unsigned int    totSymbolsInSet (t_extSetType setType, t_symbolCode symbolCode);
static t_symbolCode    symbolInSet     (t_extSetType setType, t_symbolCode symbolCode, unsigned int posInSet);

static bool has_SET         (t_symbolCode symbolCode,     t_extSetType setType);
static bool can_be_in_SET   (t_symbolCode symbolCode,     t_extSetType setType);
static bool is_in_SET       (t_symbolCode symbolToCheck,  t_extSetType setType, t_symbolCode setOfSymbol);
static bool add_to_SET      (t_symbolCode symbolToAdd,    t_extSetType setType, t_symbolCode setOfSymbol);
static bool remove_from_SET (t_symbolCode symbolToRemove, t_extSetType setType, t_symbolCode setOfSymbol);
static bool add_SETS        (t_extSetType fromSetType,    t_symbolCode fromSymbol, t_extSetType toSetType, t_symbolCode toSymbol);
static void clear_SET       (t_extSetType setType,        t_symbolCode symbolCode);

/* Methods for SLR(1) and LALR(1) parsers */

static unsigned int stateCode2parseTableRow  (t_stateCode stateCode);
static unsigned int symbolCode2parseTableCol (t_symbolCode symbolCode);

static void addParseAction (
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode,
  t_parseAction      parseAction );

static bool b_parseAction_isIn_parseTablePos (
  t_parseAction      parseAction,
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode );

/*
*---------------------------------------------------------------------
* Comparison functions used by qsort
*---------------------------------------------------------------------
*/

static int compare_symbolCodes (const void *p1, const void *p2)
{
 return (*(t_symbolCode *) p1 - *(t_symbolCode *) p2);
}

/*
static int compare_LR0items (const void *p1, const void *p2)
{
 t_LR0item
   item1,
   item2;
 int
   result;

 item1 = LR0items[*(int *) p1];
 item2 = LR0items[*(int *) p2];
 result = (int) item1.ruleNumber - item2.ruleNumber;
 if (result == 0)
   result = (int) item1.dotPosition - item2.dotPosition;
 return (result);
}
*/

static int compare_transitions_key_origin (const void *p1, const void *p2)
{
 return (((t_stateTransition *) p1)->fromState - ((t_stateTransition *) p2)->fromState);
}

static int compare_transitions_key_symbol (const void *p1, const void *p2)
{
 return (((t_stateTransition *) p1)->withSymbol - ((t_stateTransition *) p2)->withSymbol);
}

static int compare_transitions_key_destination (const void *p1, const void *p2)
{
 return (((t_stateTransition *) p1)->toState - ((t_stateTransition *) p2)->toState);
}

/*
*---------------------------------------------------------------------
* Process options and arguments in the command line
*---------------------------------------------------------------------
*/

static void validateToken (
  LEXAN_t_tokenType    tokenExpected,
  LEXAN_t_tokenVal valueExpected,
  LEXAN_t_tokenType    tokenFound,
  LEXAN_t_tokenVal valueFound )
{
 if (tokenFound == tokenExpected) {
   if (tokenFound == LEXAN_token_delim) {
     if (valueFound.delimChar == valueExpected.delimChar)
       return;
     else {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Expected %s [%c], found %s [%c]",
         LEXAN_tokenTypeNames[tokenExpected],
         valueExpected.delimChar,
         LEXAN_tokenTypeNames[tokenFound],
         valueFound.delimChar);
       ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
   }
   if (! strcmp (valueFound.tokenStr, valueExpected.tokenStr))
     return;
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Expected %s [%s], found %s [%s]",
     LEXAN_tokenTypeNames[tokenExpected],
     valueExpected.tokenStr,
     LEXAN_tokenTypeNames[tokenFound],
     valueFound.tokenStr);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 switch (tokenExpected) {
   case (LEXAN_token_single_str):
   case (LEXAN_token_double_str):
   case (LEXAN_token_iden)      :
   case (LEXAN_token_resWord)   :
     snprintf (ERROR_errorMsg, ERROR_maxErrorMsgSize, "Expected %s [%s]",
       LEXAN_tokenTypeNames[tokenExpected],
       valueExpected.tokenStr);
     break;
   case (LEXAN_token_delim):
     snprintf (ERROR_errorMsg, ERROR_maxErrorMsgSize, "Expected %s [%c]",
       LEXAN_tokenTypeNames[tokenExpected],
       valueExpected.delimChar);
     break;
   case (LEXAN_token_longInt):
     snprintf (ERROR_errorMsg, ERROR_maxErrorMsgSize, "Expected %s [%s]",
       LEXAN_tokenTypeNames[tokenExpected],
       valueExpected.tokenStr);
     break;
   case (LEXAN_token_double):
     snprintf (ERROR_errorMsg, ERROR_maxErrorMsgSize, "Expected %s [%s]",
       LEXAN_tokenTypeNames[tokenExpected],
       valueExpected.tokenStr);
     break;
   case (LEXAN_token_EOF)         :
   case (LEXAN_token_NULL)        :
   case (LEXAN_token_LINE_COMMENT)     :
   case (LEXAN_token_OPEN_COMMENT) :
   case (LEXAN_token_CLOSE_COMMENT):
     snprintf (ERROR_errorMsg, ERROR_maxErrorMsgSize, "Expected %s [%s]",
       LEXAN_tokenTypeNames[tokenExpected],
       valueExpected.tokenStr);
 }
 switch (tokenFound) {
   case (LEXAN_token_single_str):
   case (LEXAN_token_double_str):
   case (LEXAN_token_iden)      :
   case (LEXAN_token_resWord)   :
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "%s, found %s [%s]", ERROR_errorMsg,
       LEXAN_tokenTypeNames[tokenFound],
       valueFound.tokenStr);
     break;
   case (LEXAN_token_delim):
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "%s, found %s [%c]", ERROR_errorMsg,
       LEXAN_tokenTypeNames[tokenFound],
       valueFound.delimChar);
     break;
   case (LEXAN_token_longInt):
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "%s, found %s [%ld]", ERROR_errorMsg,
       LEXAN_tokenTypeNames[tokenFound],
       valueFound.longIntVal);
     break;
   case (LEXAN_token_double):
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "%s, found %s [%f]", ERROR_errorMsg,
       LEXAN_tokenTypeNames[tokenFound],
       valueFound.doubleVal);
     break;
   case (LEXAN_token_EOF)         :
   case (LEXAN_token_NULL)        :
   case (LEXAN_token_LINE_COMMENT)     :
   case (LEXAN_token_OPEN_COMMENT) :
   case (LEXAN_token_CLOSE_COMMENT):
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "%s, found %s", ERROR_errorMsg,
       LEXAN_tokenTypeNames[tokenFound] );
 }
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

/*
*---------------------------------------------------------------------
* Read the grammar file and populate various data structures
*---------------------------------------------------------------------
*/

void read_grammar (char *grammarFileName, char *progName, bool b_stripoff_quotes)
{
 #define REGEX_ARROW_1     "\\(->\\)"
 #define REGEX_ARROW_2     "\\(:=\\)"
 #define REGEX_DELIM       "\\([;\\|]\\)"
 #define REGEX_COMM        "\\(#\\|//\\)"
 #define REGEX_OPEN_COMM   "/\\*"
 #define REGEX_CLOSE_COMM  "\\*/"
 #define REGEX_IDEN        "\\([[:alpha:]_][[:alnum:]_]*\\)"
 #define totTokenSpecs  9

 LEXAN_t_tokenSpecs
   tokenSpecs [totTokenSpecs] = {
     {LEXAN_token_single_str,   LEXAN_REGEX_SINGLE_STR, 1, 1},
     {LEXAN_token_double_str,   LEXAN_REGEX_DOUBLE_STR, 1, 1},
     {LEXAN_token_iden,         REGEX_IDEN,             1, 1},
     {LEXAN_token_resWord,      REGEX_ARROW_1,          1, 1},
     {LEXAN_token_resWord,      REGEX_ARROW_2,          1, 1},
     {LEXAN_token_delim,        REGEX_DELIM,            1, 1},
     {LEXAN_token_LINE_COMMENT,      REGEX_COMM,             0, 0},
     {LEXAN_token_OPEN_COMMENT,  REGEX_OPEN_COMM,        0, 0},
     {LEXAN_token_CLOSE_COMMENT, REGEX_CLOSE_COMM,       0, 0}
   };

 LEXAN_t_tokenVal
   tokenVal,
   expectedTokenVal,
   currLefthandTokenVal,
   endOfInputTokenVal;
 char
   expectedTokenStr     [MAX_CHARS_EACH_SYMBOL] = "",
   currLefthandTokenStr [MAX_CHARS_EACH_SYMBOL] = "",
   endOfInputTokenStr   [MAX_CHARS_EACH_SYMBOL] = "";
 t_symbolCode
   currLefthandSymbol = UNKNOWN_SYMBOL_CODE;
 bool
   tokenOK;
 LEXAN_t_lexJobId
   jobId;
 LEXAN_t_tokenType
   token;
 unsigned int
   currState          = 0,
   sizeCurrRule       = 0,
   startCurrRighthand = 0,
   totEpsilonRules    = 0;

 /* Some initializations */

 memset (&grammarRules,         0, sizeof (grammarRules));
 memset (&terminals,            0, sizeof (terminals));
 memset (&nonTerminals,         0, sizeof (nonTerminals));
 memset (&endOfInputUsage,      0, sizeof (endOfInputUsage));
 memset (&epsilonUsage,         0, sizeof (epsilonUsage));
 memset (&righthandSides,       0, sizeof (righthandSides));
 memset (&symbolNames,          0, sizeof (symbolNames));
 memset (&LR0items,             0, sizeof (LR0items));
 memset (&DFAstates,            0, sizeof (DFAstates));
 memset (&DFAtransitions,       0, sizeof (DFAtransitions));

 memset (&tokenVal,             0, sizeof (LEXAN_t_tokenVal));
 tokenVal.tokenStr = NULL;

 memset (&endOfInputTokenVal,   0, sizeof (LEXAN_t_tokenVal));
 memset ( endOfInputTokenStr,   0, MAX_CHARS_EACH_SYMBOL);
 endOfInputTokenVal.tokenStr = endOfInputTokenStr;

 memset (&expectedTokenVal,     0, sizeof (LEXAN_t_tokenVal));
 memset ( expectedTokenStr,     0, MAX_CHARS_EACH_SYMBOL);
 expectedTokenVal.tokenStr = expectedTokenStr;

 memset (&currLefthandTokenVal, 0, sizeof (LEXAN_t_tokenVal));
 memset ( currLefthandTokenStr, 0, MAX_CHARS_EACH_SYMBOL);
 currLefthandTokenVal.tokenStr = currLefthandTokenStr;

 if ((strcpy (epsilonString, EPSILON_STR)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (epsilonString ,\"%s\") failed", EPSILON_STR);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if ((strcpy (endOfInputString, ENDOFINPUT_STR)) == NULL) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (endOfInputString ,\"%s\") failed", ENDOFINPUT_STR);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 epsilon_code        = EPSILON_CODE;
 rule_arrow_code     = RULE_ARROW_CODE;
 item_dot_code       = DOT_CODE;
 unknown_symbol_code = UNKNOWN_SYMBOL_CODE;
 end_of_input_code   = UNKNOWN_SYMBOL_CODE;
 initialSymbolCode   = UNKNOWN_SYMBOL_CODE;

 /* AUGMENTING THE GRAMMAR:                                               */
 /*                                                                       */
 /* Reserve position 0 in the rules array for the initial rule _E_ -> E $ */
 /* where E is the lefthand symbol of the first rule in the grammar file. */
 /* Both the initial symbol _E_ and the initial rule will be added        */
 /* as soon as the first non-terminal in the grammar is read in.          */

 totRules = 1;
 token = LEXAN_token_EOF;

 /* Get the lexical analyser started */

 jobId = LEXAN_start_job (
   (char *)         grammarFileName,
   (reg_syntax_t)   LEXAN_REGEX_SYNTAX,
   (LEXAN_t_tokenCase)    LEXAN_t_keepCase,
   (int)            totTokenSpecs,
   (LEXAN_t_tokenSpecs *) tokenSpecs );

 while (true) {

   /* FSA symbols: terminals, non-terminals, epsilon, the rule arrow and the item dot. */
   /* The end-of-input symbol was added as a terminal when the grammar was augmented.  */

   totGrammarSymbols = totTerminals + totNonTerminals;
   totFSAsymbols = totGrammarSymbols + 3;

   switch (token) {
     case (LEXAN_token_single_str):
     case (LEXAN_token_double_str):
     case (LEXAN_token_iden)      :
     case (LEXAN_token_resWord)   :
       if (tokenVal.tokenStr) {
         free (tokenVal.tokenStr);
         tokenVal.tokenStr = NULL;
       }
     default:
       break;
   }
   tokenOK = LEXAN_get_token ((LEXAN_t_lexJobId) jobId, (LEXAN_t_tokenType *) &token, (LEXAN_t_tokenVal *) &tokenVal);
   if (! tokenOK) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "%s\n", LEXAN_get_error_descr (jobId));
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
   if (currState == 0) {   /* Acceptable values are: non-terminal or end of file */
     switch (token) {
       case (LEXAN_token_EOF):
         (void) LEXAN_terminate_job (jobId);
         set_string_lengths();
         return;
       case (LEXAN_token_iden):   /* It is a non-terminal */

         /* If this is the very first non-terminal in the grammar file, say E, */
         /* then we need to do the following, IN THIS ORDER:                   */
         /*   STEP 1: Create the initial symbol _E_                            */
         /*   STEP 2: Add the non-terminal _E_ to the lefthand  side of rule 0 */
         /*   STEP 3: Add the non-terminal  E  to the righthand side of rule 0 */
         /*   STEP 4: Add the terminal      $  to the righthand side of rule 0 */
         /*   STEP 5: Add the rule  _E_ -> E $                                 */

         if (totNonTerminals == 0) {

           /* STEP 1: Create the initial symbol _E_ */

           if ((strcpy (initialSymbolString, INITIAL_SYMBOL_PREFIX)) == NULL) {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (initialSymbolString ,\"%s\") failed", INITIAL_SYMBOL_PREFIX);
             ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
           }
           if ((strcat (initialSymbolString, tokenVal.tokenStr)) == NULL) {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (initialSymbolString ,\"%s\") failed", tokenVal.tokenStr);
             ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
           }
           if ((strcat (initialSymbolString, INITIAL_SYMBOL_SUFFIX)) == NULL) {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (initialSymbolString ,\"%s\") failed", INITIAL_SYMBOL_SUFFIX);
             ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
           }

           /* STEP 2: Add the non-terminal _E_ to the lefthand side of rule 0 */

           initialSymbolCode = add_nonTerminal (initialSymbolString, t_lefthand, 0);

           /* STEP 3: Add the non-terminal E to the righthand side of rule 0 */

           (void) add_nonTerminal (tokenVal.tokenStr, t_righthand, 0);

           /* STEP 4: Add the terminal $ to the righthand side of rule 0 */

           if ((strcpy (endOfInputTokenVal.tokenStr, endOfInputString)) == NULL) {
             snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (endOfInputTokenVal.tokenStr ,\"%s\") failed", endOfInputString);
             ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
           }
           end_of_input_code = add_terminal (endOfInputTokenVal, t_righthand, 0, b_stripoff_quotes);

           /*   STEP 5: Add the rule  _E_ -> E $ */

           add_rule (0, initialSymbolCode, 2, 0);
         }

         /* Add the non-terminal to the lefthand side of the next rule */

         currLefthandSymbol = add_nonTerminal (tokenVal.tokenStr, t_lefthand, totRules);
         startCurrRighthand = nextRighthandPos;
         currLefthandTokenVal = tokenVal;
         currLefthandTokenVal.tokenStr = currLefthandTokenStr;
         if ((strcpy (currLefthandTokenVal.tokenStr, tokenVal.tokenStr)) == NULL) {
           snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "State %u, rule %u: strcpy (currLefthandTokenVal.tokenStr,\"%s\") failed", currState, totRules+1, tokenVal.tokenStr);
           ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
         }
         currState = 1;
         break;
       default:    /* This should never happen! */
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "State %u, rule %u: Invalid token %d while processing symbol %u", currState, totRules+1, token, sizeCurrRule);
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
   }
   else if (currState == 1) {   /* Acceptable values are: arrow ("->" or ":=") */
     if ((strcpy (expectedTokenVal.tokenStr, "->")) == NULL) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (expectedTokenVal.tokenStr,\"%s\") failed", "->");
       ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
     if ((token == LEXAN_token_resWord) && ! strcmp (tokenVal.tokenStr, ":=")) {
       if ((strcpy (expectedTokenVal.tokenStr, ":=")) == NULL) {
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (expectedTokenVal.tokenStr,\"%s\") failed", ":=");
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       }
     }
     validateToken (LEXAN_token_resWord, expectedTokenVal, token, tokenVal);
     currState = 2;
   }
   else if (currState == 2) {   /* Acceptable values are: terminal, non-terminal, "|" or ";" */
     switch (token) {
       case (LEXAN_token_iden):
         (void) add_nonTerminal (tokenVal.tokenStr, t_righthand, totRules);
         sizeCurrRule++;
         break;
       case (LEXAN_token_single_str):
       case (LEXAN_token_double_str):
         (void) add_terminal (tokenVal, t_righthand, totRules, b_stripoff_quotes);
         sizeCurrRule++;
         break;
       case (LEXAN_token_delim):   /* Either "|" or ";" */
         add_rule (totRules++, currLefthandSymbol, sizeCurrRule, startCurrRighthand);
         if (sizeCurrRule == 0)
           totEpsilonRules++;
         if (totEpsilonRules > 1) {   /* No more than one epsilon rule per symbol */
           snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "State %u, rule %u: Multiple epsilon rules for symbol %d [%s]", currState, totRules+1, currLefthandSymbol, symbolCode2symbolString (currLefthandSymbol));
           ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
         }
         sizeCurrRule = totEpsilonRules = 0;
         startCurrRighthand = nextRighthandPos;
         if (tokenVal.delimChar == ';') {
           currLefthandSymbol = UNKNOWN_SYMBOL_CODE;
           currState = 0;
         }
         else if (tokenVal.delimChar == '|') {
           (void) add_nonTerminal (currLefthandTokenVal.tokenStr, t_lefthand, totRules);
         }
         else {
           snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "State %u, rule %u: Invalid token %d while processing symbol %u", currState, totRules+1, token, sizeCurrRule);
           ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
         }
         break;
       default:       /* This should never happen! */
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "State %u, rule %u: Invalid token %d while processing symbol %u", currState, totRules+1, token, sizeCurrRule);
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
   }
   else {   /* This should never happen! */
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown state %u while processing symbol %u of rule %u", currState, sizeCurrRule, totRules+1);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }
}

/*
*---------------------------------------------------------------------
* Set various string length variables
*---------------------------------------------------------------------
*/

static void set_string_lengths (void)
{
 unsigned int
   iSymbol;
 size_t
   symbolLength;
 t_symbolCode
   symbolCode;

 /* Work out the maximum and average lengths of          */
 /* - terminal strings                                   */
 /* - non-terminal strings                               */
 /* - all grammar symbols strings including end-of-input */

 maxTerminalLength        =
   maxNonTerminalLength   =
   maxGrammarSymbolLength = (size_t) 0;
 totTerminalLength        =
   totNonTerminalLength   =
   totGrammarSymbolLength = (unsigned int) 0;
 avgTerminalLength        =
   avgNonTerminalLength   =
   avgGrammarSymbolLength = (float) 0;

 for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
   symbolLength = strlen (symbolCode2symbolString (symbolCode));
   maxTerminalLength = GREATEST (maxTerminalLength, symbolLength);
   totTerminalLength += symbolLength;
 }
 for (iSymbol = 2; iSymbol <= totNonTerminals; iSymbol++) {
   symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
   symbolLength = strlen (symbolCode2symbolString (symbolCode));
   maxNonTerminalLength = GREATEST (maxNonTerminalLength, symbolLength);
   totNonTerminalLength += symbolLength;
 }
 symbolLength = strlen (symbolCode2symbolString (end_of_input_code));
 maxGrammarSymbolLength = GREATEST (maxGrammarSymbolLength, symbolLength);
 symbolLength = strlen (symbolCode2symbolString (epsilon_code));
 maxGrammarSymbolLength = GREATEST (maxGrammarSymbolLength, symbolLength);
 maxGrammarSymbolLength = GREATEST (maxGrammarSymbolLength, maxTerminalLength);
 maxGrammarSymbolLength = GREATEST (maxGrammarSymbolLength, maxNonTerminalLength);
 totGrammarSymbolLength = symbolLength + totTerminalLength + totNonTerminalLength;

 avgTerminalLength      = (float) totTerminalLength      / totTerminals;
 avgNonTerminalLength   = (float) totNonTerminalLength   / totNonTerminals;
 avgGrammarSymbolLength = (float) totGrammarSymbolLength / (totTerminals+totNonTerminals+1);
}

/*
*---------------------------------------------------------------------
* Given a grammar symbol string, obtain its numeric code
*---------------------------------------------------------------------
*/

static t_symbolCode symbolStr2Code (char *symbolStr)
{
 unsigned int
   countSymbol;

 if (symbolStr == NULL)
   return (UNKNOWN_SYMBOL_CODE);

 /* Is it a previously encountered terminal? */

 for (countSymbol = 0; countSymbol < totTerminals; countSymbol++)
   if (! strcmp (&symbolNames[terminals[countSymbol].posFirstChar], symbolStr))
     return (terminals[countSymbol].symbolCode);

 /* Is it a previously encountered non-terminal? */

 for (countSymbol = 0; countSymbol < totNonTerminals; countSymbol++)
   if (! strcmp (&symbolNames[nonTerminals[countSymbol].posFirstChar], symbolStr))
     return (nonTerminals[countSymbol].symbolCode);

 /* Is it EPSILON_STR ? */

 if (! strcmp (epsilonString, symbolStr)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "%s is a reserved word\n", symbolStr);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* It's a new symbol, so it hasn't been given a code yet */

 return (UNKNOWN_SYMBOL_CODE);
}

/*
*---------------------------------------------------------------------
* Add terminal to various data structures
*---------------------------------------------------------------------
*/

static t_symbolCode add_terminal (
 LEXAN_t_tokenVal    tokenVal,
 t_whereInRule whereInRule,
 unsigned int  ruleNumber,
 bool          b_stripoff_quotes )
{
 t_symbolCode
   symbolCode;
 unsigned int
   index,
   posName;
 size_t
   nameLength;
 t_symbolData
   *p_symbolData;

 /* Strip off trailing and leading quotes */

 if (b_stripoff_quotes) {
   nameLength = strlen (tokenVal.tokenStr);
   (void) memmove ((void *) tokenVal.tokenStr, (void *) &tokenVal.tokenStr[1], (size_t) nameLength-1);
   tokenVal.tokenStr[nameLength-1] = '\0';
   tokenVal.tokenStr[nameLength-2] = '\0';
 }

 if ((symbolCode = symbolStr2Code (tokenVal.tokenStr)) == UNKNOWN_SYMBOL_CODE) {

   /* First occurrence of this symbol                */
   /* Add symbol string to array of all symbol names */

   nameLength = strlen (tokenVal.tokenStr);
   posName = nextSymbolChar;
   if ((posName + nameLength) > MAX_CHARS_ALL_SYMBOLS) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to add terminal %s: Symbol names array overflow\n", tokenVal.tokenStr);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
   if ((strcpy (&symbolNames[posName], tokenVal.tokenStr)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (symbolNames[%u],\"%s\") failed", posName, tokenVal.tokenStr);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
   nextSymbolChar += nameLength + 1;

   /* Add symbol to array of terminals and clear relevant fields */

   symbolCode = (t_symbolCode) (TERMINAL_START_CODE + totTerminals);
   p_symbolData = &terminals[totTerminals];
   p_symbolData->symbolCode = symbolCode;
   p_symbolData->posFirstChar = posName;
   p_symbolData->totLefthandUses = p_symbolData->totRighthandUses = 0;
   for (index = 0; index < MAX_SYMBOL_LEFTHAND_USES; index++)
     p_symbolData->lefthandUses[index] = p_symbolData->righthandUses[index] = 0;
   totTerminals++;
 }

 p_symbolData = &terminals[symbolCode - TERMINAL_START_CODE];

 /* Terminals are not allowed on the lefthand side of a rule! */

 if (whereInRule == t_lefthand) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Terminal %d [%s] on lefthand side of rule\n", symbolCode, tokenVal.tokenStr);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* This is the righthand side of a rule. However, the terminal */
 /*   may occur several times on the same righthand side        */
 /*   so make sure it's counted in only once                    */

 for (index = 0; index < p_symbolData->totRighthandUses; index++)
   if (p_symbolData->righthandUses[index] == ruleNumber)
     break;
 if (index == p_symbolData->totRighthandUses) {

   /* First occurrence in this righthand side, so make a note of it */

   p_symbolData->righthandUses[index] = ruleNumber;
   p_symbolData->totRighthandUses++;
 }

 /* Add symbol to array of all righthand sides */

 righthandSides[nextRighthandPos++] = symbolCode;

 return (symbolCode);
}

/*
*---------------------------------------------------------------------
* Add non-terminal to various data structures
*---------------------------------------------------------------------
*/

static t_symbolCode add_nonTerminal (
 char         *symbolStr,
 t_whereInRule whereInRule,
 unsigned int  ruleNumber )
{
 t_symbolCode
   symbolCode;
 unsigned int
   index,
   posName;
 size_t
   nameLength;
 t_symbolData
   *p_symbolData;

 if ((symbolCode = symbolStr2Code (symbolStr)) == UNKNOWN_SYMBOL_CODE) {

   /* First occurrence of this symbol                */
   /* Add symbol string to array of all symbol names */

   posName = nextSymbolChar;
   nameLength = strlen (symbolStr);
   if ((posName + nameLength) > MAX_CHARS_ALL_SYMBOLS) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to add non-terminal %s: Symbol names array overflow\n", symbolStr);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
   if ((strcpy (&symbolNames[posName], symbolStr)) == NULL) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (symbolNames[%u],\"%s\") failed", posName, symbolStr);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
   nextSymbolChar += nameLength + 1;

   /* Add symbol to array of non-terminals and clear relevant fields */

   symbolCode = (t_symbolCode) (NON_TERMINAL_START_CODE + totNonTerminals);
   p_symbolData = &nonTerminals[totNonTerminals];
   p_symbolData->symbolCode = symbolCode;
   p_symbolData->posFirstChar = posName;
   p_symbolData->totLefthandUses = p_symbolData->totRighthandUses = 0;
   for (index = 0; index < MAX_SYMBOL_LEFTHAND_USES; index++)
     p_symbolData->lefthandUses[index] = 0;
   for (index = 0; index < MAX_SYMBOL_RIGHTHAND_USES; index++)
     p_symbolData->righthandUses[index] = 0;
   totNonTerminals++;
 }
 else {

   /* The symbol already exists, so make sure it's not the initial symbol as it must */
   /* appear excatly once in the grammar (on the lefthand side of the first rule)    */

   if (symbolCode == initialSymbolCode) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to add non-terminal: %s is a reserved word\n", symbolStr);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }

 p_symbolData = &nonTerminals[symbolCode - NON_TERMINAL_START_CODE];
 switch (whereInRule) {
   case (t_lefthand):

     /* This is the leftthand side of a rule */

     p_symbolData->lefthandUses[p_symbolData->totLefthandUses++] = ruleNumber;
     break;

   case (t_righthand):

     /* This is the righthand side of a rule. The non-terminal */
     /*   may occur several times on the same righthand side   */
     /*   so make sure it's counted in only once               */

     for (index = 0; index < p_symbolData->totRighthandUses; index++)
       if (p_symbolData->righthandUses[index] == ruleNumber)
         break;
     if (index == p_symbolData->totRighthandUses)

       /* First occurrence in this righthand side, so make a note of it */

       p_symbolData->righthandUses[p_symbolData->totRighthandUses++] = ruleNumber;

     /* Add symbol to array of all righthand sides */

     righthandSides[nextRighthandPos++] = symbolCode;
 }

 return (symbolCode);
}

/*
*---------------------------------------------------------------------
* Add new rule
*---------------------------------------------------------------------
*/

static void add_rule (
 unsigned int ruleNumber,
 t_symbolCode lefthandSymbol,
 unsigned int righthandSize,
 unsigned int posFirstSymbol )
{
 t_ruleData
   *p_ruleData;

 /* Is this an epsilon rule?  eg. Z -> ;                         */
 /* If so, we need to take note of this epsilon usage in a rule. */

 if (righthandSize == 0)
   epsilonUsage.righthandUses[epsilonUsage.totRighthandUses++] = ruleNumber;

 /* Epsilon rules can be added as any other rule, in which case  */
 /* posFirstSymbol is meaningless.                               */

 p_ruleData = &grammarRules[ruleNumber];
 p_ruleData->lefthandSymbol = lefthandSymbol;
 p_ruleData->righthandSize = righthandSize;
 p_ruleData->posFirstSymbol = posFirstSymbol;
}

/*
*---------------------------------------------------------------------
* Take a symbol number (1 onwards) and type (terminal or non-terminal)
* and return its numeric code
*---------------------------------------------------------------------
*/

t_symbolCode symbolNumber2symbolCode (unsigned int symbolNumber, t_symbolType symbolType)
{
 if (symbolType == t_endOfInput) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "End-of-input does not have a symbol code (%u)\n", symbolNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (symbolType == t_epsilon) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Epsilon does not have a symbol code (%u)\n", symbolNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (symbolType == t_terminal) {
   if (symbolNumber <= totTerminals)
     return ((t_symbolCode) (TERMINAL_START_CODE + symbolNumber - 1));
   else {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid terminal number: %u\n", symbolNumber);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }
 if (symbolType == t_nonTerminal) {
   if (symbolNumber <= totNonTerminals)
     return ((t_symbolCode) (NON_TERMINAL_START_CODE + symbolNumber - 1));
   else {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid non-terminal number: %u\n", symbolNumber);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown symbol type: %d\n", symbolType);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

/*
*---------------------------------------------------------------------
* Take a symbol code and return its number (1 onwards)
*---------------------------------------------------------------------
*/

static int symbolCode2symbolNumber (t_symbolCode symbolCode)
{
 if (symbolCode == EPSILON_CODE) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Epsilon does not have a symbol number\n");
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (symbolCode2symbolType (symbolCode) == t_terminal)
   return ((int) (symbolCode - TERMINAL_START_CODE));
 else
   return ((int) (symbolCode - NON_TERMINAL_START_CODE));
}

/*
*----------------------------------------------------------------------------
* Take a symbol code and return its type (terminal, non-terminal or epsilon)
*----------------------------------------------------------------------------
*/

t_symbolType symbolCode2symbolType (t_symbolCode symbolCode)
{
 if (symbolCode == end_of_input_code)
   return (t_endOfInput);
 if (symbolCode == EPSILON_CODE)
   return (t_epsilon);
 if (((int) symbolCode >= (int) TERMINAL_START_CODE) &&
     ((int) symbolCode <= (int) (TERMINAL_START_CODE + totTerminals - 1)))
   return (t_terminal);
 if (((int) symbolCode >= (int) NON_TERMINAL_START_CODE) &&
     ((int) symbolCode <= (int) (NON_TERMINAL_START_CODE + totNonTerminals - 1)))
   return (t_nonTerminal);
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid symbol code: %d\n", symbolCode);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

/*
*---------------------------------------------------------------------
* Take a symbol code and return a pointer to its string
*---------------------------------------------------------------------
*/

const char *symbolCode2symbolString (t_symbolCode symbolCode)
{
 t_symbolType
   symbolType;
 unsigned int
   countSymbol;

 if (symbolCode == end_of_input_code)
   return (endOfInputString);
 if (symbolCode == EPSILON_CODE)
   return (epsilonString);
 symbolType = symbolCode2symbolType (symbolCode);
 if (symbolType == t_terminal) {
   for (countSymbol = 0; countSymbol < totTerminals; countSymbol++)
     if (terminals[countSymbol].symbolCode == symbolCode)
       return (&symbolNames[terminals[countSymbol].posFirstChar]);
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Could not find terminal string (code %d)\n", symbolCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (symbolType == t_nonTerminal) {
   for (countSymbol = 0; countSymbol < totNonTerminals; countSymbol++)
     if (nonTerminals[countSymbol].symbolCode == symbolCode)
       return (&symbolNames[nonTerminals[countSymbol].posFirstChar]);
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Could not find non-terminal string (code %d)\n", symbolCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown symbol type: %d\n", symbolType);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

/*
*--------------------------------------------------------------------------
* Take a rule number (1 onwards) and return its size, ie.
* the number of symbols on its righthand side
*--------------------------------------------------------------------------
*/

unsigned int ruleNumber2ruleSize (t_ruleNumber ruleNumber)
{
 if ((ruleNumber < 1) || (ruleNumber > totRules)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid rule number: %u\n", ruleNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (grammarRules[ruleNumber-1].righthandSize);
}

/*
*--------------------------------------------------------------------------------------
* Take a rule number (1 onwards) and position and return the corresponding symbol code
* Position 0 means the non-terminal on lefthand side of rule
* Position 1 means the first symbol on righthand side of rule
*--------------------------------------------------------------------------------------
*/

t_symbolCode rulePos2symbolCode (t_ruleNumber ruleNumber, unsigned int rulePos)
{
 t_ruleData
   *p_ruleData;

 if ((ruleNumber < 1) || (ruleNumber > totRules)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid rule number: %u\n", ruleNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 p_ruleData = &grammarRules[ruleNumber - 1];
 if (rulePos > p_ruleData->righthandSize) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid position %u in rule %u; rule size = %u\n", rulePos, ruleNumber, p_ruleData->righthandSize);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (rulePos == 0)
   return (p_ruleData->lefthandSymbol);
 if ((rulePos == 1) && (p_ruleData->righthandSize == 0))
   return (EPSILON_CODE);
 return (righthandSides[p_ruleData->posFirstSymbol + rulePos -1]);
}

/*
*--------------------------------------------------------------------------
* Take a rule number (1 onwards) and return a boolean saying whether
* this is a rule of the form X->epsilon
*--------------------------------------------------------------------------
*/

bool isEpsilonRule (t_ruleNumber ruleNumber)
{
 if ((ruleNumber < 1) || (ruleNumber > totRules)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid rule number: %u\n", ruleNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return ((bool) (grammarRules[ruleNumber-1].righthandSize == 0));
}

/*
*--------------------------------------------------------------------------
* Take a symbol code and a given rule position (left or right hand side)
* and return the number of rules where the symbol occurs in that position
*--------------------------------------------------------------------------
*/

unsigned int symbolCode2totUses (t_symbolCode symbolCode, t_whereInRule whereInRule)
{
 t_symbolType
   symbolType;
 t_symbolData
   *p_symbolData;

 symbolType = symbolCode2symbolType (symbolCode);
 if (whereInRule == t_lefthand) {
   if (symbolType == t_terminal) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Terminals never occur on the lefthand side of rules\n");
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     return (0);
   }
   if (symbolType == t_endOfInput) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "End-of-input never occurs on the lefthand side of rules\n");
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     return (0);
   }
   if (symbolType == t_epsilon) {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Epsilon never occurs on the lefthand side of rules\n");
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }
 if (symbolType == t_epsilon)
   p_symbolData = &epsilonUsage;
 else if ((symbolType == t_terminal) || (symbolType == t_endOfInput))
   p_symbolData = &terminals[symbolCode - TERMINAL_START_CODE];
 else if ((symbolType == t_nonTerminal) || (symbolType == t_endOfInput))
   p_symbolData = &nonTerminals[symbolCode - NON_TERMINAL_START_CODE];
 else {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown symbol type: %d\n", symbolType);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 if (whereInRule == t_lefthand)
   return (p_symbolData->totLefthandUses);
 else if (whereInRule == t_righthand)
   return (p_symbolData->totRighthandUses);
 else {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown position: %d\n", whereInRule);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
}

/*
*--------------------------------------------------------------------------
* Take a symbol code, a given rule position (left or right hand side) and
* an ordinal for the use number (1 onwards) and return the number of the
* corresponding rule
*--------------------------------------------------------------------------
*/

t_ruleNumber symbolCode2use (t_symbolCode symbolCode, t_whereInRule whereInRule, unsigned int useNumber)
{
 t_symbolType
   symbolType;
 t_symbolData
   *p_symbolData;
 unsigned int
   totUses;

 symbolType = symbolCode2symbolType (symbolCode);
 if (whereInRule == t_lefthand) {
   if (symbolType == t_terminal) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Terminals never occur on the lefthand side of rules\n");
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     return (0);
   }
   if (symbolType == t_endOfInput) {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "End-of-input never occurs on the lefthand side of rules\n");
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     return (0);
   }
   if (symbolType == t_epsilon) {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Epsilon never occurs on the lefthand side of rules\n");
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }
 if (symbolType == t_epsilon)
   p_symbolData = &epsilonUsage;
 else if ((symbolType == t_terminal) || (symbolType == t_endOfInput))
   p_symbolData = &terminals[symbolCode - TERMINAL_START_CODE];
 else if ((symbolType == t_nonTerminal) || (symbolType == t_endOfInput))
   p_symbolData = &nonTerminals[symbolCode - NON_TERMINAL_START_CODE];
 else {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown symbol type: %d\n", symbolType);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 if (whereInRule == t_lefthand)
   totUses = p_symbolData->totLefthandUses;
 else if (whereInRule == t_righthand)
   totUses = p_symbolData->totRighthandUses;
 else {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown position: %d\n", whereInRule);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (useNumber > totUses) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid use number (%u) for symbol %d [%s]\n", useNumber, symbolCode, symbolCode2symbolString (symbolCode));
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (whereInRule == t_lefthand)
   return ((t_ruleNumber) (p_symbolData->lefthandUses[useNumber-1] + 1));
 else
   return ((t_ruleNumber) (p_symbolData->righthandUses[useNumber-1] + 1));
}

/*
*--------------------------------------------------------------------------
* Take a rule number and a dot rule position (0 onwards),
* create the corresponding LR(0) item if it doesn't exist,
* and return the item's numeric code
*--------------------------------------------------------------------------
*/

static t_itemCode newItem (t_ruleNumber ruleNumber, t_dotPosition dotPosition)
{
 unsigned int
   iItem;

 if (ruleNumber > totRules) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to create LR(0) item {%u,%u}: invalid rule number\n", ruleNumber, dotPosition);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (dotPosition > grammarRules[ruleNumber-1].righthandSize) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to create LR(0) item {%u,%u}: invalid dot position\n", ruleNumber, dotPosition);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (nextLR0item == MAX_LR0_ITEMS) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to create LR(0) item {%u,%u}: array overflow\n", ruleNumber, dotPosition);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Check if this LR(0) item has been created before */

 for (iItem = 0; iItem < nextLR0item; iItem++)
   if ( (LR0items[iItem].ruleNumber ==  ruleNumber) &&
        (LR0items[iItem].dotPosition == dotPosition) )
     return ((t_itemCode) (iItem + ITEM_START_CODE));

 /* This is a new item; create it and return its numeric code */

 LR0items[nextLR0item].ruleNumber  = ruleNumber;
 LR0items[nextLR0item].dotPosition = dotPosition;
 totLR0items++;
 totLR0itemSymbols += (grammarRules[ruleNumber-1].righthandSize + 3);  /* 3 = lefthand non-terminal + rule arrow + item dot */
 return ((t_itemCode) ((nextLR0item++) + ITEM_START_CODE));
}

/*
*----------------------------------------------------------------------------
* Take an LR(0) item number (1 onwards) and return its numeric code
*----------------------------------------------------------------------------
*/

t_itemCode itemNumber2itemCode (unsigned int itemNumber)
{
 if ((itemNumber < 1) || (itemNumber > nextLR0item)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid item number %u\n", itemNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return ((t_itemCode) (ITEM_START_CODE + itemNumber - 1));
}

/*
*----------------------------------------------------------------------------
* Take an LR(0) item's numeric code and return the corresponding rule number
*----------------------------------------------------------------------------
*/

t_ruleNumber itemCode2ruleNumber (t_itemCode itemCode)
{
 int
   itemIndex;

 itemIndex = itemCode - ITEM_START_CODE;
 if ((itemIndex < 0) || (itemIndex >= MAX_LR0_ITEMS)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid LR(0) item code %d\n", itemCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (LR0items[itemIndex].ruleNumber);
}

/*
*----------------------------------------------------------------------------
* Take an LR(0) item's numeric code and return the corresponding dot position
*----------------------------------------------------------------------------
*/

t_dotPosition itemCode2dotPosition (t_itemCode itemCode)
{
 int
   itemIndex;

 itemIndex = itemCode - ITEM_START_CODE;
 if ((itemIndex < 0) || (itemIndex >= (int) nextLR0item)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid LR(0) item code %d\n", itemCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (LR0items[itemIndex].dotPosition);
}

/*
*----------------------------------------------------------------------------------------------
* Take a rule number and a dot position and return the corresponding LR(0) item's numeric code
*----------------------------------------------------------------------------------------------
*/

t_itemCode ruleNumberDotPosition2itemCode (t_ruleNumber ruleNumber, t_dotPosition dotPosition)
{
 unsigned int
   iItem;
 t_LR0item
   *pItem;

 if ((ruleNumber < 1) || (ruleNumber > totRules)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid rule number %u\n", ruleNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (dotPosition > grammarRules[ruleNumber-1].righthandSize) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid dot position %u\n", dotPosition);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 for (iItem = 0, pItem = LR0items; iItem < nextLR0item; iItem++, pItem++) {
   if (pItem->ruleNumber != ruleNumber)
     continue;
   if (pItem->dotPosition != dotPosition)
     continue;
   return ((t_itemCode) (iItem + ITEM_START_CODE));
 }
 return ((t_itemCode) UNKNOWN_ITEM_CODE);
}

/*
*----------------------------------------------------------------------------
* Take an LR(0) item's numeric code and return the numeric code of the
* corresponding transition symbol, i.e. the grammar symbol immediately
* after the dot
*----------------------------------------------------------------------------
*/

t_symbolCode itemCode2transitionSymbol (t_itemCode itemCode)
{
 t_ruleNumber
   ruleNumber;

 ruleNumber = itemCode2ruleNumber (itemCode);
 if (isEpsilonRule (ruleNumber))
   return (EPSILON_CODE);
 else
   return (rulePos2symbolCode (ruleNumber, (unsigned int) 1 + itemCode2dotPosition (itemCode)));
}

/*----------------------------------------------------------------------------
* Take an LR(0) item's numeric code and return a boolean indicating whether
* it is a reduction item, i.e. whether the dot is at the end of the item
*----------------------------------------------------------------------------
*/

bool isReductionItem (t_itemCode itemCode)
{
 int
   itemIndex;
 t_LR0item
   *p_LR0item;

 itemIndex = itemCode - ITEM_START_CODE;
 if ((itemIndex < 0) || (itemIndex > (int) nextLR0item)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid LR(0) item code %d\n", itemCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 p_LR0item = &LR0items[itemIndex];
 return ((bool) (ruleNumber2ruleSize (p_LR0item->ruleNumber) == p_LR0item->dotPosition));
}

/*
*------------------------------------------------------------------------------
* Take a state code and return the type of automaton it belongs to (NFA or DFA)
*------------------------------------------------------------------------------
*/

static t_FSA_type stateCode2fsaType (t_stateCode stateCode)
{
 if ( ((int) stateCode >= (int)  NFA_STATE_START_CODE) &&
      ((int) stateCode <= (int) (NFA_STATE_START_CODE + totNFAstates - 1)) )
   return (t_NFA);
 if ( ((int) stateCode >= (int)  DFA_STATE_START_CODE) &&
      ((int) stateCode <= (int) (DFA_STATE_START_CODE + totDFAstates - 1)) )
   return (t_DFA);
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid state code: %d\n", stateCode);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

/*
*---------------------------------------------------------------------------
* Ensure a given state really belongs to the expected FSA type (NFA or DFA)
*---------------------------------------------------------------------------
*/

static void validate_state_FSA_type (t_stateCode stateCode, t_FSA_type expected_fsaType, char *ERROR_errorMsg)
{
 t_FSA_type
   state_fsaType;

 state_fsaType = stateCode2fsaType(stateCode);
 if ((state_fsaType != t_NFA) & (state_fsaType != t_DFA)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "State %d of unknown automaton type %d\n", stateCode, state_fsaType);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 if (state_fsaType != expected_fsaType)
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_errorMsg);
}

/*
*------------------------------------------------------------------------------
* Take a state code and return its type (shift, reduce, etc)
*------------------------------------------------------------------------------
*/

t_stateType stateCode2stateType (t_stateCode stateCode)
{
 if ( ((int) stateCode >= (int)  NFA_STATE_START_CODE) &&
      ((int) stateCode <= (int) (NFA_STATE_START_CODE + totNFAstates - 1)) )
   return (NFAstates[stateCode - NFA_STATE_START_CODE].stateType);
 if ( ((int) stateCode >= (int)  DFA_STATE_START_CODE) &&
      ((int) stateCode <= (int) (DFA_STATE_START_CODE + totDFAstates - 1)) )
   return (DFAstates[stateCode - DFA_STATE_START_CODE].stateType);
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid state code: %d\n", stateCode);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

/*
*------------------------------------------------------------------------------
* Take a state type and return a pointer to a descriptive string
*------------------------------------------------------------------------------
*/

const char *stateType2stateTypeString (t_stateType stateType)
{
 if ( ((int) stateType >= (int) FIRST_STATE_TYPE) &&
      ((int) stateType <= (int)  LAST_STATE_TYPE) )
   return ((char *) stateTypeString[stateType]);
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid state type: %d\n", stateType);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

/*
*--------------------------------------------------------------------------
* Create an empty NFA state (no items in it) and return its numeric code
*--------------------------------------------------------------------------
*/

static t_stateCode nfa_newEmptyState (void)
{
 t_NFAstate
   *p_NFAstate;

 if (nfa_nextState == MAX_NFA_STATES) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to create new NFA state: array overflow\n");
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 p_NFAstate = &NFAstates[nfa_nextState];
 p_NFAstate->itemIndex = UNKNOWN_ITEM_INDEX;
 p_NFAstate->totInwardTransitions = p_NFAstate->totOutwardTransitions = 0;
 totNFAstates++;
 return ((t_stateCode) ((nfa_nextState++) + NFA_STATE_START_CODE));
}

/*
*--------------------------------------------------------------------------
* Create an empty DFA state (no items in it) and return its numeric code
*--------------------------------------------------------------------------
*/

static t_stateCode dfa_newEmptyState (void)
{
 int
   iSymbol,
   iItem;
 t_DFAstate
   *p_DFAstate;

 if (dfa_nextState == MAX_DFA_STATES) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to create new DFA state: array overflow\n");
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 p_DFAstate = &DFAstates[dfa_nextState];
 p_DFAstate->itemsInState = 0;
 for (iItem = 0; iItem < MAX_ITEMS_PER_DFA_STATE; iItem++)
   p_DFAstate->itemIndex[iItem] = UNKNOWN_ITEM_INDEX;
 p_DFAstate->totInwardTransitions = p_DFAstate->totTransitionSymbols = 0;
 for (iSymbol = 0; iSymbol < MAX_TRANSITIONS_PER_DFA_STATE; iSymbol++)
   p_DFAstate->transitionSymbols[iSymbol] = UNKNOWN_SYMBOL_CODE;
 totDFAstates++;
 return ((t_stateCode) ((dfa_nextState++) + DFA_STATE_START_CODE));
}

/*
*--------------------------------------------------------------------------
* Add an existing LR(0) item to an existing NFA state.
* Return true if the item was not present in state and was actually added.
* Return false otherwise.
*--------------------------------------------------------------------------
*/

static bool nfa_addItemToState (t_itemCode itemCode, t_stateCode stateCode)
{
 t_LR0item
   *p_LR0item;
 t_NFAstate
   *p_NFAstate;

 /* Ensure the state code provided really is a NFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Attempt to add item %d to DFA state %d; NFA state expected\n", itemCode, stateCode);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 p_NFAstate = &NFAstates[stateCode - NFA_STATE_START_CODE];

 /* The item may already be in this state */

 if ((int) p_NFAstate->itemIndex == (int) itemCode - ITEM_START_CODE)
   return (false);

 /* The item is not in this state yet, but we can only add it if the state is still empty */

 if (! nfa_isEmptyState (stateCode)) {
   p_LR0item = &LR0items[itemCode - ITEM_START_CODE];
   snprintf (
     ERROR_auxErrorMsg,
   ERROR_maxErrorMsgSize,
     "Unable to add LR(0) item %d {%u,%u} to NFA state %d: state is not empty\n",
     itemCode,
     p_LR0item->ruleNumber,
     p_LR0item->dotPosition,
     stateCode );
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* The state is empty, so add the item */

 p_NFAstate->itemIndex = itemCode - ITEM_START_CODE;
 p_NFAstate->totInwardTransitions = p_NFAstate->totOutwardTransitions = 0;
 return (true);
}

/*
*--------------------------------------------------------------------------
* Add an existing LR(0) item to an existing DFA state.
* Return true if the item was not present in state and was actually added.
* Return false otherwise.
*--------------------------------------------------------------------------
*/

static bool dfa_addItemToState (t_itemCode itemCode, t_stateCode stateCode)
{
 unsigned int
   iSymbol,
   iItem;
 t_symbolCode
   transitionSymbol;
 t_LR0item
   *p_LR0item;
 t_DFAstate
   *p_DFAstate;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Attempt to add item %d to NFA state %d; DFA state expected\n", itemCode, stateCode);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 p_DFAstate = &DFAstates[stateCode - DFA_STATE_START_CODE];

 /* This item may already be in this state */

 for (iItem = 0; iItem < p_DFAstate->itemsInState; iItem++)
   if ((int) p_DFAstate->itemIndex[iItem] == (int) itemCode - ITEM_START_CODE)
     return (false);

 /* This item is not in this state yet, so add it */

 if (p_DFAstate->itemsInState == MAX_ITEMS_PER_DFA_STATE) {
   p_LR0item = &LR0items[itemCode - ITEM_START_CODE];
   snprintf (
     ERROR_auxErrorMsg,
   ERROR_maxErrorMsgSize,
     "Unable to add LR(0) item %d {%u,%u} to state %d: array overflow\n",
     itemCode,
     p_LR0item->ruleNumber,
     p_LR0item->dotPosition,
     stateCode );
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 p_DFAstate->itemIndex[p_DFAstate->itemsInState++] = itemCode - ITEM_START_CODE;

 /* Ensure that itemIndex array is always sorted */
 
 /*
 qsort (
   (void *)                              &p_DFAstate->itemIndex[0],
   (size_t)                              p_DFAstate->itemsInState,
   (size_t)                              sizeof (p_DFAstate->itemIndex[0]),
   (int (*)(const void *, const void *)) compare_LR0items );
*/

 /* If this is a reduction item then do nothing else  */
 /* (no transition is possible from a reduction item) */

 if (isReductionItem (itemCode))
   return (true);

 /* May need to update transition symbols list */

 transitionSymbol = itemCode2transitionSymbol (itemCode);
 for (iSymbol = 0; iSymbol < p_DFAstate->totTransitionSymbols; iSymbol++)
   if (p_DFAstate->transitionSymbols[iSymbol] == transitionSymbol)
     return (true);

 /* Indeed a new transition symbol in this state, so add it to the list */

 p_DFAstate->transitionSymbols[p_DFAstate->totTransitionSymbols++] = transitionSymbol;

 /* Ensure that transitionSymbols array is always sorted */

 qsort (
   (void *)                              &p_DFAstate->transitionSymbols[0],
   (size_t)                              p_DFAstate->totTransitionSymbols,
   (size_t)                              sizeof (p_DFAstate->transitionSymbols[0]),
   (int (*)(const void *, const void *)) compare_symbolCodes );

 return (true);
}

/*
*----------------------------------------------------------------------------
* Add new NFA state transition if it doesn't exist yet
*----------------------------------------------------------------------------
*/

static void nfa_newTransition (t_stateCode fromState, t_symbolCode withSymbol, t_stateCode toState)
{
 unsigned int
   iTransition;
 t_stateTransition
   *p_NFAtransition;
 t_symbolData
   *p_symbolData;
 t_symbolType
   symbolType;

 /* Make sure the state codes provided really are NFA states */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Attempt to add to an NFA a transition from DFA state %d\n", fromState);
 validate_state_FSA_type (fromState, t_NFA, ERROR_auxErrorMsg);
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Attempt to add to an NFA a transition to DFA state %d\n", toState);
 validate_state_FSA_type (toState, t_NFA, ERROR_auxErrorMsg);

 /* Make sure the symbol code is valid */

 symbolType = symbolCode2symbolType (withSymbol);
 switch (symbolType) {
   case (t_epsilon)    : p_symbolData = &epsilonUsage;                                       break;
   case (t_endOfInput) : p_symbolData = &endOfInputUsage;                                    break;
   case (t_terminal)   : p_symbolData = &terminals   [withSymbol -     TERMINAL_START_CODE]; break;
   case (t_nonTerminal): p_symbolData = &nonTerminals[withSymbol - NON_TERMINAL_START_CODE]; break;
   default:
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Symbol %d has invalid type %d\n", withSymbol, symbolType);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Make sure we don't add an existing transition twice */

 for (iTransition = 0; iTransition < totNFAtransitions; iTransition++) {
   p_NFAtransition = &NFAtransitions[iTransition];
   if ((p_NFAtransition->fromState  == fromState ) &&
       (p_NFAtransition->withSymbol == withSymbol) &&
       (p_NFAtransition->toState    == toState   ) )
     return;
 }

 /* This is not a duplicate transition, so add it */

 if (nextNFAtransition == MAX_NFA_TRANSITIONS) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to create new NFA transition: array overflow\n");
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 p_NFAtransition = &NFAtransitions[nextNFAtransition++];
 p_NFAtransition->fromState  = fromState;
 p_NFAtransition->withSymbol = withSymbol;
 p_NFAtransition->toState    = toState;

 NFAstates[fromState - NFA_STATE_START_CODE].totOutwardTransitions++;
 p_symbolData->totNFAtransitionsWithSymbol++;
 NFAstates[toState - NFA_STATE_START_CODE].totInwardTransitions++;
 totNFAtransitions++;
}

/*
*----------------------------------------------------------------------------
* Add new DFA state transition if it doesn't exist yet
*----------------------------------------------------------------------------
*/

static void dfa_newTransition (t_stateCode fromState, t_symbolCode withSymbol, t_stateCode toState )
{
 unsigned int
   iTransition;
 t_stateTransition
   *p_DFAtransition;
 t_symbolData
   *p_symbolData;
 t_symbolType
   symbolType;

 /* Make sure the state codes provided really are DFA states */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Attempt to add to a DFA a transition from NFA state %d\n", fromState);
 validate_state_FSA_type (fromState, t_DFA, ERROR_auxErrorMsg);
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Attempt to add to a DFA a transition to NFA state %d\n", toState);
 validate_state_FSA_type (toState, t_DFA, ERROR_auxErrorMsg);

 /* Make sure the symbol code is valid */

 symbolType = symbolCode2symbolType (withSymbol);
 switch (symbolType) {
   case (t_terminal)   : p_symbolData = &terminals   [symbolCode2symbolNumber (withSymbol)]; break;
   case (t_nonTerminal): p_symbolData = &nonTerminals[symbolCode2symbolNumber (withSymbol)]; break;
   case (t_epsilon)    : p_symbolData = &epsilonUsage;                                       break;
   case (t_endOfInput) : p_symbolData = &endOfInputUsage;                                    break;
   default:
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Symbol %d has invalid type %d\n", withSymbol, symbolType);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Make sure we don't add an existing transition twice */

 for (iTransition = 0; iTransition < totDFAtransitions; iTransition++) {
   p_DFAtransition = &DFAtransitions[iTransition];
   if ((p_DFAtransition->fromState  == fromState ) &&
       (p_DFAtransition->withSymbol == withSymbol) &&
       (p_DFAtransition->toState    == toState   ) )
     return;
 }

 /* This is not a duplicate transition, so add it */

 if (nextDFAtransition == MAX_DFA_TRANSITIONS) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to create new DFA transition: array overflow\n");
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 p_DFAtransition = &DFAtransitions[nextDFAtransition++];
 p_DFAtransition->fromState  = fromState;
 p_DFAtransition->withSymbol = withSymbol;
 p_DFAtransition->toState    = toState;

 p_symbolData->totDFAtransitionsWithSymbol++;
 DFAstates[toState - DFA_STATE_START_CODE].totInwardTransitions++;
 totDFAtransitions++;
}

/*
*----------------------------------------------------------------------------
* Sort array of NFA state transitions using sort key provided
*----------------------------------------------------------------------------
*/

void nfa_sortTransitions (t_transitionSortKey sortKey)
{
 switch (sortKey) {
   case (t_transitionSortKey_origin):
     qsort (
       (void *)                              &NFAtransitions[0],
       (size_t)                              totNFAtransitions,
       (size_t)                              sizeof (t_stateTransition),
       (int (*)(const void *, const void *)) compare_transitions_key_origin );
     break;
   case (t_transitionSortKey_symbol):
     qsort (
       (void *)                              &NFAtransitions[0],
       (size_t)                              totNFAtransitions,
       (size_t)                              sizeof (t_stateTransition),
       (int (*)(const void *, const void *)) compare_transitions_key_symbol );
     break;
   case (t_transitionSortKey_destination):
     qsort (
       (void *)                              &NFAtransitions[0],
       (size_t)                              totNFAtransitions,
       (size_t)                              sizeof (t_stateTransition),
       (int (*)(const void *, const void *)) compare_transitions_key_destination );
    break;
   default:
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid transition sort key %d\n", sortKey);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
}

/*
*----------------------------------------------------------------------------
* Sort array of DFA state transitions using sort key provided
*----------------------------------------------------------------------------
*/

void dfa_sortTransitions (t_transitionSortKey sortKey)
{
 switch (sortKey) {
   case (t_transitionSortKey_origin):
     qsort (
       (void *)                              &DFAtransitions[0],
       (size_t)                              totDFAtransitions,
       (size_t)                              sizeof (t_stateTransition),
       (int (*)(const void *, const void *)) compare_transitions_key_origin );
     break;
   case (t_transitionSortKey_symbol):
     qsort (
       (void *)                              &DFAtransitions[0],
       (size_t)                              totDFAtransitions,
       (size_t)                              sizeof (t_stateTransition),
       (int (*)(const void *, const void *)) compare_transitions_key_symbol );
     break;
   case (t_transitionSortKey_destination):
     qsort (
       (void *)                              &DFAtransitions[0],
       (size_t)                              totDFAtransitions,
       (size_t)                              sizeof (t_stateTransition),
       (int (*)(const void *, const void *)) compare_transitions_key_destination );
    break;
   default:
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid transition sort key %d\n", sortKey);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
}

/*
*-------------------------------------------------------------------------------
* Take a NFA state transition number (1 onwards) and return the state of origin
*-------------------------------------------------------------------------------
*/

t_stateCode nfa_transitionNumber2originState (unsigned int transitionNumber)
{
 if (transitionNumber > nextNFAtransition) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid NFA transition number %u\n", transitionNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (NFAtransitions[transitionNumber - 1].fromState);
}

/*
*-------------------------------------------------------------------------------
* Take a DFA state transition number (1 onwards) and return the state of origin
*-------------------------------------------------------------------------------
*/

t_stateCode dfa_transitionNumber2originState (unsigned int transitionNumber)
{
 if (transitionNumber > nextDFAtransition) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid DFA transition number %u\n", transitionNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (DFAtransitions[transitionNumber - 1].fromState);
}

/*
*---------------------------------------------------------------------------------
* Take a NFA state transition number (1 onwards) and return the transition symbol
*---------------------------------------------------------------------------------
*/

t_symbolCode nfa_transitionNumber2symbol (unsigned int transitionNumber)
{
 if (transitionNumber > nextNFAtransition) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid NFA transition number %u\n", transitionNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (NFAtransitions[transitionNumber - 1].withSymbol);
}

/*
*---------------------------------------------------------------------------------
* Take a DFA state transition number (1 onwards) and return the transition symbol
*---------------------------------------------------------------------------------
*/

t_symbolCode dfa_transitionNumber2symbol (unsigned int transitionNumber)
{
 if (transitionNumber > nextDFAtransition) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid DFA transition number %u\n", transitionNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (DFAtransitions[transitionNumber - 1].withSymbol);
}

/*
*---------------------------------------------------------------------------------
* Take a NFA state transition number (1 onwards) and return the destination state
*---------------------------------------------------------------------------------
*/

t_stateCode nfa_transitionNumber2destState (unsigned int transitionNumber)
{
  if (transitionNumber > nextNFAtransition) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid NFA transition number %u\n", transitionNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (NFAtransitions[transitionNumber - 1].toState);
}

/*
*---------------------------------------------------------------------------------
* Take a DFA state transition number (1 onwards) and return the destination state
*---------------------------------------------------------------------------------
*/

t_stateCode dfa_transitionNumber2destState (unsigned int transitionNumber)
{
 if (transitionNumber > nextDFAtransition) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid DFA transition number %u\n", transitionNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (DFAtransitions[transitionNumber - 1].toState);
}

/*
*---------------------------------------------------------------------------------
* Take a symbol code and return the total number of NFA transitions on this symbol
*---------------------------------------------------------------------------------
*/

unsigned int nfa_symbolCode2totTransitionsWithSymbol (t_symbolCode symbolCode)
{
 t_symbolData
   *p_symbolData;
 t_symbolType
   symbolType;

 symbolType = symbolCode2symbolType (symbolCode);
 switch (symbolType) {
   case (t_terminal)   : p_symbolData = &terminals   [symbolCode2symbolNumber (symbolCode)]; break;
   case (t_nonTerminal): p_symbolData = &nonTerminals[symbolCode2symbolNumber (symbolCode)]; break;
   case (t_epsilon)    : p_symbolData = &epsilonUsage;                                       break;
   case (t_endOfInput) : p_symbolData = &endOfInputUsage;                                    break;
   default:
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Symbol %d has invalid type %d\n", symbolCode, symbolType);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (p_symbolData->totNFAtransitionsWithSymbol);
}

/*
*---------------------------------------------------------------------------------
* Take a symbol code and return the total number of DFA transitions on this symbol
*---------------------------------------------------------------------------------
*/

unsigned int dfa_symbolCode2totTransitionsWithSymbol (t_symbolCode symbolCode)
{
 t_symbolData
   *p_symbolData;
 t_symbolType
   symbolType;

 symbolType = symbolCode2symbolType (symbolCode);
 switch (symbolType) {
   case (t_terminal)   : p_symbolData = &terminals   [symbolCode2symbolNumber (symbolCode)]; break;
   case (t_nonTerminal): p_symbolData = &nonTerminals[symbolCode2symbolNumber (symbolCode)]; break;
   case (t_epsilon)    : p_symbolData = &epsilonUsage;                                       break;
   case (t_endOfInput) : p_symbolData = &endOfInputUsage;                                    break;
   default:
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Symbol %d has invalid type %d\n", symbolCode, symbolType);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (p_symbolData->totDFAtransitionsWithSymbol);
}

/*
*----------------------------------------------------------------------------
* Take a NFA state number (1 onwards) and return its numeric code
*----------------------------------------------------------------------------
*/

t_stateCode nfa_stateNumber2stateCode (unsigned int stateNumber)
{
 if ((stateNumber < 1) || (stateNumber > nfa_nextState)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid state number %u\n", stateNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return ((t_stateCode) (NFA_STATE_START_CODE + stateNumber - 1));
}

/*
*----------------------------------------------------------------------------
* Take a NFA state numeric code and return its state number (1 onwards)
*----------------------------------------------------------------------------
*/

unsigned int nfa_stateCode2stateNumber (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a NFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d passed to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);
 
 return ((unsigned int) (stateCode - NFA_STATE_START_CODE + 1));
}

/*
*----------------------------------------------------------------------------
* Take a DFA state number (1 onwards) and return its numeric code
*----------------------------------------------------------------------------
*/

t_stateCode dfa_stateNumber2stateCode (unsigned int stateNumber)
{
 if ((stateNumber < 1) || (stateNumber > dfa_nextState)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid state number %u\n", stateNumber);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return ((t_stateCode) (DFA_STATE_START_CODE + stateNumber - 1));
}

/*
*----------------------------------------------------------------------------
* Take a DFA state numeric code and return its state number (1 onwards)
*----------------------------------------------------------------------------
*/

unsigned int dfa_stateCode2stateNumber (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);
 
 return ((unsigned int) (stateCode - DFA_STATE_START_CODE + 1));
}

/*
*----------------------------------------------------------------------------
* Take a DFA state code and return a boolean indicating whether
* it is a duplicate of an existing state
*----------------------------------------------------------------------------
*/

static bool dfa_isDuplicateState (t_stateCode stateCode)
{
 int
   iState,
   iIndex,
   iTransition,
   stateIndex;
 t_DFAstate
   *p_DFAstate,
   *p_state;
 bool
   isDuplicate;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d provided to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 stateIndex = stateCode - DFA_STATE_START_CODE;
 p_DFAstate = &DFAstates[stateIndex];
 for (iState = 0; iState < (int) dfa_nextState; iState++) {
   if (iState == stateIndex)
     break;
   p_state = &DFAstates[iState];
   isDuplicate = (bool) (p_state->itemsInState == p_DFAstate->itemsInState);
   if (isDuplicate)
     for (iIndex = 0; iIndex < (int) p_state->itemsInState; iIndex++)
       isDuplicate &= (p_state->itemIndex[iIndex] == p_DFAstate->itemIndex[iIndex]);
   if (isDuplicate)
     isDuplicate &= (p_state->totTransitionSymbols == p_DFAstate->totTransitionSymbols);
   if (isDuplicate)
     for (iTransition = 0; iTransition < (int) p_state->totTransitionSymbols; iTransition++)
       isDuplicate &= (p_state->transitionSymbols[iTransition] == p_DFAstate->transitionSymbols[iTransition]);
   if (isDuplicate)
     return (true);
 }
 return (false);
}

/*
*----------------------------------------------------------------------------
* Take a NFA state code and return a boolean indicating whether
* it is an empty state
*----------------------------------------------------------------------------
*/

static bool nfa_isEmptyState (t_stateCode stateCode)
{
 /* Ensure the state code provided really is an NFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d provided to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 return ((bool) (DFAstates[stateCode - DFA_STATE_START_CODE].itemsInState == 0));
}

/*
*----------------------------------------------------------------------------
* Take a DFA state code and return a boolean indicating whether
* it is an empty state
*----------------------------------------------------------------------------
*/

static bool dfa_isEmptyState (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d provided to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 return ((bool) (DFAstates[stateCode - DFA_STATE_START_CODE].itemsInState == 0));
}

/*
*----------------------------------------------------------------------------
* Take a DFA state code and check if it is a duplicate of an existing state.
* If so, return the numeric code of the existing duplicate,
* otherwise return UNKNOWN_STATE_CODE
*----------------------------------------------------------------------------
*/

static t_stateCode dfa_stateCode2duplicateStateCode (t_stateCode stateCode)
{
 int
   iState,
   iIndex,
   iTransition,
   stateIndex;
 t_DFAstate
   *p_DFAstate,
   *p_state;
 bool
   isDuplicate;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d provided to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 stateIndex = stateCode - DFA_STATE_START_CODE;
 p_DFAstate = &DFAstates[stateIndex];
 for (iState = 0; iState < (int) dfa_nextState; iState++) {
   if (iState == stateIndex)
     break;
   p_state = &DFAstates[iState];
   isDuplicate = (bool) (p_state->itemsInState == p_DFAstate->itemsInState);
   if (isDuplicate)
     for (iIndex = 0; iIndex < (int) p_state->itemsInState; iIndex++)
       isDuplicate &= (p_state->itemIndex[iIndex] == p_DFAstate->itemIndex[iIndex]);
   if (isDuplicate)
     isDuplicate &= (p_state->totTransitionSymbols == p_DFAstate->totTransitionSymbols);
   if (isDuplicate)
     for (iTransition = 0; iTransition < (int) p_state->totTransitionSymbols; iTransition++)
       isDuplicate &= (p_state->transitionSymbols[iTransition] == p_DFAstate->transitionSymbols[iTransition]);
   if (isDuplicate)
     return ((t_stateCode) (iState + DFA_STATE_START_CODE));
 }
 return ((t_stateCode) UNKNOWN_STATE_CODE);
}

/*
*----------------------------------------------------------------------------
* Take a DFA state code and return the number of LR(0) items in it
*----------------------------------------------------------------------------
*/

unsigned int dfa_stateCode2totItems (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d provided to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 return (DFAstates[stateCode - DFA_STATE_START_CODE].itemsInState);
}

/*
*----------------------------------------------------------------------------
* Take a NFA state code and return the corresponding item's numeric code
*----------------------------------------------------------------------------
*/

t_itemCode nfa_stateCode2itemCode (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a NFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 return (NFAstates[stateCode - NFA_STATE_START_CODE].itemIndex + ITEM_START_CODE);
}

/*
*----------------------------------------------------------------------------
* Take a DFA state code and an LR(0) item number (1 onwards)
* and return the corresponding item's numeric code
*----------------------------------------------------------------------------
*/

t_itemCode dfa_stateCode2itemCode (t_stateCode stateCode, unsigned int itemNumber)
{
 t_DFAstate
   *p_DFAstate;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 p_DFAstate = &DFAstates[stateCode - DFA_STATE_START_CODE];
 if (itemNumber > p_DFAstate->itemsInState) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid DFA item number %u in state %d\n", itemNumber, stateCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (p_DFAstate->itemIndex[itemNumber-1] + ITEM_START_CODE);
}

/*
*----------------------------------------------------------------------------
* Take a NFA state code and return the total number of transitions
* from this state
*----------------------------------------------------------------------------
*/

unsigned int nfa_stateCode2totTransitionsFromState (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a NFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d passed to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 return (NFAstates[stateCode - NFA_STATE_START_CODE].totOutwardTransitions);
}

/*
*----------------------------------------------------------------------------
* Take a DFA state code and return the total number of transitions
* from this state
*----------------------------------------------------------------------------
*/

unsigned int dfa_stateCode2totTransitionsFromState (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 return (DFAstates[stateCode - DFA_STATE_START_CODE].totTransitionSymbols);
}

/*
*----------------------------------------------------------------------------
* Take a NFA state code and return the total number of transitions
* to this state
*----------------------------------------------------------------------------
*/

unsigned int nfa_stateCode2totTransitionsToState (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a NFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d passed to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 return (NFAstates[stateCode - NFA_STATE_START_CODE].totInwardTransitions);
}

/*
*----------------------------------------------------------------------------
* Take a DFA state code and return the total number of transitions
* to this state
*----------------------------------------------------------------------------
*/

unsigned int dfa_stateCode2totTransitionsToState (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 return (DFAstates[stateCode - DFA_STATE_START_CODE].totInwardTransitions);
}

/*
*---------------------------------------------------------------------------------
* Take a NFA state code and an ordinal (1 onwards)
* and return the corresponding transition symbol.
*
* NOTE:
* Any NFA state contains exactly one LR(0) item.
* - If the dot in this item is at the end of the rule, we have a reduction item.
*   There are no transitions from a reduction state.
* - If the dot is not at the end of the rule, then there is at least one transition from the state.
* - If the transition symbol (immediately after the dot) is a non-terminal then
*   there will also be epsilon transitions, one for each of the transition symbol's rules.
*
* Example:
*
* Suppose NFA state 5 contains the LR(0) item "E -> E + . T" and that the
* grammar contains 2 rules with the non-terminal T on the lefthand side.
*
* In this scenario:
*
* - nfa_stateCode2transitionSymbol (5, 0) causes an error, as whichSymbol
*     must be greater than 0
* - nfa_stateCode2transitionSymbol (5, 1) returns the code of non-terminal T
* - nfa_stateCode2transitionSymbol (5, 2) returns the code of epsilon
* - nfa_stateCode2transitionSymbol (5, 3) returns the code of epsilon
* - nfa_stateCode2transitionSymbol (5, 4) returns UNKNOWN_SYMBOL_CODE, as there
*     are no more transitions from this state.
*---------------------------------------------------------------------------------
*/

t_symbolCode nfa_stateCode2transitionSymbol (t_stateCode stateCode, unsigned int whichSymbol)
{
 t_symbolCode
   transitionSymbol;
 unsigned int
   totEpsilonTransitions;

 /* Ensure the state code provided really is a NFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d passed to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);
 
 /* Reduction states have no state transitions */

 if (nfa_isReductionState (stateCode)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "There are no transitions from NFA reduction state %d\n", stateCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Now we know this is not a reduction state, so there must be at least one state transition */
 /* on a terminal or non-terminal, and possibly epsilon transitions as well                   */

 if (whichSymbol < 1) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid symbol ordinal %u in NFA state %d; must be > 0\n", whichSymbol, stateCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Now we know this is not a reduction state, so there must be at least one state transition */
 /* on a terminal or non-terminal, and possibly epsilon transitions as well                   */

 transitionSymbol = itemCode2transitionSymbol (nfa_stateCode2itemCode (stateCode));
 if (whichSymbol == 1)
   return (transitionSymbol);
 totEpsilonTransitions = symbolCode2totUses (transitionSymbol, t_lefthand);
 if (whichSymbol > totEpsilonTransitions + 1) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid symbol ordinal %u; NFA state %d has %u transitions\n", whichSymbol, stateCode, totEpsilonTransitions + 1);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 return (epsilon_code);
}

/*
*-------------------------------------------------------------------------------------------------------------
* Take a NFA state code and an ordinal (1 onwards),  
* and return the corresponding transition number.
*
* NOTE:
* Any NFA state contains exactly one LR(0) item.
* - If the dot in this item is at the end of the rule, we have a reduction item.
*   There are no transitions from a reduction state.
* - If the dot is not at the end of the rule, then there is at least one transition from the state.
* - If the transition symbol (immediately after the dot) is a non-terminal then
*   there will also be epsilon transitions, one for each of the transition symbol's rules.
* IMPORTANT:
* - For epsilon transitions, the returned values will be sorted by the destination state's rule number.
*
* Example:
*
* - NFA state 5 contains the LR(0) item "E -> E + . T" 
* - The symbom code of the non-terminal T is 2
* - The transition from state 5 with the non-terminal T leads to state 6
* - The grammar contains 2 rules with the non-terminal T on the lefthand side: rules 3 and 4
* - The item for rule 3 is in state 10
* - The item for rule 4 is in state 8
* - The 2 epsilon transitions from state 5 lead to states 10 and 8
*
* In this scenario:
*
* - nfa_stateCode2transitionNumber (5, 0) causes an error, as whichTransition must be > 0
* - nfa_stateCode2transitionNumber (5, 1) returns 6  (transition with non-terminal)
* - nfa_stateCode2transitionNumber (5, 2) returns 8  (transition with epsilon; the smaller of 8 and 10)
* - nfa_stateCode2transitionNumber (5, 3) returns 10 (transition with epsilon; the larger of 8 and 10)
* - nfa_stateCode2transitionNumber (5, 4) causes an error, as there are only 3 transitions from this state
*-------------------------------------------------------------------------------------------------------------
*/
typedef struct {
  unsigned int transitionNumber;
  t_ruleNumber transitionDestRuleNumber; 
}
  t_epsilonTransition;

static int compareRuleNumbers (const void *trans1, const void *trans2) 
{
  return ( ((t_epsilonTransition *) trans1)->transitionDestRuleNumber - ((t_epsilonTransition *) trans2)->transitionDestRuleNumber);
}

unsigned int nfa_stateCode2transitionNumber (t_stateCode stateCode, unsigned int whichTransition)
{
 t_symbolCode
   transitionSymbol;
 t_symbolType
   transitionSymbolType;
 int
   iTransition,
	 epsilonTransitionsFound,
   totTransitions,
   totEpsilonTransitions;
 t_stateTransition
   *p_transition;

 /* Ensure the state code provided really is a NFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d passed to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 /* Reduction states have no state transitions */

 if (nfa_isReductionState (stateCode)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "There are no transitions from NFA reduction state %d\n", stateCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Now we know this is not a reduction state, so there must be at least one state transition */
 /* on a terminal or non-terminal, and possibly epsilon transitions as well                   */

 if (whichTransition < 1) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid transition %u in NFA state %d; must be > 0\n", whichTransition, stateCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 transitionSymbol = itemCode2transitionSymbol (nfa_stateCode2itemCode (stateCode));
 transitionSymbolType = symbolCode2symbolType (transitionSymbol);
 switch (transitionSymbolType) {
   case (t_terminal):
	 case (t_endOfInput): {
     for (iTransition = 0, p_transition = NFAtransitions; iTransition < totNFAtransitions; iTransition++, p_transition++) {
       if ((p_transition->fromState == stateCode) && (p_transition->withSymbol == transitionSymbol))
         return (iTransition + 1);
     }
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to find transition from NFA state %d with symbol %u\n", stateCode, transitionSymbol);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
	 }
   case (t_nonTerminal): {
     totEpsilonTransitions = symbolCode2totUses (transitionSymbol, t_lefthand);
     totTransitions = totEpsilonTransitions + 1;
     if ((whichTransition > totTransitions)) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid transition ordinal %u; NFA state %d has only %d transitions\n", whichTransition, stateCode, totTransitions);
       ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
		 t_epsilonTransition
       epsilonTransitions [totEpsilonTransitions];
     epsilonTransitionsFound = 0;
     for (iTransition = 0, p_transition = NFAtransitions; iTransition < totNFAtransitions; iTransition++, p_transition++) {
       if (p_transition->fromState != stateCode)
         continue;
       if ((p_transition->withSymbol == transitionSymbol) && (whichTransition == 1)) {
			   return (iTransition + 1);
			 }
       if (p_transition->withSymbol == epsilon_code) {
			   epsilonTransitions[epsilonTransitionsFound].transitionNumber = iTransition + 1;
				 epsilonTransitions[epsilonTransitionsFound].transitionDestRuleNumber = itemCode2ruleNumber (nfa_stateCode2itemCode (p_transition->toState));
				 epsilonTransitionsFound++;
			   continue;
       }
     }
     if (epsilonTransitionsFound != totEpsilonTransitions) {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Expected %d epsilon transitions from NFA state %d, found %d\n", totEpsilonTransitions, stateCode, epsilonTransitionsFound);
       ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
		 /* Sort transitions by rule number */
		 qsort (epsilonTransitions, epsilonTransitionsFound, sizeof (epsilonTransitions[0]), compareRuleNumbers);
		 return (epsilonTransitions[whichTransition-2].transitionNumber);
	 }
   case (t_epsilon): {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid transition symbol type %d\n", transitionSymbolType);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     break;
	 }
   default: {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown transition symbol type %d\n", transitionSymbolType);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
	 }
 }
 if (iTransition == totNFAtransitions) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to find transition ordinal %d from NFA state %d\n", whichTransition, stateCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
}

/*
*-------------------------------------------------------------------------------------------------------------
* Take a DFA state code and an ordinal (1 onwards),  
* and return the corresponding transition symbol.
*
* NOTE:
* Any DFA state contains one or more LR(0) item.
* - If the state contains only one item and if the dot is at the end of the item, we have a reduction state.
*   There are no transitions from a reduction state.
* - Otherwise, there is at least one transition symbol in the state, possibly more.
* - For each transition symbol, there is exactly one transition.
*
* Example:
* 
*  AUGMENTED GRAMMAR:
*                                               4
*    1) E'-> E $                    #============#
*    2) E -> E + E                  | E'-> E $ . |
*    3) E -> E * E                  #============#
*    4) E -> id                         ^   
*                                       | $ [t3]
*                      1                |         2                          6                      8
*        +--------------+           +--------------+           +--------------+  E [t8]   ################
*        | E'-> . E $   |  E [t1]   | E'-> E . $   |  * [t5]   | E -> E * . E | --------> # E -> E * E . #
*        | E -> . E + E | --------> | E -> E . + E | --------> | E -> . E + E |           # E -> E . + E #
*        | E -> . E * E |           | E -> E . * E |           | E -> . E * E | <-------- # E -> E . * E #
*        | E -> . id    |           +--------------+           | E -> . id    |  * [t13]  ################
*        +--------------+                |                     +--------------+                |        
*           |                            |                        ^            \               | + [t12]
*           | id [t2]                    | + [t4]                 |             \              |        
*           |                            |                        | * [t11]      \             V 
*           V       3                    V        5               |          7    \         +-----+
*        #===========#  id [t7]     +--------------+  E [t6]   ################    \        |  5  |
*        | E -> id . | <----------- | E -> E + . E | --------> # E -> E + E . #     \       +-----+
*        #===========#              | E -> . E + E |           # E -> E . + E #     |
*                ^                  | E -> . E * E | <-------- # E -> E . * E #     |
*                |                  | E -> . id    |  + [t10]  ################     | id [t9]
*                |                  +--------------+                                |
*                |                                                                  |
*                +------------------------------------------------------------------+
*    
*    
* - The symbom code of the non-terminal E  is 1
* - The symbom code of the     terminal +  is 2
* - The symbom code of the     terminal *  is 3
* - The symbom code of the     terminal id is 4
* - The symbom code of the     terminal $  is 5
*
* In this scenario:
*
* - dfa_stateCode2transitionSymbol (1, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionSymbol (1, 1) returns 1 because the first item in state 1 is E'->.E$ and the symbol code for E is 1
* - dfa_stateCode2transitionSymbol (1, 2) returns 4 because the next item with the dot before a different symbol is E->.id and the symbol code for id is 4
* - dfa_stateCode2transitionSymbol (1, 3) causes an error, as there are only 2 transitions from state 1
*
* - dfa_stateCode2transitionSymbol (2, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionSymbol (2, 1) returns 5 because the first item in state 2 is E'->E.$ and the symbol code for $ is 5
* - dfa_stateCode2transitionSymbol (2, 2) returns 2 because the next item with the dot before a different symbol is E->E.+E and the symbol code for + is 2
* - dfa_stateCode2transitionSymbol (2, 3) returns 3 because the next item with the dot before a different symbol is E->E.*E and the symbol code for * is 3
* - dfa_stateCode2transitionSymbol (2, 4) causes an error, as there are only 3 transitions from state 1
*
* - dfa_stateCode2transitionSymbol (3, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionSymbol (3, 1) causes an error, as state 3 is a reduction state and there are no transitions from reduction states
*
* - dfa_stateCode2transitionSymbol (4, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionSymbol (4, 1) causes an error, as state 4 is a reduction state and there are no transitions from reduction states
*
* - dfa_stateCode2transitionSymbol (5, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionSymbol (5, 1) returns 1 because the first item in state 5 is E->E+.E and the symbol code for E is 1
* - dfa_stateCode2transitionSymbol (5, 2) returns 4 because the next item with the dot before a different symbol is E->.id and the symbol code for id is 4
* - dfa_stateCode2transitionSymbol (5, 3) causes an error, as there are only 2 transitions from state 1
*
*   etc.
*-------------------------------------------------------------------------------------------------------------
*/

t_symbolCode dfa_stateCode2transitionSymbol (t_stateCode stateCode, unsigned int whichSymbol)
{
 int
   iItem,
   iSymbol,
   totTransSymbolsExpected,
   totTransSymbolsFound;
 bool
   b_symbolOnList;
 t_itemCode
   itemCode;
 t_symbolCode
   thisTransSymbol;
 t_DFAstate
   *p_DFAstate;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);
 
 /* Reduction states have no state transitions */

 if (dfa_isReductionState (stateCode)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "There are no transitions from DFA reduction state %d\n", stateCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Now we know this is not a reduction state, so there must be at least one state transition */
 /* on a terminal or non-terminal                                                             */

 if (whichSymbol < 1) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid symbol ordinal %u in DFA state %d; must be > 0\n", whichSymbol, stateCode);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* Now we know this is not a reduction state, so there must be at least one state transition */
 
 p_DFAstate = &DFAstates[stateCode - DFA_STATE_START_CODE];
 totTransSymbolsExpected = p_DFAstate->totTransitionSymbols;
 if (whichSymbol > totTransSymbolsExpected) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid symbol ordinal %u; DFA state %d has only %d transitions\n", whichSymbol, stateCode, totTransSymbolsExpected);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 
 /* Build a list of transition symbols in the order the corresponding items occur in the state */
 totTransSymbolsFound = 0;
 int
   transitionSymbols[totTransSymbolsExpected];
 for (iItem = 1; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
   itemCode = dfa_stateCode2itemCode (stateCode, iItem);
   if (isReductionItem (itemCode))
     continue;
   thisTransSymbol = itemCode2transitionSymbol (itemCode);
   /* Only add this transition symbol to the list if it's not there already */
   for (iSymbol = 0, b_symbolOnList = false; iSymbol < totTransSymbolsFound; iSymbol++) {
     if (transitionSymbols[iSymbol] == thisTransSymbol) {
       b_symbolOnList = true;
       break;
     }
   }
   if (! b_symbolOnList)
     transitionSymbols[totTransSymbolsFound++] = itemCode2transitionSymbol (itemCode);
 }
 if (totTransSymbolsFound != totTransSymbolsExpected) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Expected %d transition symbols in state %d, found %d", totTransSymbolsExpected, stateCode, totTransSymbolsExpected);
   ERROR_fatal_error (errno, __FILE__, __func__, ERROR_auxErrorMsg);
 } 
 return (transitionSymbols[whichSymbol-1]);
}

/*
*-------------------------------------------------------------------------------------------------------------
* Take a DFA state code and an ordinal (1 onwards),  
* and return the corresponding transition number.
*
* NOTE:
* Any DFA state contains one or more LR(0) item.
* - If the state contains only one item and if the dot is at the end of the item, we have a reduction state.
*   There are no transitions from a reduction state.
* - Otherwise, there is at least one transition symbol in the state, possibly more.
* - For each transition symbol, there is exactly one transition.
*
* Example:
* 
*  AUGMENTED GRAMMAR:
*                                               4
*    1) E'-> E $                    #============#
*    2) E -> E + E                  | E'-> E $ . |
*    3) E -> E * E                  #============#
*    4) E -> id                         ^   
*                                       | $ [t3]
*                      1                |         2                          6                      8
*        +--------------+           +--------------+           +--------------+  E [t8]   ################
*        | E'-> . E $   |  E [t1]   | E'-> E . $   |  * [t5]   | E -> E * . E | --------> # E -> E * E . #
*        | E -> . E + E | --------> | E -> E . + E | --------> | E -> . E + E |           # E -> E . + E #
*        | E -> . E * E |           | E -> E . * E |           | E -> . E * E | <-------- # E -> E . * E #
*        | E -> . id    |           +--------------+           | E -> . id    |  * [t13]  ################
*        +--------------+                |                     +--------------+                |        
*           |                            |                        ^            \               | + [t12]
*           | id [t2]                    | + [t4]                 |             \              |        
*           |                            |                        | * [t11]      \             V 
*           V       3                    V        5               |          7    \         +-----+
*        #===========#  id [t7]     +--------------+  E [t6]   ################    \        |  5  |
*        | E -> id . | <----------- | E -> E + . E | --------> # E -> E + E . #     \       +-----+
*        #===========#              | E -> . E + E |           # E -> E . + E #     |
*                ^                  | E -> . E * E | <-------- # E -> E . * E #     |
*                |                  | E -> . id    |  + [t10]  ################     | id [t9]
*                |                  +--------------+                                |
*                |                                                                  |
*                +------------------------------------------------------------------+
*    
*    
* - The symbom code of the non-terminal E  is 1
* - The symbom code of the     terminal +  is 2
* - The symbom code of the     terminal *  is 3
* - The symbom code of the     terminal id is 4
* - The symbom code of the     terminal $  is 5
*
* In this scenario:
*
* - dfa_stateCode2transitionNumber (1, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionNumber (1, 1) returns 1 (transition t1) because the first item in state 1 is E'->.E$ and the transition with E is t1
* - dfa_stateCode2transitionNumber (1, 2) returns 2 (transition t2) because the next item with the dot before a different symbol is E->.id and the transition with id is t2
* - dfa_stateCode2transitionNumber (1, 3) causes an error, as there are only 2 transitions from state 1
*
* - dfa_stateCode2transitionNumber (2, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionNumber (2, 1) returns 3 (transition t3) because the first item in state 2 is E'->E.$ and the transition with $ is t3
* - dfa_stateCode2transitionNumber (2, 2) returns 4 (transition t4) because the next item with the dot before a different symbol is E->E.+E and the transition with + is t4
* - dfa_stateCode2transitionNumber (2, 3) returns 5 (transition t5) because the next item with the dot before a different symbol is E->E.*E and the transition with * is t5
* - dfa_stateCode2transitionNumber (2, 4) causes an error, as there are only 3 transitions from state 1
*
* - dfa_stateCode2transitionNumber (3, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionNumber (3, 1) causes an error, as state 3 is a reduction state and there are no transitions from reduction states
*
* - dfa_stateCode2transitionNumber (4, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionNumber (4, 1) causes an error, as state 4 is a reduction state and there are no transitions from reduction states
*
* - dfa_stateCode2transitionNumber (5, 0) causes an error, as whichTransition must be > 0
* - dfa_stateCode2transitionNumber (5, 1) returns 6 (transition t6) because the first item in state 5 is E->E+.E and the transition with E is t6
* - dfa_stateCode2transitionNumber (5, 2) returns 7 (transition t7) because the next item with the dot before a different symbol is E->.id and the transition with id is t7
* - dfa_stateCode2transitionNumber (5, 3) causes an error, as there are only 2 transitions from state 1
*
*   etc.
*-------------------------------------------------------------------------------------------------------------
*/

unsigned int dfa_stateCode2transitionNumber (t_stateCode stateCode, unsigned int whichTransition)
{
 int
   iTransition;
 t_stateTransition
   *p_DFAtransition;
 t_symbolCode
   transitionSymbol = dfa_stateCode2transitionSymbol (stateCode, whichTransition);
 for (iTransition = 0; iTransition < totDFAtransitions; iTransition++) {
   p_DFAtransition = &DFAtransitions[iTransition];
   if ((p_DFAtransition->fromState == stateCode ) && (p_DFAtransition->withSymbol == transitionSymbol))
     return (iTransition + 1);
 }
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unable to find transition ordinal %u in DFA state %d\n", whichTransition, stateCode);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}
  
/*
*----------------------------------------------------------------------------
* Take the code of a DFA state and return the number of reduction items in it
*----------------------------------------------------------------------------
*/

unsigned int dfa_stateCode2totReductions (t_stateCode stateCode)
{
 unsigned int
   iItem,
   totReductions;
 t_DFAstate
   *p_DFAstate;
 t_LR0item
   *p_LR0item;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 p_DFAstate = &DFAstates[stateCode - DFA_STATE_START_CODE];

 for (totReductions = iItem = 0; iItem < p_DFAstate->itemsInState; iItem++) {

   /* In a reduction state the dot is at the end of the rule */

   p_LR0item = &LR0items[p_DFAstate->itemIndex[iItem]];
   if (p_LR0item->dotPosition == grammarRules[p_LR0item->ruleNumber-1].righthandSize)
     ++totReductions;
 }
 return (totReductions);
}

/*
*----------------------------------------------------------------------------
* Take the code of a NFA reduction state (i.e. containing a reduction item)
* and return the number of the corresponding rule
*----------------------------------------------------------------------------
*/

t_ruleNumber nfa_stateCode2reductionRule (t_stateCode stateCode)
{
 /* Ensure the state code provided really is a NFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d passed to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 /* Ensure the NFA state code provided really is a reduction state */

 if (! nfa_isReductionState (stateCode)) {
   snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s() does not contain a reduction item\n", stateCode, __func__);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }

 /* The NFA state code provided really is a reduction state, so return the rule number */

 return (itemCode2ruleNumber (nfa_stateCode2itemCode (stateCode)));
}

/*
*----------------------------------------------------------------------------
* Take the code of a DFA state (with possibly multiple reductions) and
* an ordinal number, and return the number of the corresponding rule
*----------------------------------------------------------------------------
*/

t_ruleNumber dfa_stateCode2reductionRule (t_stateCode stateCode, unsigned int reductionNumber)
{
 unsigned int
   iItem,
   totReductions;
 t_DFAstate
   *p_DFAstate;
 t_LR0item
   *p_LR0item;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 p_DFAstate = &DFAstates[stateCode - DFA_STATE_START_CODE];

 for (totReductions = iItem = 0; iItem < p_DFAstate->itemsInState; iItem++) {

   /* In a reduction state the dot is at the end of the rule */

   p_LR0item = &LR0items[p_DFAstate->itemIndex[iItem]];
   if (p_LR0item->dotPosition == grammarRules[p_LR0item->ruleNumber-1].righthandSize) {
     if (++totReductions == reductionNumber)
       return (p_LR0item->ruleNumber);
   }
 }

 /* No such reduction number in state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Requested reduction rule %u but DFA state %d has only %u reduction item(s)\n", reductionNumber, stateCode, totReductions);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

/*
*----------------------------------------------------------------------------
* Take a DFA state code and return a Boolean indicating whether it is a
* reduction state (i.e. whether it only contains reduction items)
*----------------------------------------------------------------------------
*/

bool dfa_isReductionState (t_stateCode stateCode)
{
  return ((bool) (dfa_stateCode2totItems (stateCode) == dfa_stateCode2totReductions (stateCode)));
}

/*
*----------------------------------------------------------------------------
* Take a NFA state code and return a Boolean indicating whether it is a
* reduction state (i.e. whether it contains a reduction item)
*----------------------------------------------------------------------------
*/

bool nfa_isReductionState (t_stateCode stateCode)
{
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d passed to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);
 return ((bool) (stateCode2stateType (stateCode) == t_NFA_reduce_state));
}

/*
*----------------------------------------------------------------------------
* Take a DFA state code and delete it
*----------------------------------------------------------------------------
*/

static void dfa_removeState (t_stateCode stateCode)
{
 int
   stateIndex;
 int
   iItem,
   iSymbol;
 t_DFAstate
   *p_DFAstate;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_DFA, ERROR_auxErrorMsg);

 stateIndex = stateCode - DFA_STATE_START_CODE;

 /* If this state is not at the very end of the    */
 /* DFAstates array then shift down array contents */

 if (stateIndex < (int) dfa_nextState - 1)
   memmove (
     &DFAstates[stateIndex],
     &DFAstates[stateIndex + 1],
     (dfa_nextState - stateIndex - 1) * (sizeof (t_DFAstate)) );

 /* Now clear the element at the end of the array */

 p_DFAstate = &DFAstates[--dfa_nextState];
 p_DFAstate->itemsInState = 0;
 for (iItem = 0; iItem < MAX_ITEMS_PER_DFA_STATE; iItem++)
   p_DFAstate->itemIndex[iItem] = UNKNOWN_ITEM_CODE;
 p_DFAstate->totInwardTransitions = p_DFAstate->totTransitionSymbols = 0;
 for (iSymbol = 0; iSymbol < MAX_TRANSITIONS_PER_DFA_STATE; iSymbol++)
   p_DFAstate->transitionSymbols[iSymbol] = UNKNOWN_SYMBOL_CODE;
 totDFAstates--;
}

/*
*---------------------------------------------------------------------
* Compute the nextState(S,X) operation where
*   S is a DFA state containing a set of items and
*   X is a grammar symbol
* This method only performs a simple table look up operation.
* It does not create items or states, like the dfa_gotoState() method.
*---------------------------------------------------------------------
*/

t_stateCode dfa_lookupNextState (t_stateCode fromState, t_symbolCode withSymbol)
{
 unsigned int
   iTransition;
 t_stateTransition
   *p_DFAtransition;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", fromState, __func__);
 validate_state_FSA_type (fromState, t_DFA, ERROR_auxErrorMsg);

 for (iTransition = 0; iTransition < totDFAtransitions; iTransition++) {
   p_DFAtransition = &DFAtransitions[iTransition];
   if ((p_DFAtransition->fromState  == fromState ) &&
       (p_DFAtransition->withSymbol == withSymbol) )
     return (p_DFAtransition->toState);
 }

 /* There is no transition from currStateCode with */
 /* transitionSymbol, so return UNKNOWN_STATE_CODE */

 return (UNKNOWN_STATE_CODE);
}

/*
*---------------------------------------------------------------------
* Compute the goto(S,X) operation where
*   S is a DFA state containing a set of LR(0) items and
*   X is a grammar symbol
* This method creates new items and states as it goes along,
* as opposed to the dfa_lookupNextState() method.
*---------------------------------------------------------------------
*
* GENERAL PROCEDURE:
* Find all items in S with the dot just before X, move the dot one
* position and add these new items to a new state.
* Return the numeric code of the goto() state.
*
* ALGORITHM:
*   Create a new empty state
*   For each non-reduction item in state S
*     If its transition symbol matches X then
*       Move the dot one position
*       Add this new item to the new state
*   Perform the closure of the new state
*   If the new state is identical to an existing state then
*     Delete the new state and use the existing state instead
*   Update data structures to reflect transition to the new state
*   Return the numeric code of the new state
*
*   PS- Don't bother looking for the transition symbol in reduction
*       items as the dot is already at the end position in those.
*---------------------------------------------------------------------
*/

static t_stateCode dfa_gotoState (t_stateCode currStateCode, t_symbolCode transitionSymbol)
{
 unsigned int
   iItem;
 t_symbolCode
   currTransitionSymbol;
 t_itemCode
   currItemCode,
   newItemCode;
 t_stateCode
   newStateCode,
   duplicateStateCode;

 /* Ensure the state code provided really is a DFA state */

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "NFA state %d passed to %s(); expected DFA state\n", currStateCode, __func__);
 validate_state_FSA_type (currStateCode, t_DFA, ERROR_auxErrorMsg);

 /* Create a new empty state */

 newStateCode = dfa_newEmptyState();

 /* For each non-reduction item in the state */

 for (iItem = 1; iItem <= dfa_stateCode2totItems (currStateCode); iItem++) {
   currItemCode = dfa_stateCode2itemCode (currStateCode, iItem);
   if (! isReductionItem (currItemCode)) {

     /* If its transition symbol matches the specified one then... */

     currTransitionSymbol = itemCode2transitionSymbol (currItemCode);
     if (currTransitionSymbol == transitionSymbol) {

       /* Move the dot one position */

       newItemCode = newItem (itemCode2ruleNumber (currItemCode), itemCode2dotPosition (currItemCode) + 1);

       /* Add this new item to the new state */

       (void) dfa_addItemToState (newItemCode, newStateCode);
     }
   }
 }

 /* If there is no transition from currStateCode with transitionSymbol */
 /* then remove the newly created state and return UNKNOWN_STATE_CODE  */

 if (dfa_isEmptyState (newStateCode)) {
   dfa_removeState (newStateCode);
   return (UNKNOWN_STATE_CODE);
 }

 /* Perform the dfa_closure of the new state */

 dfa_closure (newStateCode);

 /* if the complete state already exists, then remove this */
 /* latest duplicate and return the code of existing state */

 if (dfa_isDuplicateState (newStateCode)) {
   duplicateStateCode = dfa_stateCode2duplicateStateCode (newStateCode);
   dfa_removeState (newStateCode);
   newStateCode = duplicateStateCode;
 }

 /* Return the numeric code of the goto() state */

 return (newStateCode);
}

/*
*---------------------------------------------------------------------
* Perform closure of a DFA state
*---------------------------------------------------------------------
*/

static void dfa_closure (t_stateCode stateCode)
{
 unsigned int
   iUse,
   iItem;
 int
   prevTotItems,
   currTotItems;
 t_itemCode
   currItemCode,
   newItemCode;
 t_symbolCode
   symbolCode;
 t_symbolType
   symbolType;

 prevTotItems = -1;
 currTotItems = (int) dfa_stateCode2totItems (stateCode);
 while (prevTotItems != currTotItems) {
   for (iItem = 1; (int) iItem <= currTotItems; iItem++) {

     /* Ignore reduction items */

     currItemCode = dfa_stateCode2itemCode (stateCode, iItem);
     if (isReductionItem (currItemCode))
       continue;

     /* Ignore items with the dot before a terminal symbol */

     symbolCode = itemCode2transitionSymbol (currItemCode);
     symbolType = symbolCode2symbolType (symbolCode);
     if ((symbolType == t_terminal) || (symbolType == t_endOfInput))
       continue;

     /* Add all rules with this non-terminal on the lefthand side */

     for (iUse = 1; iUse <= symbolCode2totUses (symbolCode, t_lefthand); iUse++) {
       newItemCode = newItem (symbolCode2use (symbolCode, t_lefthand, iUse), 0);
       (void) dfa_addItemToState (newItemCode, stateCode);
     }
   }
   prevTotItems = currTotItems;
   currTotItems = (int) dfa_stateCode2totItems (stateCode);
 }
}

/*
*---------------------------------------------------------------------
* Build LR(0) items, NFA and DFA states and state transitions
*---------------------------------------------------------------------
*/

void build_LR0_items_NFA_and_DFA (void)
{
 unsigned int
   iItem,
   iState,
   jState,
   iSymbol,
   iUse,
   totStateItems,
   totStateShifts,
   totStateReductions;
 int
   prevTotStates;
 t_stateType
   stateType;
 t_symbolCode
   startSymbol,
   transitionSymbol;
 t_ruleNumber
   ruleNumber;
 t_stateCode
   stateCode,
   fromState,
   toState,
   startState,
   currState,
   newState;
 t_itemCode
   fromItem,
   toItem,
   itemCode;
 t_dotPosition
   dotPosition;

 /* Some initializations */

 totDFAstates = totDFAtransitions = 0;

 /* First create an empty start state */

 startState = dfa_newEmptyState();

 /* Add the start symbol items to the initial state. There are two approaches to this: */
 /*                                                                                    */
 /* 1- Augmenting the grammar:                                                         */
 /*                                                                                    */
 /*      Create a new non-terminal, say E'                                             */
 /*      Add to the grammar a new rule like E'-> E $                                   */
 /*      Create an LR(0) item from this rule, like E'-> . E $                          */
 /*      Add this item to the empty start state                                        */
 /*      Carry out the state closure                                                   */
 /*                                                                                    */
 /* 2- Adding the appropriate LR(0) items by hand:                                     */
 /*                                                                                    */
 /*      Make the start symbol the non-terminal on the lefthand side of the first rule */
 /*      For each rule with the start symbol on its lefthand side:                     */
 /*        Create an LR(0) item from this rule                                         */
 /*        Add this item to the start state                                            */
 /*      Carry out the state closure                                                   */
 /*                                                                                    */
 /*     However, this will only work properly under very specific circunstances.       */
 /*     The first approach described above is more general and correct, therefore      */
 /*     it is the approach used. See read_grammar() for implementation details.        */

 startSymbol = rulePos2symbolCode (1,0);

 /* Create an item from each one of start symbol's rules */

 for (iUse = 1; iUse <= symbolCode2totUses (startSymbol, t_lefthand); iUse++) {
   ruleNumber = symbolCode2use (startSymbol, t_lefthand, iUse);
   (void) dfa_addItemToState (newItem (ruleNumber,0), startState);
 }
 dfa_closure (startState);

 /* Keep creating DFA goto() states and        */
 /* transitions until no more states are added */

 prevTotStates = -1;
 while (prevTotStates != (int) totDFAstates) {
   prevTotStates = (int) totDFAstates;
   for (iState = 1; iState <= totDFAstates; iState++) {
     currState = dfa_stateNumber2stateCode (iState);
     for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
       transitionSymbol = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
       newState = dfa_gotoState (currState, transitionSymbol);
       if (newState != UNKNOWN_STATE_CODE)
         dfa_newTransition (currState, transitionSymbol, newState);
     }
     for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
       transitionSymbol = symbolNumber2symbolCode (iSymbol, t_terminal);
       newState = dfa_gotoState (currState, transitionSymbol);
       if (newState != UNKNOWN_STATE_CODE)
         dfa_newTransition (currState, transitionSymbol, newState);
     }
   }
 }

 /* At this point all LR(0) items have been built, so  */
 /* building the NFA can be done in two simple steps   */
 /* Setp 1: for each item, put it in a new empty state */

 for (iItem = 0; iItem < nextLR0item; iItem++) {
   itemCode = (t_itemCode) (iItem + ITEM_START_CODE);
   (void) nfa_addItemToState (itemCode, nfa_newEmptyState());
 }

 /* Setp 2: for each state, work out the corresponding transitions */

 for (iState = 1; iState <= totNFAstates; iState++) {
   fromState = nfa_stateNumber2stateCode (iState);
   fromItem  = nfa_stateCode2itemCode (fromState);

   /* Reduction states have no transitions */

   if (isReductionItem (fromItem))
     continue;

   /* Locate the state containing the current item */
   /* with the dot advanced by one position        */

   ruleNumber  = itemCode2ruleNumber (fromItem);
   dotPosition = itemCode2dotPosition (fromItem);
   transitionSymbol = itemCode2transitionSymbol (fromItem);
   for (jState = 1; jState <= totNFAstates; jState++) {
     toState = nfa_stateNumber2stateCode (jState);
     toItem = nfa_stateCode2itemCode (toState);
     if ( (itemCode2ruleNumber  (toItem) == ruleNumber) &&
          (itemCode2dotPosition (toItem) == dotPosition + 1) ) {

       /* Found the state we were looking for, so add */
       /* the transition and break out of the loop    */

       nfa_newTransition (fromState, transitionSymbol, toState);
       break;
     }
   }

   /* If the transition symbol is a non terminal, we must    */
   /* create an epsilon transition for each one of its rules */

   if (symbolCode2symbolType (transitionSymbol) != t_nonTerminal)
     continue;
   for (iUse = 1; iUse <= symbolCode2totUses (transitionSymbol, t_lefthand); iUse++) {
     ruleNumber = symbolCode2use (transitionSymbol, t_lefthand, iUse);
     for (jState = 1; jState <= totNFAstates; jState++) {
       toState = nfa_stateNumber2stateCode (jState);
       toItem  = nfa_stateCode2itemCode (toState);
       if ( (itemCode2ruleNumber (toItem) == ruleNumber) &&
            (itemCode2dotPosition (toItem) == 0) ) {

         /* Found the state we were looking for, so add the */
         /* epsilon transition and break out of the loop    */

         nfa_newTransition (fromState, EPSILON_CODE, toState);
         break;
       }
     }
   }
 }

 /* Work out the number of NFA and DFA items */

 totNFAitems = totLR0items;
 for (iState = 1, totDFAitems = 0; iState <= totDFAstates; iState++)
   totDFAitems += dfa_stateCode2totItems (dfa_stateNumber2stateCode (iState));

 /* Set all NFA state types and total counts */

 for (iState = 1; iState <= totNFAstates; iState++) {
   stateCode = nfa_stateNumber2stateCode (iState);
   itemCode = nfa_stateCode2itemCode (stateCode);
   if (isReductionItem (itemCode)) {
     stateType = t_NFA_reduce_state;
     tot_NFA_reduce_states++;
   }
   else {
     transitionSymbol = itemCode2transitionSymbol (itemCode);
     if (symbolCode2symbolType (transitionSymbol) == t_nonTerminal) {
       stateType = t_NFA_non_deterministic_state;
       tot_NFA_non_deterministic_states++;
     }
     else {
       stateType = t_NFA_shift_state;
       tot_NFA_shift_states++;
     }
   }
   NFAstates[stateCode - NFA_STATE_START_CODE].stateType = stateType;
 }

 /* Set all DFA state types and total counts */

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   totStateItems = dfa_stateCode2totItems (stateCode);
   totStateShifts = totStateReductions = 0;
   for (iItem = 1; iItem <= totStateItems; iItem++) {
     itemCode = dfa_stateCode2itemCode (stateCode, iItem);
     if (isReductionItem (itemCode))
       totStateReductions++;
     else
       totStateShifts++;
   }
   if (totStateReductions == 0) {
     stateType = t_DFA_shift_state;
     tot_DFA_shift_states++;
   }
   else if (totStateReductions == 1) {
     if (totStateItems == 1) {
       stateType = t_DFA_1_reduce_state;
       tot_DFA_1_reduce_states++;
     }
     else {
       stateType = t_DFA_shift_1_reduce_state;
       tot_DFA_shift_1_reduce_states++;
     }
   }
   else if (totStateShifts == 0) {
     stateType = t_DFA_N_reduce_state;
     tot_DFA_N_reduce_states++;
   }
   else {
     stateType = t_DFA_shift_N_reduce_state;
     tot_DFA_shift_N_reduce_states++;
   }
   DFAstates[stateCode - DFA_STATE_START_CODE].stateType = stateType;
 }
}

/*
*--------------------------------------------------------------------------------
* Return the number of symbols in a FIRST or FOLLOW set
*--------------------------------------------------------------------------------
*/

unsigned int setSize (t_setType setType, t_symbolCode symbolCode)
{
 return(totSymbolsInSet ((t_extSetType) setType, symbolCode));
}

/*
*--------------------------------------------------------------------------------
* Return a specific symbol in a FIRST or FOLLOW set
*--------------------------------------------------------------------------------
*/

t_symbolCode getSymbolInSet (t_setType setType, t_symbolCode symbolCode, unsigned int posInSet)
{
 return (symbolInSet ((t_extSetType) setType, symbolCode, posInSet));
}

/*
*--------------------------------------------------------------------------------
* Return a boolean saying whether a symbol has a FIRST or FOLLOW set
*--------------------------------------------------------------------------------
*/

static bool has_SET (t_symbolCode symbolCode, t_extSetType setType)
{
 t_symbolType
   symbolType = UNKNOWN_SYMBOL_CODE;

 if (symbolCode != ALPHA_CODE)
   symbolType = symbolCode2symbolType (symbolCode);

 switch (setType) {
   case (t_first_alpha_set):
     if (symbolCode == ALPHA_CODE)
       return (true);
     else {
       snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid use of symbol code %d with FIRST(alpha) set\n", symbolCode);
       ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
   case (t_first_set):
     switch (symbolType) {
       case (t_terminal):
       case (t_nonTerminal):
       case (t_endOfInput):
         return (true);
/*
       case (t_endOfInput):
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "End-of-input does not have a FIRST set\n");
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
*/
       case (t_epsilon):
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Epsilon does not have a FIRST set\n");
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       default:      /* This should never happen! */
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Symbol %d of Unknown type %d - does not have a FIRST set\n", symbolCode, symbolType);
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
   case (t_follow_set):
     switch (symbolType) {
       case (t_nonTerminal):
         return (true);
       case (t_terminal):
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Symbol %d [%s] is a terminal. Only non-terminals have a FOLLOW set\n", symbolCode, symbolCode2symbolString (symbolCode));
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       case (t_endOfInput):
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "End-of-input does not have a FOLLOW set\n");
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       case (t_epsilon):
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Epsilon does not have a FOLLOW set\n");
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       default:      /* This should never happen! */
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Symbol %d of Unknown type %d - does not have a FOLLOW set\n", symbolCode, symbolType);
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
   default: {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown set type %d\n", setType);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }
}

/*
*--------------------------------------------------------------------------------
* Return a boolean saying whether a symbol can be in a FIRST or FOLLOW set
*--------------------------------------------------------------------------------
*/

static bool can_be_in_SET (t_symbolCode symbolCode, t_extSetType setType)
{
 t_symbolType
   symbolType;

 symbolType = symbolCode2symbolType (symbolCode);
 switch (setType) {
   case (t_first_alpha_set):
   case (t_first_set):
     switch (symbolType) {
       case (t_terminal):
       case (t_epsilon):
       case (t_endOfInput):
         return (true);
/*       case (t_endOfInput):
           snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "End-of-input not allowed in FIRST set, only terminals or epsilon\n");
           ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
*/
       case (t_nonTerminal):
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Non-terminal %d not allowed in FIRST set, only terminals or epsilon\n", symbolCode);
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       default:      /* This should never happen! */
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Symbol %d of Unknown type (%d). Not allowed in FIRST set\n", symbolCode, symbolType);
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
   case (t_follow_set):
     switch (symbolType) {
       case (t_terminal):
       case (t_endOfInput):
         return (true);
       case (t_epsilon):
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Epsilon not allowed in FOLLOW set, only terminals or end_of_input\n");
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       case (t_nonTerminal):
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Non-terminal %d not allowed in FOLLOW set, only terminals or end_of_input\n", symbolCode);
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       default:      /* This should never happen! */
         snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Symbol %d of Unknown type (%d). Not allowed in FOLLOW set\n", symbolCode, symbolType);
         ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
     }
   default: {
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown set type %d\n", setType);
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   }
 }
}

/*
*---------------------------------------------------------------------------------
* Take as input a symbol code and a set type
* Return the corresponding set code
*---------------------------------------------------------------------------------
*/

static DATA_t_set_code get_set_code (t_extSetType setType, t_symbolCode symbolCode)
{
 t_symbolType
   symbolType;
 t_symbolData
   *p_symbolData;

 if (setType == t_first_alpha_set)
   p_symbolData = &alphaUsage;
 else {
   symbolType = symbolCode2symbolType (symbolCode);
   if ((symbolType == t_terminal) || (symbolType == t_endOfInput))
     p_symbolData = &terminals[symbolCode - TERMINAL_START_CODE];
   else
     p_symbolData = &nonTerminals[symbolCode - NON_TERMINAL_START_CODE];
 }

 /* The call to is_in_SET() has already checked for unknown extSetType */

 if ((setType == t_first_alpha_set) || (setType == t_first_set))
   return (p_symbolData->firstSet);
 else
   return (p_symbolData->followSet);
}

/*
*--------------------------------------------------------------------------------
* Clear the FIRST or FOLLOW set of a grammar symbol or alpha
*--------------------------------------------------------------------------------
*/

void clear_SET (t_extSetType setType, t_symbolCode symbolCode)
{
 (void) has_SET (symbolCode, setType);
 DATA_clear_set (get_set_code (setType, symbolCode));
}

/*
*--------------------------------------------------------------------------------
* Return the number of symbols in a FIRST or FOLLOW set
*--------------------------------------------------------------------------------
*/

static unsigned int totSymbolsInSet (t_extSetType setType, t_symbolCode symbolCode)
{
 (void) has_SET (symbolCode, setType);
 return (DATA_get_set_size (get_set_code (setType, symbolCode)));
}

/*
*--------------------------------------------------------------------------------
* Return a specific symbol in a FIRST or FOLLOW set
*--------------------------------------------------------------------------------
*/

static t_symbolCode symbolInSet (t_extSetType setType, t_symbolCode symbolCode, unsigned int posInSet)
{
 unsigned int
   symbolsInSet;
 DATA_t_set_code
   setCode;

 (void) has_SET (symbolCode, setType);
 setCode = get_set_code (setType, symbolCode);
 symbolsInSet = DATA_get_set_size (setCode);
 if (posInSet > symbolsInSet) {
   if ((setType == t_first_set) || (setType == t_first_alpha_set))
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Only %u symbols in FIRST(%s) - position %u invalid\n", symbolsInSet, symbolCode2symbolString (symbolCode), posInSet);
   else
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Only %u symbols in FOLLOW(%s) - position %u invalid\n", symbolsInSet, symbolCode2symbolString (symbolCode), posInSet);
   ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
 DATA_read_from_set (setCode, posInSet, &symbolCode);
 return (symbolCode);
}

/*
*------------------------------------------------------------------------------------------
* Return a boolean saying whether a symbol is in the FIRST or FOLLOW set of another symbol
*------------------------------------------------------------------------------------------
*/

static bool is_in_SET (t_symbolCode symbolToCheck, t_extSetType setType, t_symbolCode setOfSymbol)
{
 unsigned int
   iSymbol;

 (void) can_be_in_SET (symbolToCheck, setType);
 (void) has_SET (setOfSymbol, setType);

 for (iSymbol = 1; iSymbol <= totSymbolsInSet (setType, setOfSymbol); iSymbol++)
   if (symbolInSet (setType, setOfSymbol, iSymbol) == symbolToCheck)
     return (true);
 return (false);
}

/*
*-----------------------------------------------------------------------------
* Add a terminal symbol to the FIRST or FOLLOW set of another grammar symbol
* Return a boolean indicating whether the terminal was actually added.
* In other words, return
*   true  if the terminal was not yet in the set
*   false if the terminal already was in the set
*-----------------------------------------------------------------------------
*/

static bool add_to_SET (t_symbolCode symbolToAdd, t_extSetType setType, t_symbolCode setOfSymbol)
{
 t_symbolCode
   symbol_to_add = symbolToAdd;
 DATA_t_set_code
   setCode;

 /* The symbol may already be in the set */

 if (is_in_SET (symbolToAdd, setType, setOfSymbol))
   return (false);

 /* The symbol is not in the set yet, so add it                        */
 /* The call to is_in_SET() has already checked for unknown extSetType */

 setCode = get_set_code (setType, setOfSymbol);
 DATA_add_to_set (setCode, (void *) &symbol_to_add, true);
 return (true);
}

/*
*---------------------------------------------------------------------------------
* Remove a terminal symbol from the FIRST or FOLLOW set of another grammar symbol
* Return a boolean indicating whether the terminal was actually removed.
* In other words, return
*   false if the terminal was not in the set
*   true  if the terminal was in the set and got removed
*---------------------------------------------------------------------------------
*/

static bool remove_from_SET (t_symbolCode symbolToRemove, t_extSetType setType, t_symbolCode setOfSymbol)
{
 /* The symbol may not be in the set */

 if (! is_in_SET (symbolToRemove, setType, setOfSymbol))
   return (false);

 /* The symbol is in the set, so remove it */
 /* The call to is_in_SET() has already checked for unknown extSetType */

 DATA_remove_from_set (get_set_code (setType, setOfSymbol), (void *) &symbolToRemove, 0);
 return (true);
}

/*
*---------------------------------------------------------------------
* Add all symbols in FIRST or FOLLOW sets of two grammar symbols
*---------------------------------------------------------------------
*/

static bool add_SETS (
  t_extSetType fromSetType,
  t_symbolCode fromSymbol,
  t_extSetType toSetType,
  t_symbolCode toSymbol )
{
 unsigned int
   iSymbol;
 t_symbolCode
   symbolToAdd;
 bool
   b_stillAdding = false;

 (void) has_SET (fromSymbol, fromSetType);
 (void) has_SET (toSymbol, toSetType);

 for (iSymbol = 1; iSymbol <= totSymbolsInSet (fromSetType, fromSymbol); iSymbol++) {
   symbolToAdd = symbolInSet (fromSetType, fromSymbol, iSymbol);
   b_stillAdding |= add_to_SET (symbolToAdd, toSetType, toSymbol);
 }
 return (b_stillAdding);
}

/*
*---------------------------------------------------------------------
* Build FIRST sets
*---------------------------------------------------------------------
*/

void build_first_sets (void)
{
 unsigned int
   iXsymbol,
   iYsymbol,
   iRule,
   ruleSize;
 bool
   b_epsilonInFirstSet,
   b_continue,
   b_stillAdding;
 t_symbolCode
   XsymbolCode,
   YsymbolCode;
 t_symbolData
   *p_symbolData;

 if (! DATA_is_module_initialized())
   DATA_initialize_module (true);

 /* Set up FIRST sets for all terminals and non-terminals */

 alphaUsage.firstSet = DATA_new_set (setElementSize, 0, 0);
 for (iXsymbol = 1; iXsymbol <= totTerminals; iXsymbol++) {
   XsymbolCode = symbolNumber2symbolCode (iXsymbol, t_terminal);
   p_symbolData = &terminals[XsymbolCode - TERMINAL_START_CODE];
   p_symbolData->firstSet = DATA_new_set (setElementSize, 0, 0);
 }
 for (iXsymbol = 1; iXsymbol <= totNonTerminals; iXsymbol++) {
   XsymbolCode = symbolNumber2symbolCode (iXsymbol, t_nonTerminal);
   p_symbolData = &nonTerminals[XsymbolCode - NON_TERMINAL_START_CODE];
   p_symbolData->firstSet = DATA_new_set (setElementSize, 0, 0);
 }

 /* If X is a terminal or epsilon, then FIRST(X) is {X} */

 for (iXsymbol = 1; iXsymbol <= totTerminals; iXsymbol++) {
   XsymbolCode = symbolNumber2symbolCode (iXsymbol, t_terminal);
   (void) add_to_SET (XsymbolCode, t_first_set, XsymbolCode);
 }

 /* Keep going until nothing more gets added to any FIRST set */

 b_stillAdding = true;
 while (b_stillAdding) {
   b_stillAdding = false;

   /* For each production choice X -> Y1 Y2 ... Yn do */

   for (iRule = 1; iRule <= totRules; iRule++) {
     ruleSize = ruleNumber2ruleSize ((t_ruleNumber) iRule);
     XsymbolCode = rulePos2symbolCode (iRule, 0);
     iYsymbol = 1;
     b_continue = true;
     while (b_continue && (iYsymbol <= ruleSize)) {

       /* Add FIRST(Yi)-{epsilon} to FIRST(X)                          */
       /*                                                              */
       /* NOTE:                                                        */
       /* We would need a temp set for working out FIRST(Yi)-{epsilon} */
       /* before adding it to FIRST(X), but we haven't got one so we   */
       /* will break this step into three separate steps:              */
       /*                                                              */
       /* - Take epsilon out of FIRST(Yi)                              */
       /* - Add FIRST(Yi+1) to FIRST(X)                                */
       /* - If epsilon was in FIRST(Yi) then put it back in            */

       YsymbolCode = rulePos2symbolCode ((t_ruleNumber) iRule, iYsymbol);
       if (YsymbolCode == end_of_input_code)
         b_continue = false;
       b_epsilonInFirstSet = is_in_SET (EPSILON_CODE, t_first_set, YsymbolCode);
       (void) remove_from_SET (EPSILON_CODE, t_first_set, YsymbolCode);
       b_stillAdding |= add_SETS (t_first_set, YsymbolCode, t_first_set, XsymbolCode);
       if (b_epsilonInFirstSet)
         (void) add_to_SET (EPSILON_CODE, t_first_set, YsymbolCode);
       else
         b_continue = false;
       iYsymbol++;
     }
     if (b_continue)
       b_stillAdding |= add_to_SET (EPSILON_CODE, t_first_set, XsymbolCode);
   }
 }
}

/*
*---------------------------------------------------------------------
* Build follow sets
*---------------------------------------------------------------------
*/

void build_follow_sets (void)
{
 unsigned int
   iXsymbol,
   iYsymbol,
   iYYsymbol,
   iXleftUse,
   XleftUses,
   XruleSize;
 bool
   b_epsilonInFirstSet,
   b_stillAdding;
 t_symbolCode
   startSymbol  = UNKNOWN_SYMBOL_CODE,
   XsymbolCode  = UNKNOWN_SYMBOL_CODE,
   YsymbolCode  = UNKNOWN_SYMBOL_CODE,
   YYsymbolCode = UNKNOWN_SYMBOL_CODE;
 t_symbolData
   *p_symbolData;
 t_ruleNumber
   XruleNumber = 0;

 if (! DATA_is_module_initialized())
   DATA_initialize_module (true);

 /* Set up FOLLOW sets for all non-terminals */

 for (iXsymbol = 1; iXsymbol <= totNonTerminals; iXsymbol++) {
   XsymbolCode = symbolNumber2symbolCode (iXsymbol, t_nonTerminal);
   p_symbolData = &nonTerminals[XsymbolCode - NON_TERMINAL_START_CODE];
   p_symbolData->followSet = DATA_new_set (setElementSize, 0, 0);
 }

 /* We augmented the grammar, so the initial rule is of the form _E_ -> E $ */
 /* Therefore we need to make                                               */
 /*   FOLLOW (_E_) = {$}                                                    */
 /*   FOLLOW (E)   = {$}                                                    */

 startSymbol = rulePos2symbolCode (1,0);
 (void) add_to_SET (end_of_input_code, t_follow_set, startSymbol);
 startSymbol = rulePos2symbolCode (1,1);
 (void) add_to_SET (end_of_input_code, t_follow_set, startSymbol);

 /* Keep going until nothing more gets added to any FOLLOW set */

 b_stillAdding = true;
 while (b_stillAdding) {
   b_stillAdding = false;

   /* For each production X -> Y1 Y2 ... Yn do */

   for (iXsymbol = 1; iXsymbol <= totNonTerminals; iXsymbol++) {
     XsymbolCode = symbolNumber2symbolCode (iXsymbol, t_nonTerminal);
     XleftUses = symbolCode2totUses (XsymbolCode, t_lefthand);
     for (iXleftUse = 1; iXleftUse <= XleftUses; iXleftUse++) {
       XruleNumber = symbolCode2use (XsymbolCode, t_lefthand, iXleftUse);
       XruleSize = ruleNumber2ruleSize (XruleNumber);

       /* For each Yi that is a non-terminal do */

       for (iYsymbol = 1; iYsymbol <= XruleSize; iYsymbol++) {
         YsymbolCode = rulePos2symbolCode (XruleNumber, iYsymbol);
         if (symbolCode2symbolType (YsymbolCode) != t_nonTerminal)
           continue;

         /* Build FIRST(alpha) where alpha = Yi+1 Yi+2 ... Yn */

         clear_SET (t_first_alpha_set, ALPHA_CODE);

         /* Add FIRST(Yi+1)-{epsilon} to FIRST(alpha)                      */
         /*                                                                */
         /* NOTE:                                                          */
         /* We would need a temp set for working out FIRST(Yi+1)-{epsilon} */
         /* before adding it to FIRST(alpha), but we haven't got one so we */
         /* will break this step into three separate steps:                */
         /*                                                                */
         /* - Take epsilon out of FIRST(Yi+1)                              */
         /* - Add FIRST(Yi+1) to FIRST(alpha)                              */
         /* - If epsilon was in FIRST(Yi+1) then put it back in            */

         if (iYsymbol == XruleSize)
           continue;

         iYYsymbol = iYsymbol+1;
         YYsymbolCode = rulePos2symbolCode (XruleNumber, iYYsymbol);
         b_epsilonInFirstSet = is_in_SET (EPSILON_CODE, t_first_set, YYsymbolCode);
         (void) remove_from_SET (EPSILON_CODE, t_first_set, YYsymbolCode);
         (void) add_SETS (t_first_set, YYsymbolCode, t_first_alpha_set, ALPHA_CODE);
         if (b_epsilonInFirstSet)
           (void) add_to_SET (EPSILON_CODE, t_first_set, YYsymbolCode);

         /* For each j=2,...,n                                             */
         /*   if epsilon is in FIRST(Yk) for all k=1,...,j-1 then          */
         /*     add FIRST(Yj)-{epsilon} to FIRST(alpha)                    */
         /*                                                                */
         /* NOTE:                                                          */
         /* We would need a temp set for working out FIRST(Yj)-{epsilon}   */
         /* before adding it to FIRST(alpha), but we haven't got one so we */
         /* will break this step into three separate steps:                */
         /*                                                                */
         /* - Take epsilon out of FIRST(Yj)                                */
         /* - Add FIRST(Yj) to FIRST(alpha)                                */
         /* - If epsilon was in FIRST(Yj) then put it back in              */

         for (iYYsymbol = iYsymbol+2; iYYsymbol <= XruleSize; iYYsymbol++) {
           if (! b_epsilonInFirstSet)
             break;
           YYsymbolCode = rulePos2symbolCode (XruleNumber, iYYsymbol);
           b_epsilonInFirstSet = is_in_SET (EPSILON_CODE, t_first_set, YYsymbolCode);
           (void) remove_from_SET (EPSILON_CODE, t_first_set, YYsymbolCode);
           (void) add_SETS (t_first_set, YYsymbolCode, t_first_alpha_set, ALPHA_CODE);
           if (b_epsilonInFirstSet)
             (void) add_to_SET (EPSILON_CODE, t_first_set, YYsymbolCode);
         }

         /* Finally, if for all j=i+1,...,n FIRST(Yj) contains epsilon then */
         /*   add epsilon to FIRST(alpha)                                   */

         if (b_epsilonInFirstSet)
           (void) add_to_SET (EPSILON_CODE, t_first_alpha_set, ALPHA_CODE);

         /* Add FIRST(alpha)-{epsilon} to FOLLOW(Yi)                        */
         /*                                                                 */
         /* NOTE:                                                           */
         /* We would need a temp set for working out FIRST(alpha)-{epsilon} */
         /* before adding it to FOLLOW(Yi), but we haven't got one so we    */
         /* will break this step into three separate steps:                 */
         /*                                                                 */
         /* - Take epsilon out of FIRST(alpha)                              */
         /* - Add FIRST(alpha) to FOLLOW(Yi)                                */
         /* - If epsilon was in FIRST(alpha) then put it back in            */

         b_epsilonInFirstSet = is_in_SET (EPSILON_CODE, t_first_alpha_set, ALPHA_CODE);
         (void) remove_from_SET (EPSILON_CODE, t_first_alpha_set, ALPHA_CODE);
         b_stillAdding |= add_SETS (t_first_alpha_set, ALPHA_CODE, t_follow_set, YsymbolCode);
         if (b_epsilonInFirstSet)
           (void) add_to_SET (EPSILON_CODE, t_first_alpha_set, ALPHA_CODE);

         /* If epsilon is in FIRST(alpha) then add FOLLOW(X) to FOLLOW(Yi) */

         if (is_in_SET (EPSILON_CODE, t_first_alpha_set, ALPHA_CODE))
           b_stillAdding |= add_SETS (t_follow_set, XsymbolCode, t_follow_set, YsymbolCode);
       }

       /* NOTE:                                                */
       /* If i=n then alpha=epsilon and FIRST(alpha)={epsilon} */
       /* therefore add FOLLOW(X) to FOLLOW(Yn)                */

       if (symbolCode2symbolType (YsymbolCode) == t_nonTerminal)
         b_stillAdding |= add_SETS (t_follow_set, XsymbolCode, t_follow_set, YsymbolCode);
     }
   }
 }
}

/*
*---------------------------------------------------------------------
* Take a state code and a symbol code and return the number of parse
* actions in the corresponding parse table
*---------------------------------------------------------------------
*/

unsigned int parseTablePos2totParseActions (
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode )
{
  if (parse_table_type == t_LR0_parse_table)
    return (LR0parseTable [stateCode2parseTableRow(stateCode)][symbolCode2parseTableCol(symbolCode)].totParseActions);
  if (parse_table_type == t_sLR1_parse_table)
    return (sLR1parseTable[stateCode2parseTableRow(stateCode)][symbolCode2parseTableCol(symbolCode)].totParseActions);
  if (parse_table_type == t_diff_parse_table)
    return (diffParseTable[stateCode2parseTableRow(stateCode)][symbolCode2parseTableCol(symbolCode)].totParseActions);
  snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
           "Unknown parse table type %d (state=%d, symbol=%d)\n",
           parse_table_type, stateCode, symbolCode);
  ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

/*
*---------------------------------------------------------------------
* Take a state code, a symbol code and an action number and return
* the corresponding parse action in the corresponding parse table
*---------------------------------------------------------------------
*/

t_parseAction parseTablePos2parseAction (
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode,
  unsigned int       actionNumber )
{
  unsigned int
    totalActions;
  struct t_parseTableEntry
    parseTableEntry;
  struct t_parseActionList
    *p_parseAction;

  if ((parse_table_type != t_LR0_parse_table) && (parse_table_type != t_sLR1_parse_table) && (parse_table_type != t_diff_parse_table)) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
             "Unknown parse table type %d (state=%d, symbol=%d, actionNumber=%u)\n",
             parse_table_type, stateCode, symbolCode, actionNumber);
    ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  if (actionNumber < 1) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
             "Invalid parse action number %u, must be >= 1 (parse_table_type=%d, state=%d, symbol=%d)\n",
             actionNumber, parse_table_type, stateCode, symbolCode);
    ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  if (parse_table_type == t_LR0_parse_table)
    parseTableEntry = LR0parseTable [stateCode2parseTableRow(stateCode)][symbolCode2parseTableCol(symbolCode)];
  else if (parse_table_type == t_sLR1_parse_table)
    parseTableEntry = sLR1parseTable[stateCode2parseTableRow(stateCode)][symbolCode2parseTableCol(symbolCode)];
  else
    parseTableEntry = diffParseTable[stateCode2parseTableRow(stateCode)][symbolCode2parseTableCol(symbolCode)];

  totalActions = parseTableEntry.totParseActions;
  if (actionNumber > totalActions) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize,
             "Invalid parse action number %u, total actions = %u (parse_table_type=%d, state=%d, symbol=%d)\n",
             actionNumber, totalActions, parse_table_type, stateCode, symbolCode);
    ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
  }
  p_parseAction = parseTableEntry.p_parseActionList;
  while (--actionNumber > 0)
    p_parseAction = p_parseAction->p_nextParseAction;
  return (p_parseAction->parseAction);
}

/*
*---------------------------------------------------------------------
* Take a state code and return its position (ie. row index)
* in the SLR(1) parse table array
*---------------------------------------------------------------------
*/

static unsigned int stateCode2parseTableRow (t_stateCode stateCode)
{
 return ((unsigned int) (stateCode - DFA_STATE_START_CODE));
}

/*
*---------------------------------------------------------------------
* Take a symbol code and return its position (ie. column index)
* in the SLR(1) parse table array
*---------------------------------------------------------------------
*/

static unsigned int symbolCode2parseTableCol (t_symbolCode symbolCode)
{
 t_symbolType
   symbolType;

 symbolType = symbolCode2symbolType (symbolCode);
 switch (symbolType) {
   case (t_terminal):
     return ((unsigned int) (symbolCode - TERMINAL_START_CODE));
   case (t_nonTerminal):
     return ((unsigned int) (totTerminals + symbolCode - NON_TERMINAL_START_CODE));
   case (t_endOfInput):
     return ((unsigned int) (totTerminals + totNonTerminals));
   case (t_epsilon):   /* This should never happen! */
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Epsilon not allowed in parse table\n");
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
   default:   /* This should never happen! */
     snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown symbol type %d [%s]. Not allowed in parse table\n", symbolType, symbolCode2symbolString (symbolCode));
     ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
 }
}

/*
*---------------------------------------------------------------------
* Add a parse action to a parsing table
*---------------------------------------------------------------------
*/

static void addParseAction (
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode,
  t_parseAction      parseAction )
{
  unsigned int
    tableRow,
    tableCol;
  struct t_parseTableEntry
     *p_parseTablePos = NULL;
  t_parseTableSummary
    *p_parseTableSummaryPosInRow = NULL,
    *p_parseTableSummaryPosInCol = NULL;
  struct t_parseActionList
    *p_parseActionNode    = NULL,
    *p_newParseActionNode = NULL;

  tableRow = stateCode2parseTableRow (stateCode);
  tableCol = symbolCode2parseTableCol (symbolCode);

  /* Check a few things before actally adding anything to the parse table */

  /* 1st check: parser type */

  if (parse_table_type == t_LR0_parse_table) {
    p_parseTablePos             = &LR0parseTable[tableRow][tableCol];
    p_parseTableSummaryPosInRow = &LR0parseTableSummaryRow[tableCol];
    p_parseTableSummaryPosInCol = &LR0parseTableSummaryCol[tableRow];
  }
  else if (parse_table_type == t_sLR1_parse_table) {
    p_parseTablePos             = &sLR1parseTable[tableRow][tableCol];
    p_parseTableSummaryPosInRow = &sLR1parseTableSummaryRow[tableCol];
    p_parseTableSummaryPosInCol = &sLR1parseTableSummaryCol[tableRow];
  }
  else if (parse_table_type == t_diff_parse_table) {
    p_parseTablePos             = &diffParseTable[tableRow][tableCol];
    p_parseTableSummaryPosInRow = &diffParseTableSummaryRow[tableCol];
    p_parseTableSummaryPosInCol = &diffParseTableSummaryCol[tableRow];
  }
  else {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown parse table type %d (row=%u, column=%u, actionType=%d)\n",
      parse_table_type, tableRow, tableCol, parseAction.parseActionType);
    ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
  }

  /* 2nd check: parse action type */

  switch (parseAction.parseActionType) {
    case (t_accept):
    case (t_reduce):
    case (t_shift):
    case (t_goto):
      break;
    case (t_error): {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Do not add 'error' to parse table (parse_table_type=%d, row=%u, column=%u)\n",
        parse_table_type, tableRow, tableCol);
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
    }
    default: {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown parser action type %d (row=%u, column=%u)\n",
        parseAction.parseActionType, tableRow, tableCol);
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
    }
  }

  /* 3rd check: duplicate entries */

  for (p_parseActionNode = p_parseTablePos->p_parseActionList;
       (p_parseActionNode != NULL) && memcmp(&p_parseActionNode->parseAction, &parseAction, sizeof(parseAction));
       p_parseActionNode = p_parseActionNode->p_nextParseAction);
  if (p_parseActionNode != NULL)
    return;

  /* All checks OK, so add the parse action to the parse table */

  p_newParseActionNode = (struct t_parseActionList *) malloc (sizeof (struct t_parseActionList));
  if (p_newParseActionNode == NULL)
    ERROR_no_memory (0, __FILE__, __func__, "parseActionNode");
  p_newParseActionNode->p_nextParseAction = NULL;
  p_newParseActionNode->parseAction = parseAction;
  p_newParseActionNode->p_nextParseAction = p_parseTablePos->p_parseActionList;
  p_parseTablePos->p_parseActionList = p_newParseActionNode;
  (p_parseTablePos->totParseActions)++;

  /* Update array of summary statistics */

  switch (parseAction.parseActionType) {
    case (t_accept):
    case (t_reduce): {
      (p_parseTableSummaryPosInRow->totReductions)++;
      (p_parseTableSummaryPosInCol->totReductions)++;
      break;
    }
    case (t_shift): {
      (p_parseTableSummaryPosInRow->totShifts)++;
      (p_parseTableSummaryPosInCol->totShifts)++;
      break;
    }
    case (t_goto): {
      (p_parseTableSummaryPosInRow->totGotos)++;
      (p_parseTableSummaryPosInCol->totGotos)++;
      break;
    }
    default:
      break;
  }
}

/*
*---------------------------------------------------------------------
* Check if a parser action is in a given cell of a parse table
*---------------------------------------------------------------------
*/

static bool b_parseAction_isIn_parseTablePos (
  t_parseAction      parseAction,
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode )
{
 unsigned int
   iParseAction,
   totParseActions;
 t_parseAction
   currParseAction;

 totParseActions = parseTablePos2totParseActions (parse_table_type, stateCode, symbolCode);
 for (iParseAction = 1; iParseAction <= totParseActions; iParseAction++) {
   currParseAction = parseTablePos2parseAction (parse_table_type, stateCode, symbolCode, iParseAction);
   if ((currParseAction.parseActionType == parseAction.parseActionType) &&
       (currParseAction.parseActionParam.nextState == parseAction.parseActionParam.nextState) &&
       (currParseAction.parseActionParam.reductionRule == parseAction.parseActionParam.reductionRule) )
     return (true);
 }
 return (false);
}

/*
*---------------------------------------------------------------------
* Build LR(0) parsing table
*---------------------------------------------------------------------
*/

void build_LR0_parse_table (void)
{
 unsigned int
   iState,
   iItem,
   iSymbol,
   tableRow,
   tableColumn;
 t_symbolCode
   symbolCode,
   transitionSymbol;
 t_symbolType
   transitionSymbolType;
 t_stateCode
   stateCode,
   nextState;
 t_itemCode
   itemCode;
 t_ruleNumber
   ruleNumber;
 t_parseAction
   parseAction;

 /* Initialize the parse table */

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   tableRow = stateCode2parseTableRow (stateCode);
   for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     LR0parseTable[tableRow][tableColumn].totParseActions = 0;
     LR0parseTable[tableRow][tableColumn].p_parseActionList = NULL;
   }
   for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     LR0parseTable[tableRow][tableColumn].totParseActions = 0;
     LR0parseTable[tableRow][tableColumn].p_parseActionList = NULL;
   }
 }

 /* Populate parse table */

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   for (iItem = 1; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
     itemCode = dfa_stateCode2itemCode (stateCode, iItem);
     ruleNumber = itemCode2ruleNumber (itemCode);
     if (isReductionItem (itemCode)) {
       parseAction.parseActionType = (ruleNumber == 1 ? t_accept : t_reduce);
       parseAction.parseActionParam.reductionRule = ruleNumber;
       for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
         symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
         addParseAction (t_LR0_parse_table, stateCode, symbolCode, parseAction);
       }
     }
     else {
       transitionSymbol = itemCode2transitionSymbol (itemCode);
       transitionSymbolType = symbolCode2symbolType (transitionSymbol);
       nextState = dfa_lookupNextState (stateCode, transitionSymbol);
       if (nextState == UNKNOWN_STATE_CODE)
         continue;
       parseAction.parseActionParam.nextState = nextState;
       switch (transitionSymbolType) {
         case (t_terminal):
         case (t_endOfInput):
           parseAction.parseActionType = t_shift;
           break;
         case (t_nonTerminal):
           parseAction.parseActionType = t_goto;
           break;
         default:   /* This should never happen! */
           snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown symbol type %d [%s]. Not allowed in LR(0) parse table\n", transitionSymbolType, symbolCode2symbolString (transitionSymbol));
           ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       }
       addParseAction (t_LR0_parse_table, stateCode, transitionSymbol, parseAction);
     }
   }
 }
}

/*
*---------------------------------------------------------------------
* Build sLR(1) parsing table
*---------------------------------------------------------------------
*/

void build_sLR1_parse_table (void)
{
 unsigned int
   iState,
   iItem,
   iSymbol,
   iFollowSymbol,
   tableRow,
   tableColumn;
 t_symbolCode
   symbolCode,
   lefthandSymbol,
   transitionSymbol;
 t_symbolType
   transitionSymbolType;
 t_stateCode
   stateCode,
   nextState;
 t_itemCode
   itemCode;
 t_ruleNumber
   ruleNumber;
 t_parseAction
   parseAction;

 /* Initialize the parse table */

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   tableRow = stateCode2parseTableRow (stateCode);
   for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     sLR1parseTable[tableRow][tableColumn].totParseActions = 0;
     sLR1parseTable[tableRow][tableColumn].p_parseActionList = NULL;
   }
   for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     sLR1parseTable[tableRow][tableColumn].totParseActions = 0;
     sLR1parseTable[tableRow][tableColumn].p_parseActionList = NULL;
   }
 }

 /* Populate parse table */

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   for (iItem = 1; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
     itemCode = dfa_stateCode2itemCode (stateCode, iItem);
     ruleNumber = itemCode2ruleNumber (itemCode);
     if (isReductionItem (itemCode)) {
       parseAction.parseActionType = (ruleNumber == 1 ? t_accept : t_reduce);
       parseAction.parseActionParam.reductionRule = ruleNumber;
       if (ruleNumber == 1)
         addParseAction (t_sLR1_parse_table, stateCode, end_of_input_code, parseAction);
       else {
         lefthandSymbol = rulePos2symbolCode (ruleNumber, 0);
         for (iFollowSymbol = 1; iFollowSymbol <= setSize (t_followSet, lefthandSymbol); iFollowSymbol++) {
           symbolCode = getSymbolInSet (t_followSet, lefthandSymbol, iFollowSymbol);
           addParseAction (t_sLR1_parse_table, stateCode, symbolCode, parseAction);
         }
       }
     }
     else {
       transitionSymbol = itemCode2transitionSymbol (itemCode);
       transitionSymbolType = symbolCode2symbolType (transitionSymbol);
       nextState = dfa_lookupNextState (stateCode, transitionSymbol);
       if (nextState == UNKNOWN_STATE_CODE)
         continue;
       parseAction.parseActionParam.nextState = nextState;
       switch (transitionSymbolType) {
         case (t_terminal):
         case (t_endOfInput):
           parseAction.parseActionType = t_shift;
           break;
         case (t_nonTerminal):
           parseAction.parseActionType = t_goto;
           break;
         default:   /* This should never happen! */
           snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown symbol type %d [%s]. Not allowed in sLR(1) parse table\n", transitionSymbolType, symbolCode2symbolString (transitionSymbol));
           ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
       }
       addParseAction (t_sLR1_parse_table, stateCode, transitionSymbol, parseAction);
     }
   }
 }
}

/*
*---------------------------------------------------------------------
* Build table with differences between LR(0) and sLR(1) parse tables
*---------------------------------------------------------------------
*/

void build_diff_parse_table (void)
{
 unsigned int
   iState,
   iSymbol,
   iParseAction,
   tableRow,
   tableColumn,
   totLR0parseActions,
   totsLR1parseActions;
 t_symbolCode
   symbolCode;
 t_stateCode
   stateCode;
 t_parseAction
   LR0parseAction;

 /* Initialize the parse table */

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   tableRow = stateCode2parseTableRow (stateCode);
   for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     diffParseTable[tableRow][tableColumn].totParseActions = 0;
     diffParseTable[tableRow][tableColumn].p_parseActionList = NULL;
   }
   for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     diffParseTable[tableRow][tableColumn].totParseActions = 0;
     diffParseTable[tableRow][tableColumn].p_parseActionList = NULL;
   }
 }

 /* Populate parse table */

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
     totLR0parseActions = parseTablePos2totParseActions (t_LR0_parse_table, stateCode, symbolCode);
     totsLR1parseActions = parseTablePos2totParseActions (t_sLR1_parse_table, stateCode, symbolCode);
     if (totLR0parseActions == totsLR1parseActions)
       continue;
     for (iParseAction = 1; iParseAction <= totLR0parseActions; iParseAction++) {
       LR0parseAction = parseTablePos2parseAction (t_LR0_parse_table, stateCode, symbolCode, iParseAction);
       if (! b_parseAction_isIn_parseTablePos (LR0parseAction, t_sLR1_parse_table, stateCode, symbolCode))
         addParseAction (t_diff_parse_table, stateCode, symbolCode, LR0parseAction);
     }
   }
 }
}

/*
*---------------------------------------------------------------------
* Free all memory allocated to LR(0) parse table action nodes
*---------------------------------------------------------------------
*/

void free_LR0_parse_table_memory (void)
{
 unsigned int
   iState,
   iSymbol,
   tableRow,
   tableColumn;
 t_stateCode
   stateCode;
 t_symbolCode
   symbolCode;
 struct t_parseTableEntry
   *p_parseTableEntry = NULL;
 struct t_parseActionList
   *p_parseActionCurrNode = NULL,
   *p_parseActionNextNode = NULL;

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   tableRow = stateCode2parseTableRow (stateCode);
   for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     p_parseTableEntry = &(LR0parseTable[tableRow][tableColumn]);
     for (p_parseActionCurrNode = p_parseTableEntry->p_parseActionList;
          p_parseActionCurrNode != NULL;
          p_parseActionCurrNode = p_parseActionNextNode) {
       p_parseActionNextNode = p_parseActionCurrNode->p_nextParseAction;
       free (p_parseActionCurrNode);
     }
   }
   for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     p_parseTableEntry = &(LR0parseTable[tableRow][tableColumn]);
     for (p_parseActionCurrNode = p_parseTableEntry->p_parseActionList;
          p_parseActionCurrNode != NULL;
          p_parseActionCurrNode = p_parseActionNextNode) {
       p_parseActionNextNode = p_parseActionCurrNode->p_nextParseAction;
       free (p_parseActionCurrNode);
     }
   }
 }
}

/*
*---------------------------------------------------------------------
* Free all memory allocated to sLR(1) parse table action nodes
*---------------------------------------------------------------------
*/

void free_sLR1_parse_table_memory (void)
{
 unsigned int
   iState,
   iSymbol,
   tableRow,
   tableColumn;
 t_stateCode
   stateCode;
 t_symbolCode
   symbolCode;
 struct t_parseTableEntry
   *p_parseTableEntry = NULL;
 struct t_parseActionList
   *p_parseActionCurrNode = NULL,
   *p_parseActionNextNode = NULL;

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   tableRow = stateCode2parseTableRow (stateCode);
   for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     p_parseTableEntry = &(sLR1parseTable[tableRow][tableColumn]);
     for (p_parseActionCurrNode = p_parseTableEntry->p_parseActionList;
          p_parseActionCurrNode != NULL;
          p_parseActionCurrNode = p_parseActionNextNode) {
       p_parseActionNextNode = p_parseActionCurrNode->p_nextParseAction;
       free (p_parseActionCurrNode);
     }
   }
   for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     p_parseTableEntry = &(sLR1parseTable[tableRow][tableColumn]);
     for (p_parseActionCurrNode = p_parseTableEntry->p_parseActionList;
          p_parseActionCurrNode != NULL;
          p_parseActionCurrNode = p_parseActionNextNode) {
       p_parseActionNextNode = p_parseActionCurrNode->p_nextParseAction;
       free (p_parseActionCurrNode);
     }
   }
 }
}

/*
*---------------------------------------------------------------------
* Free all memory allocated to difference parse table action nodes
*---------------------------------------------------------------------
*/

void free_diff_parse_table_memory (void)
{
 unsigned int
   iState,
   iSymbol,
   tableRow,
   tableColumn;
 t_stateCode
   stateCode;
 t_symbolCode
   symbolCode;
 struct t_parseTableEntry
   *p_parseTableEntry = NULL;
 struct t_parseActionList
   *p_parseActionCurrNode = NULL,
   *p_parseActionNextNode = NULL;

 for (iState = 1; iState <= totDFAstates; iState++) {
   stateCode = dfa_stateNumber2stateCode (iState);
   tableRow = stateCode2parseTableRow (stateCode);
   for (iSymbol = 1; iSymbol <= totTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_terminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     p_parseTableEntry = &(diffParseTable[tableRow][tableColumn]);
     for (p_parseActionCurrNode = p_parseTableEntry->p_parseActionList;
          p_parseActionCurrNode != NULL;
          p_parseActionCurrNode = p_parseActionNextNode) {
       p_parseActionNextNode = p_parseActionCurrNode->p_nextParseAction;
       free (p_parseActionCurrNode);
     }
   }
   for (iSymbol = 1; iSymbol <= totNonTerminals; iSymbol++) {
     symbolCode = symbolNumber2symbolCode (iSymbol, t_nonTerminal);
     tableColumn = symbolCode2parseTableCol (symbolCode);
     p_parseTableEntry = &(diffParseTable[tableRow][tableColumn]);
     for (p_parseActionCurrNode = p_parseTableEntry->p_parseActionList;
          p_parseActionCurrNode != NULL;
          p_parseActionCurrNode = p_parseActionNextNode) {
       p_parseActionNextNode = p_parseActionCurrNode->p_nextParseAction;
       free (p_parseActionCurrNode);
     }
   }
 }
}

/*
*---------------------------------------------------------------------------
* Return the total number of parse actions of a given type in a given state
*---------------------------------------------------------------------------
*/

unsigned int parseTableSummaryCol2totParseActions (
  t_parse_table_type parse_table_type,
  t_parseActionType  parseActionType,
  t_stateCode        stateCode )
{
  unsigned int
    tableRow;
  t_parseTableSummary
     *p_parseTableSummaryPos = NULL;

  /* First, check if the parser type is valid */

  tableRow = stateCode2parseTableRow (stateCode);
  if (parse_table_type == t_LR0_parse_table)
    p_parseTableSummaryPos = &LR0parseTableSummaryCol[tableRow];
  else if (parse_table_type == t_sLR1_parse_table)
    p_parseTableSummaryPos = &sLR1parseTableSummaryCol[tableRow];
  else if (parse_table_type == t_diff_parse_table)
    p_parseTableSummaryPos = &diffParseTableSummaryCol[tableRow];
  else {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown parse table type %d (row=%u, actionType=%d)\n",
      parse_table_type, tableRow, parseActionType);
    ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
  }

  /* Now check if the parse action type is valid */

  switch (parseActionType) {
    case (t_reduce): return (p_parseTableSummaryPos->totReductions);
    case (t_shift) : return (p_parseTableSummaryPos->totShifts);
    case (t_goto)  : return (p_parseTableSummaryPos->totGotos);
    case (t_accept): {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid parse action type 't_accept' (parse table type=%d, state=%d)\n",
        parse_table_type, stateCode);
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
    }
    case (t_error): {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid parse action type 't_error' (parse table type=%d, state=%d)\n",
        parse_table_type, stateCode);
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
    }
    default: {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown parser action type %d (parse table type=%d, state=%d)\n",
        parseActionType, parse_table_type, stateCode);
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
    }
  }
}

/*
*-----------------------------------------------------------------------------
* Return the total number of parse actions of a given type for a given symbol
*-----------------------------------------------------------------------------
*/

unsigned int parseTableSummaryRow2totParseActions (
  t_parse_table_type parse_table_type,
  t_parseActionType  parseActionType,
  t_symbolCode       symbolCode )
{
  unsigned int
    tableCol;
  t_parseTableSummary
     *p_parseTableSummaryPos = NULL;

  /* First, check if the parser type is valid */

  tableCol = symbolCode2parseTableCol (symbolCode);
  if (parse_table_type == t_LR0_parse_table)
    p_parseTableSummaryPos = &LR0parseTableSummaryRow[tableCol];
  else if (parse_table_type == t_sLR1_parse_table)
    p_parseTableSummaryPos = &sLR1parseTableSummaryRow[tableCol];
  else if (parse_table_type == t_diff_parse_table)
    p_parseTableSummaryPos = &diffParseTableSummaryRow[tableCol];
  else {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown parser type %d (column=%u, actionType=%d)\n",
      parse_table_type, tableCol, parseActionType);
    ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
  }

  /* Now check if the parse action type is valid */

  switch (parseActionType) {
    case (t_reduce): return (p_parseTableSummaryPos->totReductions);
    case (t_shift) : return (p_parseTableSummaryPos->totShifts);
    case (t_goto)  : return (p_parseTableSummaryPos->totGotos);
    case (t_accept): {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid parse action type 't_accept' (parse table type=%d, symbolCode=%d)\n",
        parse_table_type, symbolCode);
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
    }
    case (t_error): {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid parse action type 't_error' (parse table type=%d, symbolCode=%d)\n",
        parse_table_type, symbolCode);
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
    }
    default: {
      snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Unknown parser action type %d (parse table type=%d, symbolCode=%d)\n",
        parseActionType, parse_table_type, symbolCode);
      ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
    }
  }
}


/*
*------------------------------------------------------------------------------
* THE FOLLOWING FUNCTIONS MAY BE USEFUL IN THE FUTURE. THEY ARE
* COMMENTED OUT SO AS TO PREVENT "defined but not used" COMPILER WARNINGS.
*------------------------------------------------------------------------------
*

*
*------------------------------------------------------------------------------
* Function prototypes
*------------------------------------------------------------------------------
*

static void              *pointer_to_state                 (t_stateCode  stateCode);
static bool               nfa_isDuplicateState             (t_stateCode  stateCode);
static t_stateCode        nfa_stateCode2duplicateStateCode (t_stateCode  stateCode);
static void               nfa_removeState                  (t_stateCode  stateCode);
static t_parse_table_type parserType2parseTableType        (t_parserType parserType);

*
*------------------------------------------------------------------------------
* Take a state code and return a pointer to it
*------------------------------------------------------------------------------
*

static void *pointer_to_state (t_stateCode stateCode)
{
 if (((int) stateCode >= (int) NFA_STATE_START_CODE) &&
     ((int) stateCode <= (int) (NFA_STATE_START_CODE + totNFAstates - 1)))
   return ((void *) &NFAstates[stateCode - NFA_STATE_START_CODE]);
 if (((int) stateCode >= (int) DFA_STATE_START_CODE) &&
     ((int) stateCode <= (int) (DFA_STATE_START_CODE + totDFAstates - 1)))
   return ((void *) &DFAstates[stateCode - DFA_STATE_START_CODE]);
 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid state code: %d\n", stateCode);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

*
*----------------------------------------------------------------------------
* Take a NFA state code and return a boolean indicating whether
* it is a duplicate of an existing state
*----------------------------------------------------------------------------
*

static bool nfa_isDuplicateState (t_stateCode stateCode)
{
 int
   iState,
   stateIndex;

 // Ensure the state code provided really is an NFA state

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d provided to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 stateIndex = stateCode - NFA_STATE_START_CODE;
 for (iState = 0; iState < (int) nfa_nextState; iState++) {
   if (iState == stateIndex)
     break;
   if (NFAstates[iState].itemIndex == stateIndex)
     return (true);
 }
 return (false);
}

*
*----------------------------------------------------------------------------
* Take a NFA state code and check if it is a duplicate of an existing state.
* If so, return the numeric code of the existing duplicate,
* otherwise return UNKNOWN_STATE_CODE
*----------------------------------------------------------------------------
*

static t_stateCode nfa_stateCode2duplicateStateCode (t_stateCode stateCode)
{
 int
   iState,
   stateIndex;

 // Ensure the state code provided really is an NFA state

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d provided to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 stateIndex = stateCode - NFA_STATE_START_CODE;
 for (iState = 0; iState < (int) nfa_nextState; iState++) {
   if (iState == stateIndex)
     break;
   if (NFAstates[iState].itemIndex == stateIndex)
     return ((t_stateCode) (iState + NFA_STATE_START_CODE));
 }
 return ((t_stateCode) UNKNOWN_STATE_CODE);
}

*
*----------------------------------------------------------------------------
* Take a NFA state code and delete it
*----------------------------------------------------------------------------
*

static void nfa_removeState (t_stateCode stateCode)
{
 int
   stateIndex;
 t_NFAstate
   *p_NFAstate;

 // Ensure the state code provided really is an NFA state

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "DFA state %d passed to %s(); expected NFA state\n", stateCode, __func__);
 validate_state_FSA_type (stateCode, t_NFA, ERROR_auxErrorMsg);

 stateIndex = stateCode - NFA_STATE_START_CODE;

 // If this state is not at the very end of the
 // NFAstates array then shift down array contents

 if (stateIndex < (int) nfa_nextState - 1)
   memmove (
     &NFAstates[stateIndex],
     &NFAstates[stateIndex + 1],
     (nfa_nextState - stateIndex - 1) * (sizeof (t_NFAstate)) );

 // Now clear the element at the end of the array

 p_NFAstate = &NFAstates[--nfa_nextState];
 p_NFAstate->itemIndex = UNKNOWN_ITEM_CODE;
 p_NFAstate->totInwardTransitions = p_NFAstate->totOutwardTransitions = 0;
 totNFAstates--;
}

*
*---------------------------------------------------------------------
* Take a parser type and return the corresponding parse table type
*---------------------------------------------------------------------
*

static t_parse_table_type parserType2parseTableType (t_parserType parserType)
{
 if (parserType == t_LR0_parser)
   return (t_LR0_parse_table);
 if (parserType == t_sLR1_parser)
   return (t_sLR1_parse_table);

 // This should never happen!

 snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Invalid parserType %d\n", parserType);
 ERROR_fatal_error (0, __FILE__, __func__, ERROR_auxErrorMsg);
}

*/
