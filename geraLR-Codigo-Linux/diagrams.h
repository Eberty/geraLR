/*
*-----------------------------------------------------------------------
*
*   File         : diagrams.h
*   Created      : 2009-12-01
*   Last Modified: 2018-05-21
*
*   NFA and DFA diagram generation in plain text and SVG formats.
*
*   >>>  DO NOT EDIT OR SAVE THIS FILE WITH THE DEV-C++ EDITOR     <<<
*   >>>  AS IT ISN'T UNICODE COMPLIANT AND WILL MESS UP THE SVG    <<<
*   >>>  GENERATION (THE RULE ARROW, THE DOT AND EPSILON SYMBOLS   <<<
*   >>>  ARE NOT PLAIN ASCII BUT UNICODE CHARS)                    <<<
*   >>>                                                            <<<
*   >>>  USE ONLY A UNICODE-AWARE TEXT EDITOR (LIKE NOTEPAD++) OR  <<<
*   >>>  IDE (LIKE NETBEANS) BUT ENSURE IT IS PROPERLY CONFIGURED  <<<
*
*-----------------------------------------------------------------------
*/

/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _DIAGRAMS_DOT_H_
#define _DIAGRAMS_DOT_H_

/*
 *---------------------------------------------------------------------
 *   INCLUDE FILES
 *---------------------------------------------------------------------
 */

 #include <stdbool.h>
 
/*
 *---------------------------------------------------------------------
 *   Type definitions
 *---------------------------------------------------------------------
 */

typedef float
  t_realNumber;

#define REAL_ZERO ((t_realNumber) 0.0)

#ifdef _WIN32
# define lidas "lidas.exe"
#else
# define lidas "./lidas"
#endif

typedef struct {
  const char *terminal,
             *non_terminal,
             *end_of_input,
             *epsilon,
             *rule_arrow,
             *item_dot,
             *state_label_prefix,
             *state_label_number;
} 
  t_font_colour;

typedef struct {
  double terminal,
         non_terminal,
         end_of_input,
         epsilon,
         rule_arrow,
         item_dot,
         state_label_prefix,
         state_label_number;
} 
  t_font_size;

typedef struct {
  const char *terminal,
             *non_terminal,
             *end_of_input,
             *epsilon,
             *rule_arrow,
             *item_dot,
             *state_label_prefix,
             *state_label_number;
} 
  t_font_name;

typedef struct {
  t_font_name name;
  t_font_size size;
  t_font_colour colour;
} 
  t_font;

typedef struct {
  double weight;
  const char * style;
  const char * color;
  const char * dir;
  int tailclip;
  int headclip;
  const char * arrowhead;
  const char * arrowtail;
  double arrowsize;
  double labeldistance;
  int decorate;
  int constraint;
  int minlen;
  double penwidth;
} 
  t_edge;

typedef struct {
  double height;
  double width;
  int fixedsize;
  const char * shape;
  const char * color;
  const char * fillcolor;
  const char * style;
  int regular;
  int peripheries;
  int sides;
  double orientation;
  double distortion;
  double skew;
  double penwidth;
  double margin;
} 
  t_node;

typedef struct {
  const char * size;
  const char * ratio;
  double margin;
  const char * ordering;
  int rotate;
  const char * color;
  const char * bgcolor;
  const char * splines;
  double nodesep;
  double ranksep;
  const char * rankdir;
  const char * rank;
} 
  t_graph;

typedef struct {
  const char * prefix;
  /*const char * numberFormat;*/
} 
  t_stateLabel;

typedef struct {
  const char *arrow_string,
             *dot_string,
             *epsilon_string,
             *end_of_input_string;
} 
  t_symbols;

typedef struct {
  t_symbols metasymbol;
  t_stateLabel stateLabel;
  t_graph graphAttribute;
  t_node state[3];
  t_edge transitionArrow;
  t_font font;
} 
  t_fsa_svg_attributes;

