/*
*-----------------------------------------------------------------------
*
*   File         : grammar.h
*   Created      : 2006-08-01
*   Last Modified: 2018-05-19
*
*   DESCRIPTION:
*   Processing of context-free grammars, LR(0) items, NFAs and DFAs,
*   LR(0) and sLR(1) parsers
*
*-----------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _GRAMMAR_DOT_H_
#define _GRAMMAR_DOT_H_

/*
*-----------------------------------------------------------------------
* Definitions for grammar symbols and rules
*-----------------------------------------------------------------------
*/

#define MAX_TERMINALS               100
#define MAX_NON_TERMINALS           100
#define MAX_RULES                   200
#define MAX_SYMBOLS_EACH_RIGHTHAND  50
#define MAX_LR0_ITEMS               ((MAX_RULES)*((MAX_SYMBOLS_EACH_RIGHTHAND)+1))  /* This is an exact number, not an estimate */
#define MAX_NFA_STATES              (MAX_LR0_ITEMS)   /* This is an exact number, not an estimate */
#define MAX_DFA_STATES              (MAX_LR0_ITEMS)   /* This is an estimate */

typedef enum {
  t_terminal,
  t_nonTerminal,
  t_epsilon,
  t_endOfInput
}
  t_symbolType;

typedef int
  t_symbolCode;     /* Terminals and non-terminals will get numeric codes */

typedef enum {      /* As we parse the grammar, we need to know if we are */
  t_lefthand,       /*   before the arrow, or                             */
  t_righthand       /*   after the arrow                                  */
}
  t_whereInRule;

extern unsigned int
  totRules,                 /* Total number of grammar rules                           */
  totTerminals,             /* Total number of terminals                               */
  totNonTerminals,          /* Total number of non-terminals                           */
  totGrammarSymbols,        /* totTerminals + totNonTerminals                          */
  totFSAsymbols;            /* totGrammarSymbols + 3 (epsilon, rule arrow and the dot) */

extern size_t
  maxTerminalLength,        /* Length of longest terminal string       */
  maxNonTerminalLength,     /* Length of longest non-terminal string   */
  maxGrammarSymbolLength;   /* Length of longest grammar symbol string */

extern float
  avgTerminalLength,        /* Average length of all terminal strings       */
  avgNonTerminalLength,     /* Average length of all non-terminal strings   */
  avgGrammarSymbolLength;   /* Average length of all grammar symbol strings */
                            /*   including epsilon and end-of-input         */
extern t_symbolCode
  end_of_input_code,
  epsilon_code,
  rule_arrow_code,
  item_dot_code,
  unknown_symbol_code;

/*
*-----------------------------------------------------------------------
* Definitions for LR(0) items, NFA and DFA states and transitions
*-----------------------------------------------------------------------
*/

typedef enum {
  t_NFA = 0,
  t_DFA
}
 t_FSA_type;

#define MIN_FSA_TYPE_VALUE  t_NFA
#define MAX_FSA_TYPE_VALUE  t_DFA

/*

  Each NFA and DFA state is classified as follows:

  +-------------------------------+-------+--------------------------+-------+-------+
  |                               |       |      TRANSITIONS         |       |       |
  | STATE TYPE                    | ITEMS +------+---------+---------+ REDUC | AMBIG |
  |                               |       | Term | NonTerm | Epsilon | TIONS | UOUS? |
  +-------------------------------+-------+------+---------+---------+-------+-------+
  | t_NFA_shift_state             |   1   |   1  |    0    |    0    |   0   |  no   |
  | t_NFA_reduce_state            |   1   |   0  |    0    |    0    |   1   |  no   |
  | t_NFA_non_deterministic_state |   1   |   0  |    1    |    1+   |   0   |  YES  |
  +-------------------------------+-------+------+---------+---------+-------+-------+
  | t_DFA_shift_state             |   1+  |       1+       |   N/A   |   0   |  no   |
  | t_DFA_1_reduce_state          |   1   |       0        |   N/A   |   1   |  no   |
  | t_DFA_N_reduce_state          |   2+  |       0        |   N/A   |   2+  |  YES  |
  | t_DFA_shift_1_reduce_state    |   2+  |       1+       |   N/A   |   1   |  YES  |
  | t_DFA_shift_N_reduce_state    |   3+  |       1+       |   N/A   |   2+  |  YES  |
  +-------------------------------+-------+----------------+---------+-------+-------+

  Examples, where
    a state looks like          { [item] , [item] , ... , [item] }
    a shift item looks like     [item]
    a reduction item looks like [[item]]

  +-----------------------------------+-------------------+
  | STATE TYPE                        | SAMPLE NFA STATES |
  +-----------------------------------+-------------------+
  | t_NFA_shift_state                 |  { [E->E.+T] }    |
  |                                   |  { [F->.n] }      |
  |                                   |                   |
  +-----------------------------------+-------------------+
  | t_NFA_reduce_state                |  { [[E->E+T.]] }  |
  |                                   |  { [[F->n.]] }    |
  |                                   |  { [[S->.]] }     |
  +-----------------------------------+-------------------+
  | t_NFA_non_deterministic_state     |  { [E->.E+T] }    |
  |                                   |  { [E->E+.T] }    |
  |                                   |  { [S->(.E)] }    |
  +-----------------------------------+-------------------+

  +-----------------------------------+-------------------------------------------------+
  | STATE TYPE                        |  SAMPLE DFA STATES                              |
  +-----------------------------------+-------------------------------------------------+
  | t_DFA_shift_state                 |  { [E->E.+T] }                                  |
  |                                   |  { [E->E+.T] , [E->.E+T] }                      |
  |                                   |  { [E->E+.T] , [T->.F] , [F->.n] }              |
  +-----------------------------------+-------------------------------------------------+
  | t_DFA_1_reduce_state              |  { [[E->E+T.]] }                                |
  |                                   |  { [[F->n.]] }                                  |
  |                                   |  { [[S->.]] }                                   |
  +-----------------------------------+-------------------------------------------------+
  | t_DFA_N_reduce_state              |  { [[E->E+T.]] , [[E->T.]] }                    |
  |                                   |  { [[S->(E).]] , [[S->n.]] , [[S->.]] }         |
  +-----------------------------------+-------------------------------------------------+
  | t_DFA_shift_1_reduce_state        |  { [T->.F] , [[T->F.]] }                        |
  |                                   |  { [T->.F] , [F->.n] , [[F->n.]] }              |
  |                                   |  { [E->E+.T] , [T->.F] , [F->.n] , [[F->n.]] }  |
  +-----------------------------------+-------------------------------------------------+
  | t_DFA_shift_N_reduce_state        |  { [T->.F] , [[F->n.]] , [[F->(E).]] }          |
  |                                   |  { [S->(E.)] , [[S->(E).]] , [[S->.]] }         |
  +-----------------------------------+-------------------------------------------------+

*/

typedef enum {
 t_NFA_shift_state = 0,
 t_NFA_reduce_state,
 t_NFA_non_deterministic_state,
 t_DFA_shift_state,
 t_DFA_1_reduce_state,
 t_DFA_N_reduce_state,
 t_DFA_shift_1_reduce_state,
 t_DFA_shift_N_reduce_state
}
 t_stateType;

extern unsigned int
  tot_NFA_shift_states,
  tot_NFA_reduce_states,
  tot_NFA_non_deterministic_states,
  tot_DFA_shift_states,
  tot_DFA_1_reduce_states,
  tot_DFA_N_reduce_states,
  tot_DFA_shift_1_reduce_states,
  tot_DFA_shift_N_reduce_states;

typedef unsigned int
  t_ruleNumber,    /* Rule number (1 onwards)                    */
  t_dotPosition;   /* Position of dot in LR(0) items (0 onwards) */

typedef int
  t_itemCode,      /* Numeric codes of LR(0) items */
  t_stateCode;     /* Numeric codes of DFA states  */

typedef enum {                      /* Three ways of sorting state transitions: */
  t_transitionSortKey_origin,       /*   - by state of origin                   */
  t_transitionSortKey_symbol,	      /*   - by transition symbol                 */
  t_transitionSortKey_destination   /*   - by destination state                 */
}
 t_transitionSortKey;