typedef enum {
  /* How special symbols are to be printed: epsilon, the derivation arrow and the item dot */
  nfa_rule_arrow_string = 0,
  nfa_item_dot_string,
  nfa_epsilon_string,
  nfa_end_of_input_string,
  
  /* How state ids are to be printed, e.g. "e2" and "e123"; "state 002" and "state 123"; etc */
  nfa_state_label_prefix,

  /* Graph attributes */
  nfa_graph_attribute_size,
  nfa_graph_attribute_ratio,
  nfa_graph_attribute_margin,
  nfa_graph_attribute_ordering,
  nfa_graph_attribute_rotate,
  nfa_graph_attribute_color,
  nfa_graph_attribute_bgcolor,
  nfa_graph_attribute_splines,
  nfa_graph_attribute_nodesep,
  nfa_graph_attribute_ranksep,
  nfa_graph_attribute_rankdir,
  nfa_graph_attribute_rank,

  /* [nodes] Shift states */
  nfa_shift_state_height,
  nfa_shift_state_width,
  nfa_shift_state_fixedsize,
  nfa_shift_state_shape,
  nfa_shift_state_color,
  nfa_shift_state_fillcolor,
  nfa_shift_state_style,
  nfa_shift_state_regular,
  nfa_shift_state_peripheries,
  nfa_shift_state_sides,
  nfa_shift_state_orientation,
  nfa_shift_state_distortion,
  nfa_shift_state_skew,
  nfa_shift_state_penwidth,
  nfa_shift_state_margin,

  /* [nodes] Epsilon states */
  nfa_epsilon_state_height,
  nfa_epsilon_state_width,
  nfa_epsilon_state_fixedsize,
  nfa_epsilon_state_shape,
  nfa_epsilon_state_color,
  nfa_epsilon_state_fillcolor,
  nfa_epsilon_state_style,
  nfa_epsilon_state_regular,
  nfa_epsilon_state_peripheries,
  nfa_epsilon_state_sides,
  nfa_epsilon_state_orientation,
  nfa_epsilon_state_distortion,
  nfa_epsilon_state_skew,
  nfa_epsilon_state_penwidth,
  nfa_epsilon_state_margin,

  /* [nodes] Reduction states */
  nfa_reduce_state_height,
  nfa_reduce_state_width,
  nfa_reduce_state_fixedsize,
  nfa_reduce_state_shape,
  nfa_reduce_state_color,
  nfa_reduce_state_fillcolor,
  nfa_reduce_state_style,
  nfa_reduce_state_regular,
  nfa_reduce_state_peripheries,
  nfa_reduce_state_sides,
  nfa_reduce_state_orientation,
  nfa_reduce_state_distortion,
  nfa_reduce_state_skew,
  nfa_reduce_state_penwidth,
  nfa_reduce_state_margin,

  /* [edge] Arrow indicating state transition */
  nfa_transition_arrow_weight,
  nfa_transition_arrow_style,
  nfa_transition_arrow_color,
  nfa_transition_arrow_dir,
  nfa_transition_arrow_tailclip,
  nfa_transition_arrow_headclip,
  nfa_transition_arrow_arrowhead,
  nfa_transition_arrow_arrowtail,
  nfa_transition_arrow_arrowsize,
  nfa_transition_arrow_labeldistance,
  nfa_transition_arrow_decorate,
  nfa_transition_arrow_constraint,
  nfa_transition_arrow_minlen,
  nfa_transition_arrow_penwidth,

  /* Fonts (type faces) for various diagram elements */
  nfa_font_name_terminal,
  nfa_font_name_non_terminal,
  nfa_font_name_end_of_input,
  nfa_font_name_epsilon,
  nfa_font_name_rule_arrow,
  nfa_font_name_item_dot,
  nfa_font_name_state_label_prefix,
  nfa_font_name_state_label_number,

  /* Font sizes for various diagram elements */
  nfa_font_size_terminal,
  nfa_font_size_non_terminal,
  nfa_font_size_end_of_input,
  nfa_font_size_epsilon,
  nfa_font_size_rule_arrow,
  nfa_font_size_item_dot,
  nfa_font_size_state_label_prefix,
  nfa_font_size_state_label_number,

  /* Font colours for various diagram elements */
  nfa_font_colour_terminal,
  nfa_font_colour_non_terminal,
  nfa_font_colour_end_of_input,
  nfa_font_colour_epsilon,
  nfa_font_colour_rule_arrow,
  nfa_font_colour_item_dot,
  nfa_font_colour_state_label_prefix,
  nfa_font_colour_state_label_number
} 
  t_enum_nfa_svg_attributes;

typedef enum {
  /* How special symbols are to be printed: epsilon, the derivation arrow and the item dot */
  dfa_rule_arrow_string = 0,
  dfa_item_dot_string,
  dfa_epsilon_string,
  dfa_end_of_input_string,
  
  /* How state ids are to be printed, e.g. "e2" and "e123"; "state 002" and "state 123"; etc */
  dfa_state_label_prefix,

  /* Graph attributes */
  dfa_graph_attribute_size,
  dfa_graph_attribute_ratio,
  dfa_graph_attribute_margin,
  dfa_graph_attribute_ordering,
  dfa_graph_attribute_rotate,
  dfa_graph_attribute_color,
  dfa_graph_attribute_bgcolor,
  dfa_graph_attribute_splines,
  dfa_graph_attribute_nodesep,
  dfa_graph_attribute_ranksep,
  dfa_graph_attribute_rankdir,
  dfa_graph_attribute_rank,

  /* [nodes] Shift states */
  dfa_shift_state_height,
  dfa_shift_state_width,
  dfa_shift_state_fixedsize,
  dfa_shift_state_shape,
  dfa_shift_state_color,
  dfa_shift_state_fillcolor,
  dfa_shift_state_style,
  dfa_shift_state_regular,
  dfa_shift_state_peripheries,
  dfa_shift_state_sides,
  dfa_shift_state_orientation,
  dfa_shift_state_distortion,
  dfa_shift_state_skew,
  dfa_shift_state_penwidth,
  dfa_shift_state_margin,

  /* [nodes] Conflict states */
  dfa_conflict_state_height,
  dfa_conflict_state_width,
  dfa_conflict_state_fixedsize,
  dfa_conflict_state_shape,
  dfa_conflict_state_color,
  dfa_conflict_state_fillcolor,
  dfa_conflict_state_style,
  dfa_conflict_state_regular,
  dfa_conflict_state_peripheries,
  dfa_conflict_state_sides,
  dfa_conflict_state_orientation,
  dfa_conflict_state_distortion,
  dfa_conflict_state_skew,
  dfa_conflict_state_penwidth,
  dfa_conflict_state_margin,

  /* [nodes] Reduction states */
  dfa_reduce_state_height,
  dfa_reduce_state_width,
  dfa_reduce_state_fixedsize,
  dfa_reduce_state_shape,
  dfa_reduce_state_color,
  dfa_reduce_state_fillcolor,
  dfa_reduce_state_style,
  dfa_reduce_state_regular,
  dfa_reduce_state_peripheries,
  dfa_reduce_state_sides,
  dfa_reduce_state_orientation,
  dfa_reduce_state_distortion,
  dfa_reduce_state_skew,
  dfa_reduce_state_penwidth,
  dfa_reduce_state_margin,

  /* [edge] Arrow indicating state transition */
  dfa_transition_arrow_weight,
  dfa_transition_arrow_style,
  dfa_transition_arrow_color,
  dfa_transition_arrow_dir,
  dfa_transition_arrow_tailclip,
  dfa_transition_arrow_headclip,
  dfa_transition_arrow_arrowhead,
  dfa_transition_arrow_arrowtail,
  dfa_transition_arrow_arrowsize,
  dfa_transition_arrow_labeldistance,
  dfa_transition_arrow_decorate,
  dfa_transition_arrow_constraint,
  dfa_transition_arrow_minlen,
  dfa_transition_arrow_penwidth,

  /* Fonts (type faces) for various diagram elements */
  dfa_font_name_terminal,
  dfa_font_name_non_terminal,
  dfa_font_name_end_of_input,
  dfa_font_name_epsilon,
  dfa_font_name_rule_arrow,
  dfa_font_name_item_dot,
  dfa_font_name_state_label_prefix,
  dfa_font_name_state_label_number,

  /* Font sizes for various diagram elements */
  dfa_font_size_terminal,
  dfa_font_size_non_terminal,
  dfa_font_size_end_of_input,
  dfa_font_size_epsilon,
  dfa_font_size_rule_arrow,
  dfa_font_size_item_dot,
  dfa_font_size_state_label_prefix,
  dfa_font_size_state_label_number,

  /* Font colours for various diagram elements */
  dfa_font_colour_terminal,
  dfa_font_colour_non_terminal,
  dfa_font_colour_end_of_input,
  dfa_font_colour_epsilon,
  dfa_font_colour_rule_arrow,
  dfa_font_colour_item_dot,
  dfa_font_colour_state_label_prefix,
  dfa_font_colour_state_label_number
} 
  t_enum_dfa_svg_attributes;