extern unsigned int
  totLR0items,         /* Total number of LR(0) items                                                    */
  totLR0itemSymbols,   /* Total number of symbols in all LR(0) items including rule arrows and item dots */
  totNFAstates,        /* Total number of NFA states                                                     */
  totNFAitems,         /* Total number of NFA items, equal to the number of LR(0) items                  */
  totNFAtransitions,   /* Total number of NFA state transitions                                          */
  totDFAstates,        /* Total number of DFA states                                                     */
  totDFAitems,         /* Total number of DFA items, probably greater than the number of LR(0) items     */
  totDFAtransitions;   /* Total number of DFA state transitions                                          */

/*
*-----------------------------------------------------------------------
* Definitions for FIRST and FOLLOW sets
*-----------------------------------------------------------------------
*/

typedef enum {
  t_firstSet,
  t_followSet
}
  t_setType;

/*
*-----------------------------------------------------------------------
* Definitions for LR(0) and SLR(1) parsers
*-----------------------------------------------------------------------
*/

typedef enum {
  t_LR0_parser,
  t_sLR1_parser
}
  t_parserType;

typedef enum {
  t_LR0_parse_table,
  t_sLR1_parse_table,
  t_diff_parse_table
}
  t_parse_table_type;

typedef enum {
  t_shift,
  t_reduce,
  t_goto,
  t_accept,
  t_error
}
  t_parseActionType;

typedef union {
  t_stateCode  nextState;
  t_ruleNumber reductionRule;
}
  t_parseActionParam;

typedef struct {
  t_parseActionType  parseActionType;
  t_parseActionParam parseActionParam;
}
  t_parseAction;

/*
*---------------------------------------------------------------------
* Function prototypes
*---------------------------------------------------------------------
*/

extern void read_grammar (char *grammarFileName, char *progName, bool b_stripoff_quotes);

/* Methods for grammar symbols and rules */

extern t_symbolCode  symbolNumber2symbolCode (unsigned int symbolNumber, t_symbolType symbolType);
extern t_symbolType  symbolCode2symbolType   (t_symbolCode symbolCode);
extern const char   *symbolCode2symbolString (t_symbolCode symbolCode);
extern unsigned int  ruleNumber2ruleSize     (t_ruleNumber ruleNumber);
extern t_symbolCode  rulePos2symbolCode      (t_ruleNumber ruleNumber, unsigned int rulePos);
extern unsigned int  symbolCode2totUses      (t_symbolCode symbolCode, t_whereInRule whereInRule);
extern t_ruleNumber  symbolCode2use          (t_symbolCode symbolCode, t_whereInRule whereInRule, unsigned int useNumber);
extern bool          isEpsilonRule           (t_ruleNumber ruleNumber);

/* Methods for LR(0) items */

extern bool          isReductionItem                (t_itemCode itemCode);
extern t_itemCode    itemNumber2itemCode            (unsigned int itemNumber);
extern t_ruleNumber  itemCode2ruleNumber            (t_itemCode itemCode);
extern t_dotPosition itemCode2dotPosition           (t_itemCode itemCode);
extern t_symbolCode  itemCode2transitionSymbol      (t_itemCode itemCode);
extern t_itemCode    ruleNumberDotPosition2itemCode (t_ruleNumber ruleNumber, t_dotPosition dotPosition);

/* Methods for NFA and DFA states and transitions */

extern t_stateType   stateCode2stateType       (t_stateCode stateCode);
extern const char   *stateType2stateTypeString (t_stateType stateType);

extern t_stateCode   nfa_stateNumber2stateCode   (unsigned int stateNumber);
extern unsigned int  nfa_stateCode2stateNumber   (t_stateCode stateCode);
extern t_itemCode    nfa_stateCode2itemCode      (t_stateCode stateCode);
extern t_ruleNumber  nfa_stateCode2reductionRule (t_stateCode stateCode);
extern bool          nfa_isReductionState        (t_stateCode stateCode);