/* Last itens of enum */
#define LAST_t_enum_nfa nfa_font_colour_state_label_number
#define LAST_t_enum_dfa dfa_font_colour_state_label_number
#define LAST_t_enum_fsa LAST_t_enum_dfa

/*
 * ---------------------------------------------------------------------------------------------------------------
 * SVG VIEWPORTS AND COORDINATE SYSTEMS
 * ---------------------------------------------------------------------------------------------------------------
 * VIEWPORT NESTING:
 *  
 *   Level 1 -> Canvas (Graph attributes)
 *   Level 2 ---> State transition
 *   Level 2 ---> State group
 *   Level 3 -----> State header
 *   Level 3 -----> State core
 *   Level 4 -------> State item
 * ---------------------------------------------------------------------------------------------------------------
 */

/*
 *-----------------------------------------------------------------------
 *   Definition of constants
 *-----------------------------------------------------------------------
 */

/* Graph header and animation attributes */

#define GRAPH_LABEL_TYPE                                                "digraph"
#define GRAPH_LABEL_INIT                                           	    "{ \n"
#define GRAPH_LABEL_STOP                                                "} \n"
#define GRAPH_LABEL_NFA_NAME                                            "NFA"
#define GRAPH_LABEL_DFA_NAME                                            "DFA"

#define GRAPH_LABEL_START                                               "graph["
#define NODE_LABEL_START                                                "node["
#define EDGE_LABEL_START                                                "edge["
#define LABEL_END                                                       "];\n"

#define ID_LABEL_TRANSITION                                             "trans_"
#define ID_LABEL_STATE                                                  "state_"
#define ID_LABEL_ITEM                                                   "item_"
#define ID_LABEL_TITLE                                                  "_title"
#define ID_DEFINE_ITEMS                                                 "itens_"

#define BUG_FIXED                                                       "href=\"#\""
#define INDENT                                                          "  "

#define ANIMATION_DELAY		                                              0.5
#define ANIMATION_BLINKS                                                2

#define NFA_TABLE_BORDER                                                0
#define NFA_TABLE_SIDES                                                 "B"     /* T=top, B=bottom, L=left, R=right */
#define NFA_TABLE_ALIGN                                                 "LEFT"

#define DFA_TABLE_BORDER                                                0
#define DFA_TABLE_SIDES                                                 "B"     /* T=top, B=bottom, L=left, R=right */
#define DFA_TABLE_ALIGN                                                 "LEFT"

/*---------------------------------------------------------------*/
/* Default values for NFA (non-deterministic finite automaton).  */
/* Can be overridden by settings stored in a configuration file. */
/*---------------------------------------------------------------*/

/* How special symbols are to be printed: epsilon, the derivation arrow and the item dot */
#define DEFAULT_NFA_RULE_ARROW_STRING                                   "→ "
#define DEFAULT_NFA_ITEM_DOT_STRING                                     "•"
#define DEFAULT_NFA_EPSILON_STRING                                      "ε"
#define DEFAULT_NFA_END_OF_INPUT_STRING                                 "$"

/* How state IDs are to be printed, e.g. "e2" and "e123"; "State 002" and "State 123"; etc */
#define DEFAULT_NFA_STATE_LABEL_PREFIX                                  "Estado "     /* E.g. "e", "E", "Estado ", "Est_", "State ", "s" */
#define DEFAULT_NFA_STATE_LABEL_NUMBER_FORMAT                           "%d"          /* E.g. "%d", "%3d", "%03d"                        */

/* Graph attributes */
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_SIZE                                "auto"
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_RATIO                               "auto"
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_MARGIN                              0.0
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_ORDERING                            ""
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_ROTATE                              0
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_COLOR                               "black"
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_BGCOLOR                             "transparent"
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_SPLINES                             "spline"
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_NODESEP                             0.25
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_RANKSEP                             0.5
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_RANKDIR                             "LR"
#define DEFAULT_NFA_GRAPH_ATTRIBUTE_RANK                                "same"

/* [NODES] Shift states */
#define DEFAULT_NFA_SHIFT_STATE_HEIGHT                                  0.5
#define DEFAULT_NFA_SHIFT_STATE_WIDTH                                   0.75
#define DEFAULT_NFA_SHIFT_STATE_FIXEDSIZE                               false
#define DEFAULT_NFA_SHIFT_STATE_SHAPE                                   "rectangle"
#define DEFAULT_NFA_SHIFT_STATE_COLOR                                   "#00BFFF"  /* "deepskyblue" */
#define DEFAULT_NFA_SHIFT_STATE_FILLCOLOR                               "#F0F8FF"  /* "aliceblue"   */
#define DEFAULT_NFA_SHIFT_STATE_STYLE                                   "filled,rounded"
#define DEFAULT_NFA_SHIFT_STATE_REGULAR                                 false
#define DEFAULT_NFA_SHIFT_STATE_PERIPHERIES                             1
#define DEFAULT_NFA_SHIFT_STATE_SIDES                                   4
#define DEFAULT_NFA_SHIFT_STATE_ORIENTATION                             0.0
#define DEFAULT_NFA_SHIFT_STATE_DISTORTION                              0.0
#define DEFAULT_NFA_SHIFT_STATE_SKEW                                    0.0
#define DEFAULT_NFA_SHIFT_STATE_PENWIDTH                                2.0
#define DEFAULT_NFA_SHIFT_STATE_MARGIN                              	0.0

/* [NODES] Epsilon states */
#define DEFAULT_NFA_EPSILON_STATE_HEIGHT                                DEFAULT_NFA_SHIFT_STATE_HEIGHT
#define DEFAULT_NFA_EPSILON_STATE_WIDTH                                 DEFAULT_NFA_SHIFT_STATE_WIDTH
#define DEFAULT_NFA_EPSILON_STATE_FIXEDSIZE                             DEFAULT_NFA_SHIFT_STATE_FIXEDSIZE
#define DEFAULT_NFA_EPSILON_STATE_SHAPE                                 DEFAULT_NFA_SHIFT_STATE_SHAPE
#define DEFAULT_NFA_EPSILON_STATE_COLOR                                 DEFAULT_NFA_SHIFT_STATE_COLOR
#define DEFAULT_NFA_EPSILON_STATE_FILLCOLOR                             DEFAULT_NFA_SHIFT_STATE_FILLCOLOR
#define DEFAULT_NFA_EPSILON_STATE_STYLE                                 DEFAULT_NFA_SHIFT_STATE_STYLE
#define DEFAULT_NFA_EPSILON_STATE_REGULAR                               DEFAULT_NFA_SHIFT_STATE_REGULAR
#define DEFAULT_NFA_EPSILON_STATE_PERIPHERIES                           DEFAULT_NFA_SHIFT_STATE_PERIPHERIES
#define DEFAULT_NFA_EPSILON_STATE_SIDES                                 DEFAULT_NFA_SHIFT_STATE_SIDES
#define DEFAULT_NFA_EPSILON_STATE_ORIENTATION                           DEFAULT_NFA_SHIFT_STATE_ORIENTATION
#define DEFAULT_NFA_EPSILON_STATE_DISTORTION                            DEFAULT_NFA_SHIFT_STATE_DISTORTION
#define DEFAULT_NFA_EPSILON_STATE_SKEW                                  DEFAULT_NFA_SHIFT_STATE_SKEW
#define DEFAULT_NFA_EPSILON_STATE_PENWIDTH                              DEFAULT_NFA_SHIFT_STATE_PENWIDTH
#define DEFAULT_NFA_EPSILON_STATE_MARGIN                           	  	DEFAULT_NFA_SHIFT_STATE_MARGIN