extern t_stateCode   dfa_stateNumber2stateCode   (unsigned int stateNumber);
extern unsigned int  dfa_stateCode2stateNumber   (t_stateCode stateCode);
extern unsigned int  dfa_stateCode2totItems      (t_stateCode stateCode);
extern t_itemCode    dfa_stateCode2itemCode      (t_stateCode stateCode, unsigned int itemNumber);
extern unsigned int  dfa_stateCode2totReductions (t_stateCode stateCode);
extern t_ruleNumber  dfa_stateCode2reductionRule (t_stateCode stateCode, unsigned int reductionNumber);
extern bool          dfa_isReductionState        (t_stateCode stateCode);
extern t_stateCode   dfa_lookupNextState         (t_stateCode currStateCode, t_symbolCode transitionSymbol);

extern void          nfa_sortTransitions                     (t_transitionSortKey sortKey);
extern t_stateCode   nfa_transitionNumber2originState        (unsigned int transitionNumber);
extern t_symbolCode  nfa_transitionNumber2symbol             (unsigned int transitionNumber);
extern t_stateCode   nfa_transitionNumber2destState          (unsigned int transitionNumber);
extern unsigned int  nfa_symbolCode2totTransitionsWithSymbol (t_symbolCode symbolCode);
extern unsigned int  nfa_stateCode2totTransitionsFromState   (t_stateCode stateCode);
extern unsigned int  nfa_stateCode2totTransitionsToState     (t_stateCode stateCode);
extern t_symbolCode  nfa_stateCode2transitionSymbol          (t_stateCode stateCode, unsigned int whichSymbol);
extern unsigned int  nfa_stateCode2transitionNumber          (t_stateCode stateCode, unsigned int whichTransition);

extern void          dfa_sortTransitions                     (t_transitionSortKey sortKey);
extern t_stateCode   dfa_transitionNumber2originState        (unsigned int transitionNumber);
extern t_symbolCode  dfa_transitionNumber2symbol             (unsigned int transitionNumber);
extern t_stateCode   dfa_transitionNumber2destState          (unsigned int transitionNumber);
extern unsigned int  dfa_symbolCode2totTransitionsWithSymbol (t_symbolCode symbolCode);
extern unsigned int  dfa_stateCode2totTransitionsFromState   (t_stateCode stateCode);
extern unsigned int  dfa_stateCode2totTransitionsToState     (t_stateCode stateCode);
extern t_symbolCode  dfa_stateCode2transitionSymbol          (t_stateCode stateCode, unsigned int whichSymbol);
extern unsigned int  dfa_stateCode2transitionNumber          (t_stateCode stateCode, unsigned int whichTransition);

extern void build_LR0_items_NFA_and_DFA (void);

/* Methods for FIRST and FOLLOW sets */

extern void          build_first_sets  (void);
extern void          build_follow_sets (void);
extern unsigned int  setSize           (t_setType setType, t_symbolCode symbolCode);
extern t_symbolCode  getSymbolInSet    (t_setType setType, t_symbolCode symbolCode, unsigned int posInSet);

/* Methods for LR(0) and SLR(1) parser tables */

extern void build_LR0_parse_table  (void);
extern void build_sLR1_parse_table (void);
extern void build_diff_parse_table (void);

extern void free_LR0_parse_table_memory  (void);
extern void free_sLR1_parse_table_memory (void);
extern void free_diff_parse_table_memory (void);

extern unsigned int parseTablePos2totParseActions (
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode );

extern t_parseAction parseTablePos2parseAction (
  t_parse_table_type parse_table_type,
  t_stateCode        stateCode,
  t_symbolCode       symbolCode,
  unsigned int       actionNumber );

extern unsigned int parseTableSummaryCol2totParseActions (
  t_parse_table_type parse_table_type,
  t_parseActionType  parseActionType,
  t_stateCode        stateCode );

extern unsigned int parseTableSummaryRow2totParseActions (
  t_parse_table_type parse_table_type,
  t_parseActionType  parseActionType,
  t_symbolCode       symbolCode );

#endif /* ifndef _GRAMMAR_DOT_H_ */