/* [NODES] Reduction states */
#define DEFAULT_NFA_REDUCE_STATE_HEIGHT                                 DEFAULT_NFA_SHIFT_STATE_HEIGHT
#define DEFAULT_NFA_REDUCE_STATE_WIDTH                                  DEFAULT_NFA_SHIFT_STATE_WIDTH
#define DEFAULT_NFA_REDUCE_STATE_FIXEDSIZE                              DEFAULT_NFA_SHIFT_STATE_FIXEDSIZE
#define DEFAULT_NFA_REDUCE_STATE_SHAPE                                  DEFAULT_NFA_SHIFT_STATE_SHAPE
#define DEFAULT_NFA_REDUCE_STATE_COLOR                                  "#00BFFF"  /* "deepskyblue" */
#define DEFAULT_NFA_REDUCE_STATE_FILLCOLOR                              "#F0F8FF"  /* "aliceblue"   */
#define DEFAULT_NFA_REDUCE_STATE_STYLE                                  DEFAULT_NFA_SHIFT_STATE_STYLE
#define DEFAULT_NFA_REDUCE_STATE_REGULAR                                DEFAULT_NFA_SHIFT_STATE_REGULAR
#define DEFAULT_NFA_REDUCE_STATE_PERIPHERIES                            2
#define DEFAULT_NFA_REDUCE_STATE_SIDES                                  DEFAULT_NFA_SHIFT_STATE_SIDES
#define DEFAULT_NFA_REDUCE_STATE_ORIENTATION                            DEFAULT_NFA_SHIFT_STATE_ORIENTATION
#define DEFAULT_NFA_REDUCE_STATE_DISTORTION                             DEFAULT_NFA_SHIFT_STATE_DISTORTION
#define DEFAULT_NFA_REDUCE_STATE_SKEW                                   DEFAULT_NFA_SHIFT_STATE_SKEW
#define DEFAULT_NFA_REDUCE_STATE_PENWIDTH                               DEFAULT_NFA_SHIFT_STATE_PENWIDTH
#define DEFAULT_NFA_REDUCE_STATE_MARGIN                            		  DEFAULT_NFA_SHIFT_STATE_MARGIN

/* [EDGE] Arrow indicating state transition */
#define DEFAULT_NFA_TRANSITION_ARROW_WEIGHT                             1.0
#define DEFAULT_NFA_TRANSITION_ARROW_STYLE                              ""
#define DEFAULT_NFA_TRANSITION_ARROW_COLOR                              DEFAULT_NFA_SHIFT_STATE_COLOR
#define DEFAULT_NFA_TRANSITION_ARROW_DIR                                "forward"
#define DEFAULT_NFA_TRANSITION_ARROW_TAILCLIP                           true
#define DEFAULT_NFA_TRANSITION_ARROW_HEADCLIP                           true
#define DEFAULT_NFA_TRANSITION_ARROW_ARROWHEAD                          "normal"
#define DEFAULT_NFA_TRANSITION_ARROW_ARROWTAIL                          "normal"
#define DEFAULT_NFA_TRANSITION_ARROW_ARROWSIZE                          1.0
#define DEFAULT_NFA_TRANSITION_ARROW_LABELDISTANCE                      1.0
#define DEFAULT_NFA_TRANSITION_ARROW_DECORATE                           false
#define DEFAULT_NFA_TRANSITION_ARROW_CONSTRAINT                         true
#define DEFAULT_NFA_TRANSITION_ARROW_MINLEN                             1
#define DEFAULT_NFA_TRANSITION_ARROW_PENWIDTH                           1.5

/* Fonts (type faces) for various diagram elements */
#define DEFAULT_NFA_FONT_NAME_TERMINAL                                  "Times-Roman"
#define DEFAULT_NFA_FONT_NAME_NON_TERMINAL                              DEFAULT_NFA_FONT_NAME_TERMINAL
#define DEFAULT_NFA_FONT_NAME_END_OF_INPUT                              DEFAULT_NFA_FONT_NAME_TERMINAL
#define DEFAULT_NFA_FONT_NAME_EPSILON                                   DEFAULT_NFA_FONT_NAME_TERMINAL
#define DEFAULT_NFA_FONT_NAME_RULE_ARROW                                DEFAULT_NFA_FONT_NAME_TERMINAL
#define DEFAULT_NFA_FONT_NAME_ITEM_DOT                                  DEFAULT_NFA_FONT_NAME_TERMINAL
#define DEFAULT_NFA_FONT_NAME_STATE_LABEL_PREFIX                        "Calibri"
#define DEFAULT_NFA_FONT_NAME_STATE_LABEL_NUMBER                        DEFAULT_NFA_FONT_NAME_STATE_LABEL_PREFIX

/* Font sizes for various diagram elements */
#define DEFAULT_NFA_FONT_SIZE_TERMINAL                                  14.0
#define DEFAULT_NFA_FONT_SIZE_NON_TERMINAL                              DEFAULT_NFA_FONT_SIZE_TERMINAL
#define DEFAULT_NFA_FONT_SIZE_END_OF_INPUT                              DEFAULT_NFA_FONT_SIZE_TERMINAL
#define DEFAULT_NFA_FONT_SIZE_EPSILON                                   DEFAULT_NFA_FONT_SIZE_TERMINAL
#define DEFAULT_NFA_FONT_SIZE_RULE_ARROW                                DEFAULT_NFA_FONT_SIZE_TERMINAL
#define DEFAULT_NFA_FONT_SIZE_ITEM_DOT                                  DEFAULT_NFA_FONT_SIZE_TERMINAL
#define DEFAULT_NFA_FONT_SIZE_STATE_LABEL_PREFIX                        12.0
#define DEFAULT_NFA_FONT_SIZE_STATE_LABEL_NUMBER                        15.0

/* Font colours for various diagram elements */
#define DEFAULT_NFA_FONT_COLOUR_TERMINAL                                "#7F7E00"
#define DEFAULT_NFA_FONT_COLOUR_NON_TERMINAL                            "#0003FF"
#define DEFAULT_NFA_FONT_COLOUR_END_OF_INPUT                            "#7F7E00"
#define DEFAULT_NFA_FONT_COLOUR_EPSILON                                 "#0003FF"
#define DEFAULT_NFA_FONT_COLOUR_RULE_ARROW                              "#0003FF"
#define DEFAULT_NFA_FONT_COLOUR_ITEM_DOT                                "#FF8200"
#define DEFAULT_NFA_FONT_COLOUR_STATE_LABEL_PREFIX                      "#999999"
#define DEFAULT_NFA_FONT_COLOUR_STATE_LABEL_NUMBER                      "#999999"


/*----------------------------------------------------------*/
/* Default values for DFA (deterministic finite automaton). */
/* Can be overidden by command line options.                */
/*----------------------------------------------------------*/

/* How special symbols are to be printed: epsilon, the derivation arrow and the item dot */
#define DEFAULT_DFA_RULE_ARROW_STRING                                   DEFAULT_NFA_RULE_ARROW_STRING
#define DEFAULT_DFA_ITEM_DOT_STRING                                  	DEFAULT_NFA_ITEM_DOT_STRING
#define DEFAULT_DFA_EPSILON_STRING                                      DEFAULT_NFA_EPSILON_STRING
#define DEFAULT_DFA_END_OF_INPUT_STRING                                 DEFAULT_NFA_END_OF_INPUT_STRING

/* How state IDs are to be printed, e.g. "e2" and "e123"; "State 002" and "State 123"; etc */
#define DEFAULT_DFA_STATE_LABEL_PREFIX                                  DEFAULT_NFA_STATE_LABEL_PREFIX
#define DEFAULT_DFA_STATE_LABEL_NUMBER_FORMAT                           DEFAULT_NFA_STATE_LABEL_NUMBER_FORMAT

/* Graph attributes */
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_SIZE                                DEFAULT_NFA_GRAPH_ATTRIBUTE_SIZE
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_RATIO                               DEFAULT_NFA_GRAPH_ATTRIBUTE_RATIO
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_MARGIN                              DEFAULT_NFA_GRAPH_ATTRIBUTE_MARGIN
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_ORDERING                            DEFAULT_NFA_GRAPH_ATTRIBUTE_ORDERING
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_ROTATE                              DEFAULT_NFA_GRAPH_ATTRIBUTE_ROTATE
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_COLOR                               DEFAULT_NFA_GRAPH_ATTRIBUTE_COLOR
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_BGCOLOR                             DEFAULT_NFA_GRAPH_ATTRIBUTE_BGCOLOR
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_SPLINES                             DEFAULT_NFA_GRAPH_ATTRIBUTE_SPLINES
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_NODESEP                             DEFAULT_NFA_GRAPH_ATTRIBUTE_NODESEP
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_RANKSEP                             DEFAULT_NFA_GRAPH_ATTRIBUTE_RANKSEP
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_RANKDIR                             DEFAULT_NFA_GRAPH_ATTRIBUTE_RANKDIR
#define DEFAULT_DFA_GRAPH_ATTRIBUTE_RANK                                DEFAULT_NFA_GRAPH_ATTRIBUTE_RANK

/* [NODES] Shift states */
#define DEFAULT_DFA_SHIFT_STATE_HEIGHT                                  DEFAULT_NFA_SHIFT_STATE_HEIGHT
#define DEFAULT_DFA_SHIFT_STATE_WIDTH                                   DEFAULT_NFA_SHIFT_STATE_WIDTH
#define DEFAULT_DFA_SHIFT_STATE_FIXEDSIZE                               DEFAULT_NFA_SHIFT_STATE_FIXEDSIZE
#define DEFAULT_DFA_SHIFT_STATE_SHAPE                                   "rect" 
#define DEFAULT_DFA_SHIFT_STATE_COLOR                                   "#FF8C00" /* "darkorange"  */
#define DEFAULT_DFA_SHIFT_STATE_FILLCOLOR                               "#FFFFE0" /* "lightyellow" */
#define DEFAULT_DFA_SHIFT_STATE_STYLE                                   "filled,rounded"
#define DEFAULT_DFA_SHIFT_STATE_REGULAR                                 DEFAULT_NFA_SHIFT_STATE_REGULAR
#define DEFAULT_DFA_SHIFT_STATE_PERIPHERIES                             1
#define DEFAULT_DFA_SHIFT_STATE_SIDES                                   DEFAULT_NFA_SHIFT_STATE_SIDES
#define DEFAULT_DFA_SHIFT_STATE_ORIENTATION                             DEFAULT_NFA_SHIFT_STATE_ORIENTATION
#define DEFAULT_DFA_SHIFT_STATE_DISTORTION                              DEFAULT_NFA_SHIFT_STATE_DISTORTION
#define DEFAULT_DFA_SHIFT_STATE_SKEW                                    DEFAULT_NFA_SHIFT_STATE_SKEW
#define DEFAULT_DFA_SHIFT_STATE_PENWIDTH                                DEFAULT_NFA_SHIFT_STATE_PENWIDTH
#define DEFAULT_DFA_SHIFT_STATE_MARGIN                              	0.0

/* [NODES] Conflict states */
#define DEFAULT_DFA_CONFLICT_STATE_HEIGHT                               DEFAULT_DFA_SHIFT_STATE_HEIGHT
#define DEFAULT_DFA_CONFLICT_STATE_WIDTH                                DEFAULT_DFA_SHIFT_STATE_WIDTH
#define DEFAULT_DFA_CONFLICT_STATE_FIXEDSIZE                            DEFAULT_DFA_SHIFT_STATE_FIXEDSIZE
#define DEFAULT_DFA_CONFLICT_STATE_SHAPE                                DEFAULT_DFA_SHIFT_STATE_SHAPE
#define DEFAULT_DFA_CONFLICT_STATE_COLOR                                "#DC143C" /* "crimson"  */
#define DEFAULT_DFA_CONFLICT_STATE_FILLCOLOR                            "#FFE4B5" /* "moccasin" */
#define DEFAULT_DFA_CONFLICT_STATE_STYLE                                "filled,dashed,rounded"
#define DEFAULT_DFA_CONFLICT_STATE_REGULAR                              DEFAULT_DFA_SHIFT_STATE_REGULAR
#define DEFAULT_DFA_CONFLICT_STATE_PERIPHERIES                          1
#define DEFAULT_DFA_CONFLICT_STATE_SIDES                                DEFAULT_DFA_SHIFT_STATE_SIDES
#define DEFAULT_DFA_CONFLICT_STATE_ORIENTATION                          DEFAULT_DFA_SHIFT_STATE_ORIENTATION
#define DEFAULT_DFA_CONFLICT_STATE_DISTORTION                           DEFAULT_DFA_SHIFT_STATE_DISTORTION
#define DEFAULT_DFA_CONFLICT_STATE_SKEW                                 DEFAULT_DFA_SHIFT_STATE_SKEW
#define DEFAULT_DFA_CONFLICT_STATE_PENWIDTH                             DEFAULT_DFA_SHIFT_STATE_PENWIDTH
#define DEFAULT_DFA_CONFLICT_STATE_MARGIN                               DEFAULT_DFA_SHIFT_STATE_MARGIN
  
/* [NODES] Reduction states */
#define DEFAULT_DFA_REDUCE_STATE_HEIGHT                                 DEFAULT_DFA_SHIFT_STATE_HEIGHT
#define DEFAULT_DFA_REDUCE_STATE_WIDTH                                  DEFAULT_DFA_SHIFT_STATE_WIDTH
#define DEFAULT_DFA_REDUCE_STATE_FIXEDSIZE                              DEFAULT_DFA_SHIFT_STATE_FIXEDSIZE
#define DEFAULT_DFA_REDUCE_STATE_SHAPE                                  DEFAULT_DFA_SHIFT_STATE_SHAPE
#define DEFAULT_DFA_REDUCE_STATE_COLOR                                  DEFAULT_DFA_SHIFT_STATE_COLOR
#define DEFAULT_DFA_REDUCE_STATE_FILLCOLOR                              DEFAULT_DFA_SHIFT_STATE_FILLCOLOR
#define DEFAULT_DFA_REDUCE_STATE_STYLE                                  DEFAULT_DFA_SHIFT_STATE_STYLE
#define DEFAULT_DFA_REDUCE_STATE_REGULAR                                DEFAULT_DFA_SHIFT_STATE_REGULAR
#define DEFAULT_DFA_REDUCE_STATE_PERIPHERIES                            2
#define DEFAULT_DFA_REDUCE_STATE_SIDES                                  DEFAULT_DFA_SHIFT_STATE_SIDES
#define DEFAULT_DFA_REDUCE_STATE_ORIENTATION                            DEFAULT_DFA_SHIFT_STATE_ORIENTATION
#define DEFAULT_DFA_REDUCE_STATE_DISTORTION                             DEFAULT_DFA_SHIFT_STATE_DISTORTION
#define DEFAULT_DFA_REDUCE_STATE_SKEW                                   DEFAULT_DFA_SHIFT_STATE_SKEW
#define DEFAULT_DFA_REDUCE_STATE_PENWIDTH                               DEFAULT_DFA_SHIFT_STATE_PENWIDTH
#define DEFAULT_DFA_REDUCE_STATE_MARGIN                            		DEFAULT_DFA_SHIFT_STATE_MARGIN

/* [EDGE] Arrow indicating state transition */
#define DEFAULT_DFA_TRANSITION_ARROW_WEIGHT                             DEFAULT_NFA_TRANSITION_ARROW_WEIGHT
#define DEFAULT_DFA_TRANSITION_ARROW_STYLE                              DEFAULT_NFA_TRANSITION_ARROW_STYLE
#define DEFAULT_DFA_TRANSITION_ARROW_COLOR                              DEFAULT_DFA_SHIFT_STATE_COLOR
#define DEFAULT_DFA_TRANSITION_ARROW_DIR                                DEFAULT_NFA_TRANSITION_ARROW_DIR
#define DEFAULT_DFA_TRANSITION_ARROW_TAILCLIP                           DEFAULT_NFA_TRANSITION_ARROW_TAILCLIP
#define DEFAULT_DFA_TRANSITION_ARROW_HEADCLIP                           DEFAULT_NFA_TRANSITION_ARROW_HEADCLIP
#define DEFAULT_DFA_TRANSITION_ARROW_ARROWHEAD                          DEFAULT_NFA_TRANSITION_ARROW_ARROWHEAD
#define DEFAULT_DFA_TRANSITION_ARROW_ARROWTAIL                          DEFAULT_NFA_TRANSITION_ARROW_ARROWTAIL
#define DEFAULT_DFA_TRANSITION_ARROW_ARROWSIZE                          DEFAULT_NFA_TRANSITION_ARROW_ARROWSIZE
#define DEFAULT_DFA_TRANSITION_ARROW_LABELDISTANCE                      DEFAULT_NFA_TRANSITION_ARROW_LABELDISTANCE
#define DEFAULT_DFA_TRANSITION_ARROW_DECORATE                           DEFAULT_NFA_TRANSITION_ARROW_DECORATE
#define DEFAULT_DFA_TRANSITION_ARROW_CONSTRAINT                         DEFAULT_NFA_TRANSITION_ARROW_CONSTRAINT
#define DEFAULT_DFA_TRANSITION_ARROW_MINLEN                             DEFAULT_NFA_TRANSITION_ARROW_MINLEN
#define DEFAULT_DFA_TRANSITION_ARROW_PENWIDTH                           DEFAULT_NFA_TRANSITION_ARROW_PENWIDTH

/* Fonts (type faces) for various diagram elements */
#define DEFAULT_DFA_FONT_NAME_TERMINAL                                  DEFAULT_NFA_FONT_NAME_TERMINAL
#define DEFAULT_DFA_FONT_NAME_NON_TERMINAL                              DEFAULT_NFA_FONT_NAME_NON_TERMINAL
#define DEFAULT_DFA_FONT_NAME_END_OF_INPUT                              DEFAULT_NFA_FONT_NAME_END_OF_INPUT
#define DEFAULT_DFA_FONT_NAME_EPSILON                                   DEFAULT_NFA_FONT_NAME_EPSILON
#define DEFAULT_DFA_FONT_NAME_RULE_ARROW                                DEFAULT_NFA_FONT_NAME_RULE_ARROW
#define DEFAULT_DFA_FONT_NAME_ITEM_DOT                                  DEFAULT_NFA_FONT_NAME_ITEM_DOT
#define DEFAULT_DFA_FONT_NAME_STATE_LABEL_PREFIX                        DEFAULT_NFA_FONT_NAME_STATE_LABEL_PREFIX
#define DEFAULT_DFA_FONT_NAME_STATE_LABEL_NUMBER                        DEFAULT_NFA_FONT_NAME_STATE_LABEL_NUMBER

/* Font sizes for various diagram elements */
#define DEFAULT_DFA_FONT_SIZE_TERMINAL                                  DEFAULT_NFA_FONT_SIZE_TERMINAL
#define DEFAULT_DFA_FONT_SIZE_NON_TERMINAL                              DEFAULT_NFA_FONT_SIZE_NON_TERMINAL
#define DEFAULT_DFA_FONT_SIZE_END_OF_INPUT                              DEFAULT_NFA_FONT_SIZE_END_OF_INPUT
#define DEFAULT_DFA_FONT_SIZE_EPSILON                                   DEFAULT_NFA_FONT_SIZE_EPSILON
#define DEFAULT_DFA_FONT_SIZE_RULE_ARROW                                DEFAULT_NFA_FONT_SIZE_RULE_ARROW
#define DEFAULT_DFA_FONT_SIZE_ITEM_DOT                                	DEFAULT_NFA_FONT_SIZE_ITEM_DOT
#define DEFAULT_DFA_FONT_SIZE_STATE_LABEL_PREFIX                        DEFAULT_NFA_FONT_SIZE_STATE_LABEL_PREFIX
#define DEFAULT_DFA_FONT_SIZE_STATE_LABEL_NUMBER                        DEFAULT_NFA_FONT_SIZE_STATE_LABEL_NUMBER

/* Font colours for various diagram elements */
#define DEFAULT_DFA_FONT_COLOUR_TERMINAL                                "#000000"  /* black     */
#define DEFAULT_DFA_FONT_COLOUR_NON_TERMINAL                            "#DC143C"  /* crimson   */
#define DEFAULT_DFA_FONT_COLOUR_END_OF_INPUT                            DEFAULT_DFA_FONT_COLOUR_TERMINAL
#define DEFAULT_DFA_FONT_COLOUR_EPSILON                                 "#4169E1"  /* royalblue */
#define DEFAULT_DFA_FONT_COLOUR_RULE_ARROW                              "#000000"  /* black     */
#define DEFAULT_DFA_FONT_COLOUR_ITEM_DOT                                "#4169E1"  /* royalblue */
#define DEFAULT_DFA_FONT_COLOUR_STATE_LABEL_PREFIX                      "#696969"  /* dimgray   */
#define DEFAULT_DFA_FONT_COLOUR_STATE_LABEL_NUMBER                      DEFAULT_DFA_FONT_COLOUR_STATE_LABEL_PREFIX


/*
 *---------------------------------------------------------------------
 *   Procedures defined in "diagrams.c"
 *---------------------------------------------------------------------
 */

extern void initialize_svg_attributes     (void);
extern void initialize_nfa_svg_attributes (void);
extern void initialize_dfa_svg_attributes (void);
extern void free_svg_diagrams_data        (void);

extern void print_nfa_txt (int argc, char *argv[]);
extern void print_nfa_dot (void);
extern int  print_nfa_svg (void);
extern int  print_nfa_svg2(void);

extern void print_dfa_txt (int argc, char *argv[]);
extern void print_dfa_dot (void);
extern int  print_dfa_svg (void);
extern int  print_dfa_svg2(void);

extern void print_nfa_lda (void);
extern void print_nfa_lda2 (void);
extern void print_dfa_lda (void);
extern void print_dfa_lda2 (void);
extern int  compile_lda   (const char * ldaFileName, const char * svgFileName);

#endif /* ifndef _DIAGRAMS_DOT_H_ */
