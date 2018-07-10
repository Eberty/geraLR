/*
*-----------------------------------------------------------------------
*
*   File         : diagrams.c
*   Created      : 2009-12-02
*   Last Modified: 2018-05-21
*
*   NFA and DFA diagram generation in plain text and SVG formats.
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
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <gvc.h>       /* Graphviz  */
#include <libconfig.h> /* Libconfig */

#include "datastructs.h"
#include "error.h"
#include "mystrings.h"
#include "mytime.h"
#include "report.h"
#include "grammar.h"
#include "diagrams.h"
#include "geraLR.h"

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

void initialize_svg_attributes     (void);
void initialize_nfa_svg_attributes (void);
void initialize_dfa_svg_attributes (void);
void free_svg_diagrams_data        (void);

void print_nfa_txt (int argc, char *argv[]);
void print_nfa_dot (void);
int  print_nfa_svg (void);
int  print_nfa_svg2(void);

void print_dfa_txt (int argc, char *argv[]);
void print_dfa_dot (void);
int  print_dfa_svg (void);
int  print_dfa_svg2(void);

void print_nfa_lda  (void);
void print_nfa_lda2 (void);
void print_dfa_lda  (void);
void print_dfa_lda2 (void);
int  compile_lda    (const char * ldaFileName, const char * svgFileName);

/* To prevent "implicit declaration" warnings */
int snprintf (char *str, size_t size, const char *format, ...);

/*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION (not visible from other modules)
*
*---------------------------------------------------------------------
*/

/* Pointer to config file */
config_t configFilePt;

/* Array position indexed by t_FSA_type: t_NFA = 0, t_DFA = 1 */

static t_fsa_svg_attributes FSA[2] = {  
  {
    .metasymbol.arrow_string = DEFAULT_NFA_RULE_ARROW_STRING, 
    .metasymbol.dot_string = DEFAULT_NFA_ITEM_DOT_STRING, 
    .metasymbol.end_of_input_string = DEFAULT_NFA_END_OF_INPUT_STRING,
    .metasymbol.epsilon_string = DEFAULT_NFA_EPSILON_STRING, 
    .stateLabel.prefix = DEFAULT_NFA_STATE_LABEL_PREFIX,
    .graphAttribute.size = DEFAULT_NFA_GRAPH_ATTRIBUTE_SIZE,
    .graphAttribute.ratio = DEFAULT_NFA_GRAPH_ATTRIBUTE_RATIO,
    .graphAttribute.margin = DEFAULT_NFA_GRAPH_ATTRIBUTE_MARGIN,
    .graphAttribute.ordering = DEFAULT_NFA_GRAPH_ATTRIBUTE_ORDERING,
    .graphAttribute.rotate = DEFAULT_NFA_GRAPH_ATTRIBUTE_ROTATE,
    .graphAttribute.color = DEFAULT_NFA_GRAPH_ATTRIBUTE_COLOR,
    .graphAttribute.bgcolor = DEFAULT_NFA_GRAPH_ATTRIBUTE_BGCOLOR,
    .graphAttribute.splines = DEFAULT_NFA_GRAPH_ATTRIBUTE_SPLINES,
    .graphAttribute.nodesep = DEFAULT_NFA_GRAPH_ATTRIBUTE_NODESEP,
    .graphAttribute.ranksep = DEFAULT_NFA_GRAPH_ATTRIBUTE_RANKSEP,
    .graphAttribute.rankdir = DEFAULT_NFA_GRAPH_ATTRIBUTE_RANKDIR,
    .graphAttribute.rank = DEFAULT_NFA_GRAPH_ATTRIBUTE_RANK,
    .state[0].height = DEFAULT_NFA_SHIFT_STATE_HEIGHT,
    .state[0].width = DEFAULT_NFA_SHIFT_STATE_WIDTH,
    .state[0].fixedsize = DEFAULT_NFA_SHIFT_STATE_FIXEDSIZE,
    .state[0].shape = DEFAULT_NFA_SHIFT_STATE_SHAPE,
    .state[0].color = DEFAULT_NFA_SHIFT_STATE_COLOR,
    .state[0].fillcolor = DEFAULT_NFA_SHIFT_STATE_FILLCOLOR,
    .state[0].style = DEFAULT_NFA_SHIFT_STATE_STYLE,
    .state[0].regular = DEFAULT_NFA_SHIFT_STATE_REGULAR,
    .state[0].peripheries = DEFAULT_NFA_SHIFT_STATE_PERIPHERIES,
    .state[0].sides = DEFAULT_NFA_SHIFT_STATE_SIDES,
    .state[0].orientation = DEFAULT_NFA_SHIFT_STATE_ORIENTATION,
    .state[0].distortion = DEFAULT_NFA_SHIFT_STATE_DISTORTION,
    .state[0].skew = DEFAULT_NFA_SHIFT_STATE_SKEW,
    .state[0].penwidth = DEFAULT_NFA_SHIFT_STATE_PENWIDTH,
    .state[0].margin = DEFAULT_NFA_SHIFT_STATE_MARGIN,
    .state[1].height = DEFAULT_NFA_EPSILON_STATE_HEIGHT,
    .state[1].width = DEFAULT_NFA_EPSILON_STATE_WIDTH,
    .state[1].fixedsize = DEFAULT_NFA_EPSILON_STATE_FIXEDSIZE,
    .state[1].shape = DEFAULT_NFA_EPSILON_STATE_SHAPE,
    .state[1].color = DEFAULT_NFA_EPSILON_STATE_COLOR,
    .state[1].fillcolor = DEFAULT_NFA_EPSILON_STATE_FILLCOLOR,
    .state[1].style = DEFAULT_NFA_EPSILON_STATE_STYLE,
    .state[1].regular = DEFAULT_NFA_EPSILON_STATE_REGULAR,
    .state[1].peripheries = DEFAULT_NFA_EPSILON_STATE_PERIPHERIES,
    .state[1].sides = DEFAULT_NFA_EPSILON_STATE_SIDES,
    .state[1].orientation = DEFAULT_NFA_EPSILON_STATE_ORIENTATION,
    .state[1].distortion = DEFAULT_NFA_EPSILON_STATE_DISTORTION,
    .state[1].skew = DEFAULT_NFA_EPSILON_STATE_SKEW,
    .state[1].penwidth = DEFAULT_NFA_EPSILON_STATE_PENWIDTH,
    .state[1].margin = DEFAULT_NFA_EPSILON_STATE_MARGIN,
    .state[2].height = DEFAULT_NFA_REDUCE_STATE_HEIGHT,
    .state[2].width = DEFAULT_NFA_REDUCE_STATE_WIDTH,
    .state[2].fixedsize = DEFAULT_NFA_REDUCE_STATE_FIXEDSIZE,
    .state[2].shape = DEFAULT_NFA_REDUCE_STATE_SHAPE,
    .state[2].color = DEFAULT_NFA_REDUCE_STATE_COLOR,
    .state[2].fillcolor = DEFAULT_NFA_REDUCE_STATE_FILLCOLOR,
    .state[2].style = DEFAULT_NFA_REDUCE_STATE_STYLE,
    .state[2].regular = DEFAULT_NFA_REDUCE_STATE_REGULAR,
    .state[2].peripheries = DEFAULT_NFA_REDUCE_STATE_PERIPHERIES,
    .state[2].sides = DEFAULT_NFA_REDUCE_STATE_SIDES,
    .state[2].orientation = DEFAULT_NFA_REDUCE_STATE_ORIENTATION,
    .state[2].distortion = DEFAULT_NFA_REDUCE_STATE_DISTORTION,
    .state[2].skew = DEFAULT_NFA_REDUCE_STATE_SKEW,
    .state[2].penwidth = DEFAULT_NFA_REDUCE_STATE_PENWIDTH,
    .state[2].margin = DEFAULT_NFA_REDUCE_STATE_MARGIN,
    .transitionArrow.weight = DEFAULT_NFA_TRANSITION_ARROW_WEIGHT,
    .transitionArrow.style = DEFAULT_NFA_TRANSITION_ARROW_STYLE,
    .transitionArrow.color = DEFAULT_NFA_TRANSITION_ARROW_COLOR,
    .transitionArrow.dir = DEFAULT_NFA_TRANSITION_ARROW_DIR,
    .transitionArrow.tailclip = DEFAULT_NFA_TRANSITION_ARROW_TAILCLIP,
    .transitionArrow.headclip = DEFAULT_NFA_TRANSITION_ARROW_HEADCLIP,
    .transitionArrow.arrowhead = DEFAULT_NFA_TRANSITION_ARROW_ARROWHEAD,
    .transitionArrow.arrowtail = DEFAULT_NFA_TRANSITION_ARROW_ARROWTAIL,
    .transitionArrow.arrowsize = DEFAULT_NFA_TRANSITION_ARROW_ARROWSIZE,
    .transitionArrow.labeldistance = DEFAULT_NFA_TRANSITION_ARROW_LABELDISTANCE,
    .transitionArrow.decorate = DEFAULT_NFA_TRANSITION_ARROW_DECORATE,
    .transitionArrow.constraint = DEFAULT_NFA_TRANSITION_ARROW_CONSTRAINT,
    .transitionArrow.minlen = DEFAULT_NFA_TRANSITION_ARROW_MINLEN,
    .transitionArrow.penwidth = DEFAULT_NFA_TRANSITION_ARROW_PENWIDTH,
    .font.name.terminal = DEFAULT_NFA_FONT_NAME_TERMINAL,
    .font.name.non_terminal = DEFAULT_NFA_FONT_NAME_NON_TERMINAL,
    .font.name.end_of_input = DEFAULT_NFA_FONT_NAME_END_OF_INPUT,
    .font.name.epsilon = DEFAULT_NFA_FONT_NAME_EPSILON,
    .font.name.rule_arrow = DEFAULT_NFA_FONT_NAME_RULE_ARROW,
    .font.name.item_dot = DEFAULT_NFA_FONT_NAME_ITEM_DOT,
    .font.name.state_label_prefix = DEFAULT_NFA_FONT_NAME_STATE_LABEL_PREFIX,
    .font.name.state_label_number = DEFAULT_NFA_FONT_NAME_STATE_LABEL_NUMBER,
    .font.size.non_terminal = DEFAULT_NFA_FONT_SIZE_TERMINAL,
    .font.size.terminal = DEFAULT_NFA_FONT_SIZE_NON_TERMINAL,
    .font.size.end_of_input = DEFAULT_NFA_FONT_SIZE_END_OF_INPUT,
    .font.size.epsilon = DEFAULT_NFA_FONT_SIZE_EPSILON,
    .font.size.rule_arrow = DEFAULT_NFA_FONT_SIZE_RULE_ARROW,
    .font.size.item_dot = DEFAULT_NFA_FONT_SIZE_ITEM_DOT,
    .font.size.state_label_prefix = DEFAULT_NFA_FONT_SIZE_STATE_LABEL_PREFIX,
    .font.size.state_label_number = DEFAULT_NFA_FONT_SIZE_STATE_LABEL_NUMBER,
    .font.colour.terminal = DEFAULT_NFA_FONT_COLOUR_TERMINAL,
    .font.colour.non_terminal = DEFAULT_NFA_FONT_COLOUR_NON_TERMINAL,
    .font.colour.end_of_input = DEFAULT_NFA_FONT_COLOUR_END_OF_INPUT,
    .font.colour.epsilon = DEFAULT_NFA_FONT_COLOUR_EPSILON,
    .font.colour.rule_arrow = DEFAULT_NFA_FONT_COLOUR_RULE_ARROW,
    .font.colour.item_dot = DEFAULT_NFA_FONT_COLOUR_ITEM_DOT,
    .font.colour.state_label_prefix = DEFAULT_NFA_FONT_COLOUR_STATE_LABEL_PREFIX,
    .font.colour.state_label_number = DEFAULT_NFA_FONT_COLOUR_STATE_LABEL_NUMBER
  },
  
  {
    .metasymbol.arrow_string = DEFAULT_DFA_RULE_ARROW_STRING, 
    .metasymbol.dot_string = DEFAULT_DFA_ITEM_DOT_STRING, 
    .metasymbol.end_of_input_string = DEFAULT_DFA_END_OF_INPUT_STRING, 
    .metasymbol.epsilon_string = DEFAULT_DFA_EPSILON_STRING, 
    .stateLabel.prefix = DEFAULT_DFA_STATE_LABEL_PREFIX,
    .graphAttribute.size = DEFAULT_DFA_GRAPH_ATTRIBUTE_SIZE,
    .graphAttribute.ratio = DEFAULT_DFA_GRAPH_ATTRIBUTE_RATIO,
    .graphAttribute.margin = DEFAULT_DFA_GRAPH_ATTRIBUTE_MARGIN,
    .graphAttribute.ordering = DEFAULT_DFA_GRAPH_ATTRIBUTE_ORDERING,
    .graphAttribute.rotate = DEFAULT_DFA_GRAPH_ATTRIBUTE_ROTATE,
    .graphAttribute.color = DEFAULT_DFA_GRAPH_ATTRIBUTE_COLOR,
    .graphAttribute.bgcolor = DEFAULT_DFA_GRAPH_ATTRIBUTE_BGCOLOR,
    .graphAttribute.splines = DEFAULT_DFA_GRAPH_ATTRIBUTE_SPLINES,
    .graphAttribute.nodesep = DEFAULT_DFA_GRAPH_ATTRIBUTE_NODESEP,
    .graphAttribute.ranksep = DEFAULT_DFA_GRAPH_ATTRIBUTE_RANKSEP,
    .graphAttribute.rankdir = DEFAULT_DFA_GRAPH_ATTRIBUTE_RANKDIR,
    .graphAttribute.rank = DEFAULT_DFA_GRAPH_ATTRIBUTE_RANK,
    .state[0].height = DEFAULT_DFA_SHIFT_STATE_HEIGHT,
    .state[0].width = DEFAULT_DFA_SHIFT_STATE_WIDTH,
    .state[0].fixedsize = DEFAULT_DFA_SHIFT_STATE_FIXEDSIZE,
    .state[0].shape = DEFAULT_DFA_SHIFT_STATE_SHAPE,
    .state[0].color = DEFAULT_DFA_SHIFT_STATE_COLOR,
    .state[0].fillcolor = DEFAULT_DFA_SHIFT_STATE_FILLCOLOR,
    .state[0].style = DEFAULT_DFA_SHIFT_STATE_STYLE,
    .state[0].regular = DEFAULT_DFA_SHIFT_STATE_REGULAR,
    .state[0].peripheries = DEFAULT_DFA_SHIFT_STATE_PERIPHERIES,
    .state[0].sides = DEFAULT_DFA_SHIFT_STATE_SIDES,
    .state[0].orientation = DEFAULT_DFA_SHIFT_STATE_ORIENTATION,
    .state[0].distortion = DEFAULT_DFA_SHIFT_STATE_DISTORTION,
    .state[0].skew = DEFAULT_DFA_SHIFT_STATE_SKEW,
    .state[0].penwidth = DEFAULT_DFA_SHIFT_STATE_PENWIDTH,
    .state[0].margin = DEFAULT_DFA_SHIFT_STATE_MARGIN,
    .state[1].height = DEFAULT_DFA_CONFLICT_STATE_HEIGHT,
    .state[1].width = DEFAULT_DFA_CONFLICT_STATE_WIDTH,
    .state[1].fixedsize = DEFAULT_DFA_CONFLICT_STATE_FIXEDSIZE,
    .state[1].shape = DEFAULT_DFA_CONFLICT_STATE_SHAPE,
    .state[1].color = DEFAULT_DFA_CONFLICT_STATE_COLOR,
    .state[1].fillcolor = DEFAULT_DFA_CONFLICT_STATE_FILLCOLOR,
    .state[1].style = DEFAULT_DFA_CONFLICT_STATE_STYLE,
    .state[1].regular = DEFAULT_DFA_CONFLICT_STATE_REGULAR,
    .state[1].peripheries = DEFAULT_DFA_CONFLICT_STATE_PERIPHERIES,
    .state[1].sides = DEFAULT_DFA_CONFLICT_STATE_SIDES,
    .state[1].orientation = DEFAULT_DFA_CONFLICT_STATE_ORIENTATION,
    .state[1].distortion = DEFAULT_DFA_CONFLICT_STATE_DISTORTION,
    .state[1].skew = DEFAULT_DFA_CONFLICT_STATE_SKEW,
    .state[1].penwidth = DEFAULT_DFA_CONFLICT_STATE_PENWIDTH,
    .state[1].margin = DEFAULT_DFA_CONFLICT_STATE_MARGIN,
    .state[2].height = DEFAULT_DFA_REDUCE_STATE_HEIGHT,
    .state[2].width = DEFAULT_DFA_REDUCE_STATE_WIDTH,
    .state[2].fixedsize = DEFAULT_DFA_REDUCE_STATE_FIXEDSIZE,
    .state[2].shape = DEFAULT_DFA_REDUCE_STATE_SHAPE,
    .state[2].color = DEFAULT_DFA_REDUCE_STATE_COLOR,
    .state[2].fillcolor = DEFAULT_DFA_REDUCE_STATE_FILLCOLOR,
    .state[2].style = DEFAULT_DFA_REDUCE_STATE_STYLE,
    .state[2].regular = DEFAULT_DFA_REDUCE_STATE_REGULAR,
    .state[2].peripheries = DEFAULT_DFA_REDUCE_STATE_PERIPHERIES,
    .state[2].sides = DEFAULT_DFA_REDUCE_STATE_SIDES,
    .state[2].orientation = DEFAULT_DFA_REDUCE_STATE_ORIENTATION,
    .state[2].distortion = DEFAULT_DFA_REDUCE_STATE_DISTORTION,
    .state[2].skew = DEFAULT_DFA_REDUCE_STATE_SKEW,
    .state[2].penwidth = DEFAULT_DFA_REDUCE_STATE_PENWIDTH,
    .state[2].margin = DEFAULT_DFA_REDUCE_STATE_MARGIN,
    .transitionArrow.weight = DEFAULT_DFA_TRANSITION_ARROW_WEIGHT,
    .transitionArrow.style = DEFAULT_DFA_TRANSITION_ARROW_STYLE,
    .transitionArrow.color = DEFAULT_DFA_TRANSITION_ARROW_COLOR,
    .transitionArrow.dir = DEFAULT_DFA_TRANSITION_ARROW_DIR,
    .transitionArrow.tailclip = DEFAULT_DFA_TRANSITION_ARROW_TAILCLIP,
    .transitionArrow.headclip = DEFAULT_DFA_TRANSITION_ARROW_HEADCLIP,
    .transitionArrow.arrowhead = DEFAULT_DFA_TRANSITION_ARROW_ARROWHEAD,
    .transitionArrow.arrowtail = DEFAULT_DFA_TRANSITION_ARROW_ARROWTAIL,
    .transitionArrow.arrowsize = DEFAULT_DFA_TRANSITION_ARROW_ARROWSIZE,
    .transitionArrow.labeldistance = DEFAULT_DFA_TRANSITION_ARROW_LABELDISTANCE,
    .transitionArrow.decorate = DEFAULT_DFA_TRANSITION_ARROW_DECORATE,
    .transitionArrow.constraint = DEFAULT_DFA_TRANSITION_ARROW_CONSTRAINT,
    .transitionArrow.minlen = DEFAULT_DFA_TRANSITION_ARROW_MINLEN,
    .transitionArrow.penwidth = DEFAULT_DFA_TRANSITION_ARROW_PENWIDTH,
    .font.name.terminal = DEFAULT_DFA_FONT_NAME_TERMINAL,
    .font.name.non_terminal = DEFAULT_DFA_FONT_NAME_NON_TERMINAL,
    .font.name.end_of_input = DEFAULT_DFA_FONT_NAME_END_OF_INPUT,
    .font.name.epsilon = DEFAULT_DFA_FONT_NAME_EPSILON,
    .font.name.rule_arrow = DEFAULT_DFA_FONT_NAME_RULE_ARROW,
    .font.name.item_dot = DEFAULT_DFA_FONT_NAME_ITEM_DOT,
    .font.name.state_label_prefix = DEFAULT_DFA_FONT_NAME_STATE_LABEL_PREFIX,
    .font.name.state_label_number = DEFAULT_DFA_FONT_NAME_STATE_LABEL_NUMBER,
    .font.size.non_terminal = DEFAULT_DFA_FONT_SIZE_TERMINAL,
    .font.size.terminal = DEFAULT_DFA_FONT_SIZE_NON_TERMINAL,
    .font.size.end_of_input = DEFAULT_DFA_FONT_SIZE_END_OF_INPUT,
    .font.size.epsilon = DEFAULT_DFA_FONT_SIZE_EPSILON,
    .font.size.rule_arrow = DEFAULT_DFA_FONT_SIZE_RULE_ARROW,
    .font.size.item_dot = DEFAULT_DFA_FONT_SIZE_ITEM_DOT,
    .font.size.state_label_prefix = DEFAULT_DFA_FONT_SIZE_STATE_LABEL_PREFIX,
    .font.size.state_label_number = DEFAULT_DFA_FONT_SIZE_STATE_LABEL_NUMBER,
    .font.colour.terminal = DEFAULT_DFA_FONT_COLOUR_TERMINAL,
    .font.colour.non_terminal = DEFAULT_DFA_FONT_COLOUR_NON_TERMINAL,
    .font.colour.end_of_input = DEFAULT_DFA_FONT_COLOUR_END_OF_INPUT,
    .font.colour.epsilon = DEFAULT_DFA_FONT_COLOUR_EPSILON,
    .font.colour.rule_arrow = DEFAULT_DFA_FONT_COLOUR_RULE_ARROW,
    .font.colour.item_dot = DEFAULT_DFA_FONT_COLOUR_ITEM_DOT,
    .font.colour.state_label_prefix = DEFAULT_DFA_FONT_COLOUR_STATE_LABEL_PREFIX,
    .font.colour.state_label_number = DEFAULT_DFA_FONT_COLOUR_STATE_LABEL_NUMBER
  }
};

/* const char * FSA_config_attributes[] = {} */

const char * NFA_Attributes[] = {
  /* How special symbols are to be printed: epsilon, the derivation arrow and the item dot */
  "arrow",  "dot", "epsilon", "end",
  /* How state IDs are to be printed, e.g. "e2" and "e123"; "State 002" and "State 123"; etc */
  "prefix",
  /* Graph Attributes */
  "size", "ratio", "margin", "ordering", "rotate", "color", "bgcolor", "splines", "nodesep", "ranksep", "rankdir", "rank",
  /* [NODES] Shift states */
  "height", "width", "fixedsize", "shape", "color", "fillcolor", "style", "regular", "peripheries", "sides", "orientation", "distortion", "skew", "penwidth", "margin",
  /* [NODES] Epsilon states */
  "height", "width", "fixedsize", "shape", "color", "fillcolor", "style", "regular", "peripheries", "sides", "orientation", "distortion", "skew", "penwidth", "margin",
  /* [NODES] Reduction states */
  "height", "width", "fixedsize", "shape", "color", "fillcolor", "style", "regular", "peripheries", "sides", "orientation", "distortion", "skew", "penwidth", "margin",
  /* [EDGE] Arrow indicating state transition */
  "weight", "style", "color", "dir", "tailclip", "headclip", "arrowhead", "arrowtail", "arrowsize", "labeldistance", "decorate", "constraint", "minlen", "penwidth",
  /* Fonts (type faces) for various diagram elements */
  "terminal", "non_terminal", "end_of_input", "epsilon", "rule_arrow", "item_dot", "state_label_prefix", "state_label_number",
  /* Font sizes for various diagram elements */
  "terminal", "non_terminal", "end_of_input", "epsilon", "rule_arrow", "item_dot", "state_label_prefix", "state_label_number",
  /* Font colours for various diagram elements */
  "terminal", "non_terminal", "end_of_input", "epsilon", "rule_arrow", "item_dot", "state_label_prefix", "state_label_number"
};

const char * DFA_Attributes[] = {
  /* How special symbols are to be printed: epsilon, the derivation arrow and the item dot */
  "arrow",  "dot", "epsilon", "end",
  /* How state IDs are to be printed, e.g. "e2" and "e123"; "State 002" and "State 123"; etc */
  "prefix",
  /* Graph Attributes */
  "size", "ratio", "margin", "ordering", "rotate", "color", "bgcolor", "splines", "nodesep", "ranksep", "rankdir", "rank",
  /* [NODES] Shift states */
  "height", "width", "fixedsize", "shape", "color", "fillcolor", "style", "regular", "peripheries", "sides", "orientation", "distortion", "skew", "penwidth", "margin",
  /* [NODES] Conflict states */
  "height", "width", "fixedsize", "shape", "color", "fillcolor", "style", "regular", "peripheries", "sides", "orientation", "distortion", "skew", "penwidth", "margin",
  /* [NODES] Reduction states */
  "height", "width", "fixedsize", "shape", "color", "fillcolor", "style", "regular", "peripheries", "sides", "orientation", "distortion", "skew", "penwidth", "margin",
  /* [EDGE] Arrow indicating state transition */
  "weight", "style", "color", "dir", "tailclip", "headclip", "arrowhead", "arrowtail", "arrowsize", "labeldistance", "decorate", "constraint", "minlen", "penwidth",
  /* Fonts (type faces) for various diagram elements */
  "terminal", "non_terminal", "end_of_input", "epsilon", "rule_arrow", "item_dot", "state_label_prefix", "state_label_number",
  /* Font sizes for various diagram elements */
  "terminal", "non_terminal", "end_of_input", "epsilon", "rule_arrow", "item_dot", "state_label_prefix", "state_label_number",
  /* Font colours for various diagram elements */
  "terminal", "non_terminal", "end_of_input", "epsilon", "rule_arrow", "item_dot", "state_label_prefix", "state_label_number"
};

/*                       */
/* Function declarations */
/*                       */

/*
*-----------------------------------------------------------------------------
* Initialize SVG diagram attributes
*-----------------------------------------------------------------------------
*/

void initialize_svg_attributes (void) {
  config_init (&configFilePt);

  /* Read the file. If there is an error, report it and use de default values. */
  if (! config_read_file (&configFilePt, configFileName)) {
    fprintf (stderr, "%s:%d - %s\n", config_error_file (&configFilePt), config_error_line (&configFilePt), config_error_text (&configFilePt));
    config_destroy (&configFilePt);
    fprintf (stderr, "Using default values\n");
  }
  initialize_nfa_svg_attributes();
  initialize_dfa_svg_attributes();
}

void initialize_nfa_svg_attributes (void) {
  config_setting_t 
    *setting;

  /* List of all NFA atributes */
  if ((setting = config_lookup (&configFilePt, "NFA.metasymbol")) != NULL) {
    config_setting_lookup_string (setting, NFA_Attributes[nfa_rule_arrow_string], &FSA[0].metasymbol.arrow_string);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_item_dot_string], &FSA[0].metasymbol.dot_string);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_epsilon_string], &FSA[0].metasymbol.epsilon_string);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_end_of_input_string], &FSA[0].metasymbol.end_of_input_string);
  } 
  else
    fprintf (stderr, "No 'NFA.symbol' setting in configuration file.\n");
    
  if ((setting = config_lookup (&configFilePt, "NFA.stateLabel")) != NULL) {
    config_setting_lookup_string (setting, NFA_Attributes[nfa_state_label_prefix], &FSA[0].stateLabel.prefix);
  } 
  else
    fprintf (stderr, "No 'NFA.stateLabel' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "NFA.graphAttribute")) != NULL) {
    config_setting_lookup_string (setting, NFA_Attributes[nfa_graph_attribute_size], &FSA[0].graphAttribute.size);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_graph_attribute_ratio], &FSA[0].graphAttribute.ratio);
    config_setting_lookup_float (setting,NFA_Attributes[nfa_graph_attribute_margin], &FSA[0].graphAttribute.margin);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_graph_attribute_ordering], &FSA[0].graphAttribute.ordering);
    config_setting_lookup_int (setting, NFA_Attributes[nfa_graph_attribute_rotate], &FSA[0].graphAttribute.rotate);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_graph_attribute_color], &FSA[0].graphAttribute.color);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_graph_attribute_bgcolor], &FSA[0].graphAttribute.bgcolor);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_graph_attribute_splines], &FSA[0].graphAttribute.splines);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_graph_attribute_nodesep], &FSA[0].graphAttribute.nodesep);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_graph_attribute_ranksep], &FSA[0].graphAttribute.ranksep);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_graph_attribute_rankdir], &FSA[0].graphAttribute.rankdir);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_graph_attribute_rank], &FSA[0].graphAttribute.rank);
  } 
  else
    fprintf (stderr, "No 'NFA.graphAttribute' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "NFA.shiftState")) != NULL) {
    config_setting_lookup_float (setting, NFA_Attributes[nfa_shift_state_height], &FSA[0].state[0].height);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_shift_state_width], &FSA[0].state[0].width);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_shift_state_fixedsize], &FSA[0].state[0].fixedsize);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_shift_state_shape], &FSA[0].state[0].shape);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_shift_state_color], &FSA[0].state[0].color);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_shift_state_fillcolor], &FSA[0].state[0].fillcolor);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_shift_state_style], &FSA[0].state[0].style);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_shift_state_regular], &FSA[0].state[0].regular);
    config_setting_lookup_int (setting, NFA_Attributes[nfa_shift_state_peripheries], &FSA[0].state[0].peripheries);
    config_setting_lookup_int (setting, NFA_Attributes[nfa_shift_state_sides], &FSA[0].state[0].sides);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_shift_state_orientation], &FSA[0].state[0].orientation);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_shift_state_distortion], &FSA[0].state[0].distortion);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_shift_state_skew], &FSA[0].state[0].skew);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_shift_state_penwidth], &FSA[0].state[0].penwidth);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_shift_state_margin], &FSA[0].state[0].margin);
  } 
  else
    fprintf (stderr, "No 'NFA.shiftState' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "NFA.epsilonState")) != NULL) {
    config_setting_lookup_float (setting, NFA_Attributes[nfa_epsilon_state_height], &FSA[0].state[1].height);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_epsilon_state_width], &FSA[0].state[1].width);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_epsilon_state_fixedsize], &FSA[0].state[1].fixedsize);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_epsilon_state_shape], &FSA[0].state[1].shape);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_epsilon_state_color], &FSA[0].state[1].color);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_epsilon_state_fillcolor], &FSA[0].state[1].fillcolor);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_epsilon_state_style], &FSA[0].state[1].style);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_epsilon_state_regular], &FSA[0].state[1].regular);
    config_setting_lookup_int (setting, NFA_Attributes[nfa_epsilon_state_peripheries], &FSA[0].state[1].peripheries);
    config_setting_lookup_int (setting, NFA_Attributes[nfa_epsilon_state_sides], &FSA[0].state[1].sides);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_epsilon_state_orientation], &FSA[0].state[1].orientation);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_epsilon_state_distortion], &FSA[0].state[1].distortion);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_epsilon_state_skew], &FSA[0].state[1].skew);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_epsilon_state_penwidth], &FSA[0].state[1].penwidth);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_shift_state_margin], &FSA[0].state[1].margin);
  } 
  else
    fprintf (stderr, "No 'NFA.epsilonState' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "NFA.reduceState")) != NULL) {
    config_setting_lookup_float (setting, NFA_Attributes[nfa_reduce_state_height], &FSA[0].state[2].height);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_reduce_state_width], &FSA[0].state[2].width);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_reduce_state_fixedsize], &FSA[0].state[2].fixedsize);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_reduce_state_shape], &FSA[0].state[2].shape);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_reduce_state_color], &FSA[0].state[2].color);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_reduce_state_fillcolor], &FSA[0].state[2].fillcolor);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_reduce_state_style], &FSA[0].state[2].style);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_reduce_state_regular], &FSA[0].state[2].regular);
    config_setting_lookup_int (setting, NFA_Attributes[nfa_reduce_state_peripheries], &FSA[0].state[2].peripheries);
    config_setting_lookup_int (setting, NFA_Attributes[nfa_reduce_state_sides], &FSA[0].state[2].sides);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_reduce_state_orientation], &FSA[0].state[2].orientation);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_reduce_state_distortion], &FSA[0].state[2].distortion);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_reduce_state_skew], &FSA[0].state[2].skew);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_reduce_state_penwidth], &FSA[0].state[2].penwidth);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_shift_state_margin], &FSA[0].state[2].margin);
  } 
  else
    fprintf (stderr, "No 'NFA.reduceState' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "NFA.transitionArrow")) != NULL) {
    config_setting_lookup_float (setting, NFA_Attributes[nfa_transition_arrow_weight], &FSA[0].transitionArrow.weight);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_transition_arrow_style], &FSA[0].transitionArrow.style);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_transition_arrow_color], &FSA[0].transitionArrow.color);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_transition_arrow_dir], &FSA[0].transitionArrow.dir);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_transition_arrow_tailclip], &FSA[0].transitionArrow.tailclip);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_transition_arrow_headclip], &FSA[0].transitionArrow.headclip);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_transition_arrow_arrowhead], &FSA[0].transitionArrow.arrowhead);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_transition_arrow_arrowtail], &FSA[0].transitionArrow.arrowtail);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_transition_arrow_arrowsize], &FSA[0].transitionArrow.arrowsize);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_transition_arrow_labeldistance], &FSA[0].transitionArrow.labeldistance);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_transition_arrow_decorate], &FSA[0].transitionArrow.decorate);
    config_setting_lookup_bool (setting, NFA_Attributes[nfa_transition_arrow_constraint], &FSA[0].transitionArrow.constraint);
    config_setting_lookup_int (setting, NFA_Attributes[nfa_transition_arrow_minlen], &FSA[0].transitionArrow.minlen);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_transition_arrow_penwidth], &FSA[0].transitionArrow.penwidth);
  } 
  else
    fprintf (stderr, "No 'NFA.transitionArrow' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "NFA.font.name")) != NULL) {
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_name_terminal], &FSA[0].font.name.terminal);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_name_non_terminal], &FSA[0].font.name.non_terminal);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_name_end_of_input], &FSA[0].font.name.end_of_input);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_name_epsilon], &FSA[0].font.name.epsilon);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_name_rule_arrow], &FSA[0].font.name.rule_arrow);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_name_item_dot], &FSA[0].font.name.item_dot);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_name_state_label_prefix], &FSA[0].font.name.state_label_prefix);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_name_state_label_number], &FSA[0].font.name.state_label_number);
  } 
  else
    fprintf (stderr, "No 'NFA.font.name' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "NFA.font.size")) != NULL) {
    config_setting_lookup_float (setting, NFA_Attributes[nfa_font_size_terminal], &FSA[0].font.size.terminal);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_font_size_non_terminal], &FSA[0].font.size.non_terminal);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_font_size_end_of_input], &FSA[0].font.size.end_of_input);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_font_size_epsilon], &FSA[0].font.size.epsilon);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_font_size_rule_arrow], &FSA[0].font.size.rule_arrow);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_font_size_item_dot], &FSA[0].font.size.item_dot);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_font_size_state_label_prefix], &FSA[0].font.size.state_label_prefix);
    config_setting_lookup_float (setting, NFA_Attributes[nfa_font_size_state_label_number], &FSA[0].font.size.state_label_number);
  } 
  else
    fprintf (stderr, "No 'NFA.font.size' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "NFA.font.colour")) != NULL) {
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_colour_terminal], &FSA[0].font.colour.terminal);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_colour_non_terminal], &FSA[0].font.colour.non_terminal);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_colour_end_of_input], &FSA[0].font.colour.end_of_input);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_colour_epsilon], &FSA[0].font.colour.epsilon);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_colour_rule_arrow], &FSA[0].font.colour.rule_arrow);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_colour_item_dot], &FSA[0].font.colour.item_dot);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_colour_state_label_prefix], &FSA[0].font.colour.state_label_prefix);
    config_setting_lookup_string (setting, NFA_Attributes[nfa_font_colour_state_label_number], &FSA[0].font.colour.state_label_number);
  } 
  else
    fprintf (stderr, "No 'NFA.font.colour' setting in configuration file.\n");
}

void initialize_dfa_svg_attributes (void) {
  config_setting_t *setting;

  /* List of all DFA atributes to drow. */
  if ((setting = config_lookup (&configFilePt, "DFA.metasymbol")) != NULL) {
    config_setting_lookup_string (setting, DFA_Attributes[dfa_rule_arrow_string], &FSA[1].metasymbol.arrow_string);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_item_dot_string], &FSA[1].metasymbol.dot_string);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_epsilon_string], &FSA[1].metasymbol.epsilon_string);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_end_of_input_string], &FSA[1].metasymbol.end_of_input_string);
  } 
  else
    fprintf (stderr, "No 'DFA.symbol' setting in configuration file.\n");
  
  if ((setting = config_lookup (&configFilePt, "DFA.stateLabel")) != NULL) {
    config_setting_lookup_string (setting, DFA_Attributes[dfa_state_label_prefix], &FSA[1].stateLabel.prefix);
  } 
  else
    fprintf (stderr, "No 'DFA.stateLabel' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "DFA.graphAttribute")) != NULL) {
    config_setting_lookup_string (setting, DFA_Attributes[dfa_graph_attribute_size], &FSA[1].graphAttribute.size);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_graph_attribute_ratio], &FSA[1].graphAttribute.ratio);
    config_setting_lookup_float (setting,DFA_Attributes[dfa_graph_attribute_margin], &FSA[1].graphAttribute.margin);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_graph_attribute_ordering], &FSA[1].graphAttribute.ordering);
    config_setting_lookup_int (setting, DFA_Attributes[dfa_graph_attribute_rotate], &FSA[1].graphAttribute.rotate);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_graph_attribute_color], &FSA[1].graphAttribute.color);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_graph_attribute_bgcolor], &FSA[1].graphAttribute.bgcolor);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_graph_attribute_splines], &FSA[1].graphAttribute.splines);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_graph_attribute_nodesep], &FSA[1].graphAttribute.nodesep);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_graph_attribute_ranksep], &FSA[1].graphAttribute.ranksep);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_graph_attribute_rankdir], &FSA[1].graphAttribute.rankdir);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_graph_attribute_rank], &FSA[1].graphAttribute.rank);
  } 
  else
    fprintf (stderr, "No 'DFA.graphAttribute' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "DFA.shiftState")) != NULL) {
    config_setting_lookup_float (setting, DFA_Attributes[dfa_shift_state_height], &FSA[1].state[0].height);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_shift_state_width], &FSA[1].state[0].width);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_shift_state_fixedsize], &FSA[1].state[0].fixedsize);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_shift_state_shape], &FSA[1].state[0].shape);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_shift_state_color], &FSA[1].state[0].color);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_shift_state_fillcolor], &FSA[1].state[0].fillcolor);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_shift_state_style], &FSA[1].state[0].style);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_shift_state_regular], &FSA[1].state[0].regular);
    config_setting_lookup_int (setting, DFA_Attributes[dfa_shift_state_peripheries], &FSA[1].state[0].peripheries);
    config_setting_lookup_int (setting, DFA_Attributes[dfa_shift_state_sides], &FSA[1].state[0].sides);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_shift_state_orientation], &FSA[1].state[0].orientation);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_shift_state_distortion], &FSA[1].state[0].distortion);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_shift_state_skew], &FSA[1].state[0].skew);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_shift_state_penwidth], &FSA[1].state[0].penwidth);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_shift_state_margin], &FSA[1].state[0].margin);
  } 
  else
    fprintf (stderr, "No 'DFA.shiftState' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "DFA.conflictState")) != NULL) {
    config_setting_lookup_float (setting, DFA_Attributes[dfa_conflict_state_height], &FSA[1].state[1].height);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_conflict_state_width], &FSA[1].state[1].width);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_conflict_state_fixedsize], &FSA[1].state[1].fixedsize);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_conflict_state_shape], &FSA[1].state[1].shape);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_conflict_state_color], &FSA[1].state[1].color);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_conflict_state_fillcolor], &FSA[1].state[1].fillcolor);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_conflict_state_style], &FSA[1].state[1].style);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_conflict_state_regular], &FSA[1].state[1].regular);
    config_setting_lookup_int (setting, DFA_Attributes[dfa_conflict_state_peripheries], &FSA[1].state[1].peripheries);
    config_setting_lookup_int (setting, DFA_Attributes[dfa_conflict_state_sides], &FSA[1].state[1].sides);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_conflict_state_orientation], &FSA[1].state[1].orientation);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_conflict_state_distortion], &FSA[1].state[1].distortion);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_conflict_state_skew], &FSA[1].state[1].skew);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_conflict_state_penwidth], &FSA[1].state[1].penwidth);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_shift_state_margin], &FSA[1].state[1].margin);
  } 
  else
    fprintf (stderr, "No 'DFA.conflictState' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "DFA.reduceState")) != NULL) {
    config_setting_lookup_float (setting, DFA_Attributes[dfa_reduce_state_height], &FSA[1].state[2].height);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_reduce_state_width], &FSA[1].state[2].width);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_reduce_state_fixedsize], &FSA[1].state[2].fixedsize);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_reduce_state_shape], &FSA[1].state[2].shape);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_reduce_state_color], &FSA[1].state[2].color);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_reduce_state_fillcolor], &FSA[1].state[2].fillcolor);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_reduce_state_style], &FSA[1].state[2].style);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_reduce_state_regular], &FSA[1].state[2].regular);
    config_setting_lookup_int (setting, DFA_Attributes[dfa_reduce_state_peripheries], &FSA[1].state[2].peripheries);
    config_setting_lookup_int (setting, DFA_Attributes[dfa_reduce_state_sides], &FSA[1].state[2].sides);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_reduce_state_orientation], &FSA[1].state[2].orientation);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_reduce_state_distortion], &FSA[1].state[2].distortion);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_reduce_state_skew], &FSA[1].state[2].skew);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_reduce_state_penwidth], &FSA[1].state[2].penwidth);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_shift_state_margin], &FSA[1].state[2].margin);
  } 
  else
    fprintf (stderr, "No 'DFA.reduceState' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "DFA.transitionArrow")) != NULL) {
    config_setting_lookup_float (setting, DFA_Attributes[dfa_transition_arrow_weight], &FSA[1].transitionArrow.weight);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_transition_arrow_style], &FSA[1].transitionArrow.style);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_transition_arrow_color], &FSA[1].transitionArrow.color);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_transition_arrow_dir], &FSA[1].transitionArrow.dir);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_transition_arrow_tailclip], &FSA[1].transitionArrow.tailclip);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_transition_arrow_headclip], &FSA[1].transitionArrow.headclip);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_transition_arrow_arrowhead], &FSA[1].transitionArrow.arrowhead);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_transition_arrow_arrowtail], &FSA[1].transitionArrow.arrowtail);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_transition_arrow_arrowsize], &FSA[1].transitionArrow.arrowsize);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_transition_arrow_labeldistance], &FSA[1].transitionArrow.labeldistance);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_transition_arrow_decorate], &FSA[1].transitionArrow.decorate);
    config_setting_lookup_bool (setting, DFA_Attributes[dfa_transition_arrow_constraint], &FSA[1].transitionArrow.constraint);
    config_setting_lookup_int (setting, DFA_Attributes[dfa_transition_arrow_minlen], &FSA[1].transitionArrow.minlen);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_transition_arrow_penwidth], &FSA[1].transitionArrow.penwidth);
  } 
  else
    fprintf (stderr, "No 'DFA.transitionArrow' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "DFA.font.name")) != NULL) {
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_name_terminal], &FSA[1].font.name.terminal);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_name_non_terminal], &FSA[1].font.name.non_terminal);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_name_end_of_input], &FSA[1].font.name.end_of_input);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_name_epsilon], &FSA[1].font.name.epsilon);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_name_rule_arrow], &FSA[1].font.name.rule_arrow);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_name_item_dot], &FSA[1].font.name.item_dot);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_name_state_label_prefix], &FSA[1].font.name.state_label_prefix);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_name_state_label_number], &FSA[1].font.name.state_label_number);
  } 
  else
    fprintf (stderr, "No 'DFA.font.name' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "DFA.font.size")) != NULL) {
    config_setting_lookup_float (setting, DFA_Attributes[dfa_font_size_terminal], &FSA[1].font.size.terminal);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_font_size_non_terminal], &FSA[1].font.size.non_terminal);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_font_size_end_of_input], &FSA[1].font.size.end_of_input);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_font_size_epsilon], &FSA[1].font.size.epsilon);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_font_size_rule_arrow], &FSA[1].font.size.rule_arrow);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_font_size_item_dot], &FSA[1].font.size.item_dot);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_font_size_state_label_prefix], &FSA[1].font.size.state_label_prefix);
    config_setting_lookup_float (setting, DFA_Attributes[dfa_font_size_state_label_number], &FSA[1].font.size.state_label_number);
  } 
  else
    fprintf (stderr, "No 'DFA.font.size' setting in configuration file.\n");

  if ((setting = config_lookup (&configFilePt, "DFA.font.colour")) != NULL) {
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_colour_terminal], &FSA[1].font.colour.terminal);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_colour_non_terminal], &FSA[1].font.colour.non_terminal);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_colour_end_of_input], &FSA[1].font.colour.end_of_input);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_colour_epsilon], &FSA[1].font.colour.epsilon);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_colour_rule_arrow], &FSA[1].font.colour.rule_arrow);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_colour_item_dot], &FSA[1].font.colour.item_dot);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_colour_state_label_prefix], &FSA[1].font.colour.state_label_prefix);
    config_setting_lookup_string (setting, DFA_Attributes[dfa_font_colour_state_label_number], &FSA[1].font.colour.state_label_number);
  } 
  else
    fprintf (stderr, "No 'DFA.font.colour' setting in configuration file.\n");
}

/*
*-----------------------------------------------------------------------------
* Free SVG diagrams data
*-----------------------------------------------------------------------------
*/

void free_svg_diagrams_data (void) {
  config_destroy (&configFilePt);
}

/*
*-----------------------------------------------------------------------------
* Print NFA, i.e. complete set of states and LR (0) items, in plain text format
*-----------------------------------------------------------------------------
*/

void print_nfa_txt (int argc, char *argv[]) {
  unsigned int
    iLine,
    iSymbol,
    iState,
    iTransition,
    dotPosition,
    ruleSize,
    symbolNumber,
    totLines;
  t_symbolCode
    symbolCode;
  t_stateCode
    stateCode;
  t_itemCode
    itemCode;
  t_ruleNumber
    ruleNumber;
  size_t
    symbolWidth,
    columnWidth;
  char
    *headerLine1 = NULL,
    *headerLine2 = NULL,
    *detailLine  = NULL,
    *auxString   = NULL;

  /* Initialize a few things */

  headerLine1 = (char *) STRING_allocate();
  headerLine2 = (char *) STRING_allocate();
  detailLine  = (char *) STRING_allocate();
  auxString   = (char *) STRING_allocate();

  symbolWidth = maxGrammarSymbolLength;

  /* First print the header and grammar rules, then the states, items and transitions */

  if (! b_one_output_file || ! b_output_started) {
    print_output_header (nfaTextFilePt, nfaTextFileName, argc, argv);
    print_grammar_rules (nfaTextFilePt);
    b_output_started = true;
  }

  /* Print list of LR (0) states and items */

  fprintf (nfaTextFilePt, "+-------------------+\n");
  fprintf (nfaTextFilePt, "| NFA STATES: %-5d |\n", (int) totNFAstates);
  fprintf (nfaTextFilePt, "+-------------------+\n");
  REPORT_newLine (nfaTextFilePt, 1);

  for (iState = 1; iState <= totNFAstates; iState++) {
    stateCode = nfa_stateNumber2stateCode (iState);
    fprintf (nfaTextFilePt, "State %d: \n%s\n", stateCode, stateType2stateTypeString (stateCode2stateType (stateCode)));
    itemCode = nfa_stateCode2itemCode (stateCode);
    ruleNumber = itemCode2ruleNumber (itemCode);
    ruleSize = ruleNumber2ruleSize (ruleNumber);
    dotPosition = itemCode2dotPosition (itemCode);
    fprintf (nfaTextFilePt, "  %s ->", symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
    if (dotPosition == 0)
      fprintf (nfaTextFilePt, " •");
    for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
      fprintf (nfaTextFilePt, " %s", symbolCode2symbolString (rulePos2symbolCode (ruleNumber, iSymbol)));
      if (dotPosition == iSymbol)
        fprintf (nfaTextFilePt, " •");
    }
    REPORT_newLine (nfaTextFilePt, 2);
  }

  /* Print list of all states containing a reduction item */

  fprintf (nfaTextFilePt, "+-------------------------+\n");
  fprintf (nfaTextFilePt, "| REDUCTION STATES: %-5d |\n", (int) tot_NFA_reduce_states);
  if (tot_NFA_reduce_states > 0) {
    fprintf (nfaTextFilePt, "+------------+------------+\n");
    fprintf (nfaTextFilePt, "|    State   |     Rule   |\n");
    fprintf (nfaTextFilePt, "+------------+------------+\n");
    for (iState = 1; iState <= totNFAstates; iState++) {
      stateCode = nfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_NFA_reduce_state) {
        ruleNumber = nfa_stateCode2reductionRule (stateCode);
        fprintf (nfaTextFilePt, "|    %5d   |    %5d   |\n", (int) stateCode, (int) ruleNumber);
      }
    }
    fprintf (nfaTextFilePt, "+------------+------------+\n");
  }
  REPORT_newLine (nfaTextFilePt, 1);

  /* Sort transitions by origin state code */

  nfa_sortTransitions (t_transitionSortKey_origin);

  /* Now print NFA state transitions */

  (void) STRING_copy ((char **) &headerLine1, "+--------------------------");
  (void) STRING_extend ((char **) &headerLine1, '-', (size_t) symbolWidth-8);
  (void) STRING_concatenate ((char **) &headerLine1, "+");
  fprintf (nfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "| STATE TRANSITIONS: ");
  (void) STRING_set ((char **) &auxString, 0, (size_t) 5);
  snprintf (auxString, 4, "%-3d", (int) totNFAtransitions);
  (void) STRING_justify ((char **) &auxString, 7, ' ', STRING_t_justify_left);
  (void) STRING_concatenate ((char **) &headerLine1, auxString);
  (void) STRING_concatenate ((char **) &headerLine1, "|");
  fprintf (nfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine2, "+-------+-------+----------");
  (void) STRING_extend ((char **) &headerLine2, '-', (size_t) symbolWidth-8);
  (void) STRING_concatenate ((char **) &headerLine2, "+");
  fprintf (nfaTextFilePt, "%s\n", headerLine2);

  (void) STRING_copy ((char **) &headerLine1, "| From  | To    | With   ");
  (void) STRING_extend ((char **) &headerLine1, ' ', (size_t) symbolWidth-6);
  (void) STRING_concatenate ((char **) &headerLine1, "|");
  fprintf (nfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "| state | state | symbol ");
  (void) STRING_extend ((char **) &headerLine1, ' ', (size_t) symbolWidth-6);
  (void) STRING_concatenate ((char **) &headerLine1, "|");
  fprintf (nfaTextFilePt, "%s\n", headerLine1);
  fprintf (nfaTextFilePt, "%s\n", headerLine2);

  for (iTransition = 1; iTransition <= totNFAtransitions; iTransition++) {
    (void) STRING_extend ((char **) &detailLine, 0, (size_t) 20);
    snprintf (detailLine, 19, "| %5d | %5d | ",
              nfa_transitionNumber2originState (iTransition),
              nfa_transitionNumber2destState (iTransition) );
    (void) STRING_copy ((char **) &auxString, symbolCode2symbolString (nfa_transitionNumber2symbol (iTransition)));
    (void) STRING_justify ((char **) &auxString, symbolWidth, ' ', STRING_t_justify_left);
    (void) STRING_concatenate ((char **) &detailLine, auxString);
    (void) STRING_concatenate ((char **) &detailLine, " |");
    fprintf (nfaTextFilePt, "%s\n", detailLine);
  }
  fprintf (nfaTextFilePt, "%s\n", headerLine2);
  REPORT_newLine (nfaTextFilePt, 1);

  /* Print NFA state transitions summary statistics */

  fprintf (nfaTextFilePt, "+---------------------------------------------------------------------+\n");
  fprintf (nfaTextFilePt, "|                    NFA STATE TRANSITIONS SUMMARY                    |\n");
  fprintf (nfaTextFilePt, "+---------------------------------------------------------------------+\n");
  REPORT_newLine (nfaTextFilePt, 1);

  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */

  (void) STRING_copy ((char **) &headerLine1, "+-------+---------------+          +----------");
  (void) STRING_extend ((char **) &headerLine1, '-', (size_t) columnWidth);
  (void) STRING_concatenate ((char **) &headerLine1, "-+-------------+");
  fprintf (nfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "|       |  TRANSITIONS  |          | GRAMMAR SYMBOL");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine1, ' ', (size_t) columnWidth-5);
  (void) STRING_concatenate ((char **) &headerLine1, " | Number of   |");
  fprintf (nfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "| STATE +-------+-------+          +--------+-");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine1, '-', (size_t) columnWidth);
  (void) STRING_concatenate ((char **) &headerLine1, "-+ transitions |");
  fprintf (nfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "|       | From  |    To |          |   Code | String");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine1, ' ', (size_t) columnWidth-6);
  (void) STRING_concatenate ((char **) &headerLine1, " | with symbol |");
  fprintf (nfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "+-------+-------+-------+          +--------+-");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine1, '-', (size_t) columnWidth);
  (void) STRING_concatenate ((char **) &headerLine1, "-+-------------+");
  fprintf (nfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "+-------+-------+-------+");

  (void) STRING_copy ((char **) &headerLine2, "+--------+-");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine2, '-', (size_t) columnWidth);
  (void) STRING_concatenate ((char **) &headerLine2, "-+-------------+");

  totLines = GREATEST (totNFAstates, totNonTerminals + totTerminals + 1);
  totLines++;
  for (iLine = 1; iLine <= totLines; iLine++) {
    /* Print the left hand side of the table with transition details by NFA state */

    if (iLine <= totNFAstates) {
      stateCode = nfa_stateNumber2stateCode (iLine);
      (void) STRING_clear ((char **) &detailLine);
      (void) STRING_extend ((char **) &detailLine, 0, (size_t) 27);
      snprintf (detailLine, 26, "| %5d | %5d | %5d |",
                (int) stateCode,
                (int) nfa_stateCode2totTransitionsFromState (stateCode),
                (int) nfa_stateCode2totTransitionsToState (stateCode) );
    }
    else if (iLine == totNFAstates + 1)
      (void) STRING_copy ((char **) &detailLine, headerLine1);
    else
      (void) STRING_copy ((char **) &detailLine, "                         ");

    /* Print the right hand side of the table with transition details by grammar symbol */

    if (iLine <= totNonTerminals + totTerminals + 1) {
      (void) STRING_extend ((char **) &detailLine, ' ', (size_t) 10);
      (void) STRING_clear ((char **) &auxString);
      (void) STRING_extend ((char **) &auxString, 0, (size_t) 13);
      if (iLine <= totNonTerminals) {
        symbolNumber = iLine;
        symbolCode = symbolNumber2symbolCode (symbolNumber, t_nonTerminal);
        snprintf (auxString, 12, "|  %5d | ", symbolCode);
        (void) STRING_concatenate ((char **) &detailLine, auxString);
      }
      else if (iLine <= totNonTerminals + totTerminals) {
        symbolNumber = iLine - totNonTerminals;
        symbolCode = symbolNumber2symbolCode (symbolNumber, t_terminal);
        snprintf (auxString, 12, "|  %5d | ", symbolCode);
        (void) STRING_concatenate ((char **) &detailLine, auxString);
      }
      else {
        symbolCode = epsilon_code;
        snprintf (auxString, 12, "|        | ");
        (void) STRING_concatenate ((char **) &detailLine, auxString);
      }
      (void) STRING_copy ((char **) &auxString, symbolCode2symbolString (symbolCode));
      (void) STRING_justify ((char **) &auxString, columnWidth, ' ', STRING_t_justify_left);
      (void) STRING_concatenate ((char **) &detailLine, auxString);
      (void) STRING_clear ((char **) &auxString);
      (void) STRING_extend ((char **) &auxString, 0, (size_t) 18);
      snprintf (auxString, 17, " |    %5d    |", (int) nfa_symbolCode2totTransitionsWithSymbol (symbolCode));
      (void) STRING_concatenate ((char **) &detailLine, auxString);
    }
    else if (iLine == totNonTerminals + totTerminals + 2) {
      (void) STRING_extend ((char **) &detailLine, ' ', (size_t) 10);
      (void) STRING_concatenate ((char **) &detailLine, headerLine2);
    }
    fprintf (nfaTextFilePt, "%s\n", detailLine);
  }
  REPORT_newLine (nfaTextFilePt, 1);

  /* Print NFA states summary statistics */

  fprintf (nfaTextFilePt, "+--------------------------+\n");
  fprintf (nfaTextFilePt, "|    NFA STATES SUMMARY    |\n");
  fprintf (nfaTextFilePt, "+--------------------------+\n");
  fprintf (nfaTextFilePt, "|      Shift states: %5d |\n", (int) tot_NFA_shift_states);
  fprintf (nfaTextFilePt, "|     Reduce states: %5d |\n", (int) tot_NFA_reduce_states);
  fprintf (nfaTextFilePt, "| Non-deterministic: %5d |\n", (int) tot_NFA_non_deterministic_states);
  fprintf (nfaTextFilePt, "|      TOTAL STATES: %5d |\n", (int) tot_NFA_shift_states+tot_NFA_reduce_states+tot_NFA_non_deterministic_states);
  fprintf (nfaTextFilePt, "+--------------------------+\n");
  REPORT_newLine (nfaTextFilePt, 1);

  /* Print NFA summary statistics */

  fprintf (nfaTextFilePt, "+--------------------+\n");
  fprintf (nfaTextFilePt, "|    NFA SUMMARY     |\n");
  fprintf (nfaTextFilePt, "+--------------------+\n");
  fprintf (nfaTextFilePt, "|      States: %5d |\n", (int) totNFAstates);
  fprintf (nfaTextFilePt, "|       Items: %5d |\n", (int) totNFAstates);
  fprintf (nfaTextFilePt, "| Transitions: %5d |\n", (int) totNFAtransitions);
  fprintf (nfaTextFilePt, "+--------------------+\n");
  REPORT_newLine (nfaTextFilePt, 1);

  /* That's all */

  fflush (NULL);
  STRING_release (&auxString);
  STRING_release (&detailLine);
  STRING_release (&headerLine2);
  STRING_release (&headerLine1);
}

/*
*-----------------------------------------------------------------------------
* Print DFA, i.e. complete set of states and LR (0) items, in plain text format
*-----------------------------------------------------------------------------
*/

void print_dfa_txt (int argc, char *argv[]) {
  unsigned int
    iChar,
    iItem,
    iLine,
    iReduction,
    iState,
    iSymbol,
    iTransition,
    dotPosition,
    symbolNumber,
    ruleSize,
    totLines,
    totReductions,
    totItems;
  t_stateCode
    stateCode;
  t_stateType
    stateType;
  t_itemCode
    itemCode;
  t_symbolCode
    symbolCode;
  t_ruleNumber
    ruleNumber;
  size_t
    symbolStringLength;
  bool
    firstItem;
  const char
    *symbolString = NULL;
  size_t
    symbolWidth,
    columnWidth;
  char
    *headerLine1 = NULL,
    *headerLine2 = NULL,
    *detailLine  = NULL,
    *auxString   = NULL;

  /* Initialize a few things */

  headerLine1 = (char *) STRING_allocate();
  headerLine2 = (char *) STRING_allocate();
  detailLine  = (char *) STRING_allocate();
  auxString   = (char *) STRING_allocate();

  symbolWidth = maxGrammarSymbolLength;

  /* First print the header and grammar rules, then the states, items and transitions */

  if (! b_one_output_file || ! b_output_started) {
    print_output_header (dfaTextFilePt, dfaTextFileName, argc, argv);
    print_grammar_rules (dfaTextFilePt);
    b_output_started = true;
  }

  /* Print list of LR (0) states and items */

  fprintf (dfaTextFilePt, "+-------------------+\n");
  fprintf (dfaTextFilePt, "| DFA STATES: %-5d |\n", (int) totDFAstates);
  fprintf (dfaTextFilePt, "+-------------------+\n");
  REPORT_newLine (dfaTextFilePt, 1);

  for (iState = 1; iState <= totDFAstates; iState++) {
    stateCode = dfa_stateNumber2stateCode (iState);
    fprintf (dfaTextFilePt, "State %d: \n%s\n", stateCode, stateType2stateTypeString (stateCode2stateType (stateCode)));
    for (iItem = 1; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
      itemCode = dfa_stateCode2itemCode (stateCode, iItem);
      ruleNumber = itemCode2ruleNumber (itemCode);
      ruleSize = ruleNumber2ruleSize (ruleNumber);
      dotPosition = itemCode2dotPosition (itemCode);
      fprintf (dfaTextFilePt, "  %s ->", symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
      if (dotPosition == 0)
        fprintf (dfaTextFilePt, " •");
      for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
        fprintf (dfaTextFilePt, " %s", symbolCode2symbolString (rulePos2symbolCode (ruleNumber, iSymbol)));
        if (dotPosition == iSymbol)
          fprintf (dfaTextFilePt, " •");
      }
      REPORT_newLine (dfaTextFilePt, 1);
    }
    REPORT_newLine (dfaTextFilePt, 1);
  }

  /* Print list of states containing shift items only, possibly more than one */

  fprintf (dfaTextFilePt, "+---------------------------+\n");
  fprintf (dfaTextFilePt, "| SHIFT STATES: %-5d       |\n", (int) tot_DFA_shift_states);
  if (tot_DFA_shift_states > 0) {
    fprintf (dfaTextFilePt, "+-------+-------------------+\n");
    fprintf (dfaTextFilePt, "| State | Transition symbol |\n");
    fprintf (dfaTextFilePt, "+-------+-------------------+\n");
    for (iState = 1; iState <= totDFAstates; iState++) {
      stateCode = dfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_DFA_shift_state) {
        for (iItem = 1, firstItem = true; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
          if (firstItem) {
            fprintf (dfaTextFilePt, "| %5d | ", stateCode);
            firstItem = false;
          }
          else
            fprintf (dfaTextFilePt, "|       | ");
          itemCode = dfa_stateCode2itemCode (stateCode, iItem);
          ruleNumber = itemCode2ruleNumber (itemCode);
          ruleSize = ruleNumber2ruleSize (ruleNumber);
          dotPosition = itemCode2dotPosition (itemCode);
          symbolString = symbolCode2symbolString (rulePos2symbolCode (ruleNumber, dotPosition + 1));
          symbolStringLength = strlen (symbolString);
          fprintf (dfaTextFilePt, "%s", symbolString);
          if (symbolStringLength <= 17) {
            for (iChar = (unsigned int) symbolStringLength; iChar < 17; iChar++)
              fprintf (dfaTextFilePt, " ");
            fprintf (dfaTextFilePt, " |");
          }
          REPORT_newLine (dfaTextFilePt, 1);
        }
        fprintf (dfaTextFilePt, "+-------+-------------------+\n");
      }
    }
  }
  else
    fprintf (dfaTextFilePt, "+---------------------------+\n");
  REPORT_newLine (dfaTextFilePt, 1);

  /* Print list of states containing exactly one reduction item */

  fprintf (dfaTextFilePt, "+-------------------------+\n");
  fprintf (dfaTextFilePt, "| REDUCTION STATES: %-5d |\n", (int) tot_DFA_1_reduce_states);
  if (tot_DFA_1_reduce_states > 0) {
    fprintf (dfaTextFilePt, "+------------+------------+\n");
    fprintf (dfaTextFilePt, "|    State   |     Rule   |\n");
    fprintf (dfaTextFilePt, "+------------+------------+\n");
    for (iState = 1; iState <= totDFAstates; iState++) {
      stateCode = dfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_DFA_1_reduce_state) {
        ruleNumber = dfa_stateCode2reductionRule (stateCode, 1);
        fprintf (dfaTextFilePt, "|    %5d   |    %5d   |\n", (int) stateCode, (int) ruleNumber);
      }
    }
    fprintf (dfaTextFilePt, "+------------+------------+\n");
  }
  else
    fprintf (dfaTextFilePt, "+-------------------------+\n");
  REPORT_newLine (dfaTextFilePt, 1);

  /* Print list of states with shift-reduce conflicts */

  fprintf (dfaTextFilePt, "+-------------------------------+\n");
  fprintf (dfaTextFilePt, "| SHIFT-REDUCE CONFLICTS: %-5d |\n", (int) tot_DFA_shift_1_reduce_states);
  if (tot_DFA_shift_1_reduce_states > 0) {
      fprintf (dfaTextFilePt, "+-------+-----------------------+\n");
      fprintf (dfaTextFilePt, "| State | Actions               |\n");
      fprintf (dfaTextFilePt, "+-------+-----------------------+\n");
      for (iState = 1; iState <= totDFAstates; iState++) {
        stateCode = dfa_stateNumber2stateCode (iState);
        stateType = stateCode2stateType (stateCode);
        if (stateType == t_DFA_shift_1_reduce_state) {
          for (iItem = 1, firstItem = true; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
            if (firstItem) {
              fprintf (dfaTextFilePt, "| %5d | ", stateCode);
              firstItem = false;
            }
            else
              fprintf (dfaTextFilePt, "|       | ");
            itemCode = dfa_stateCode2itemCode (stateCode, iItem);
            ruleNumber = itemCode2ruleNumber (itemCode);
            if (isReductionItem (itemCode))
              fprintf (dfaTextFilePt, "reduce by rule %-5d  |\n", (int) ruleNumber);
            else {
              ruleSize = ruleNumber2ruleSize (ruleNumber);
              dotPosition = itemCode2dotPosition (itemCode);
              symbolString = symbolCode2symbolString (rulePos2symbolCode (ruleNumber, dotPosition + 1));
              symbolStringLength = strlen (symbolString);
              fprintf (dfaTextFilePt, "shift %s", symbolString);
              if (symbolStringLength <= 15) {
                for (iChar = (unsigned int) symbolStringLength; iChar < 15; iChar++)
                  fprintf (dfaTextFilePt, " ");
                fprintf (dfaTextFilePt, " |");
              }
              REPORT_newLine (dfaTextFilePt, 1);
            }
          }
          fprintf (dfaTextFilePt, "+-------+-----------------------+\n");
        }
    }
  }
  else
    fprintf (dfaTextFilePt, "+-------------------------------+\n");
  REPORT_newLine (dfaTextFilePt, 1);

  /* Print list of states containing reduce-reduce conflicts */

  fprintf (dfaTextFilePt, "+--------------------------------+\n");
  fprintf (dfaTextFilePt, "| REDUCE-REDUCE CONFLICTS: %-5d |\n", (int) tot_DFA_N_reduce_states);
  if (tot_DFA_N_reduce_states > 0) {
    fprintf (dfaTextFilePt, "+-----------------+--------------+\n");
    fprintf (dfaTextFilePt, "|      State      |      Rule    |\n");
    fprintf (dfaTextFilePt, "+-----------------+--------------+\n");
    for (iState = 1; iState <= totDFAstates; iState++) {
      stateCode = dfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_DFA_N_reduce_state) {
        totReductions = dfa_stateCode2totReductions (stateCode);
        for (iReduction = 1; iReduction <= totReductions; iReduction++) {
          ruleNumber = dfa_stateCode2reductionRule (stateCode, iReduction);
          if (iReduction == 1)
            fprintf (dfaTextFilePt, "|      %5d      |     %5d    |\n", (int) stateCode, (int) ruleNumber);
          else
            fprintf (dfaTextFilePt, "|                 |     %5d    |\n", (int) ruleNumber);
        }
        fprintf (dfaTextFilePt, "+-----------------+--------------+\n");
      }
    }
  }
  else
    fprintf (dfaTextFilePt, "+--------------------------------+\n");
  REPORT_newLine (dfaTextFilePt, 1);

  /* Print list of states containing shift-reduce-reduce conflicts */

  fprintf (dfaTextFilePt, "+--------------------------------------+\n");
  fprintf (dfaTextFilePt, "| SHIFT-REDUCE-REDUCE CONFLICTS: %-5d |\n", (int) tot_DFA_shift_N_reduce_states);
  if (tot_DFA_shift_N_reduce_states > 0) {
    fprintf (dfaTextFilePt, "+-------+------------------------------+\n");
    fprintf (dfaTextFilePt, "| State | Actions                      |\n");
    fprintf (dfaTextFilePt, "+-------+------------------------------+\n");
    for (iState = 1; iState <= totDFAstates; iState++) {
      stateCode = dfa_stateNumber2stateCode (iState);
      stateType = stateCode2stateType (stateCode);
      if (stateType == t_DFA_shift_N_reduce_state) {
        for (iItem = 1, firstItem = true; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
          if (firstItem) {
            fprintf (dfaTextFilePt, "| %5d | ", (int) stateCode);
            firstItem = false;
          }
          else
            fprintf (dfaTextFilePt, "|       | ");
          itemCode = dfa_stateCode2itemCode (stateCode, iItem);
          ruleNumber = itemCode2ruleNumber (itemCode);
          if (isReductionItem (itemCode))
            fprintf (dfaTextFilePt, "reduce by rule %-5d         |\n", (int) ruleNumber);
          else {
            ruleSize = ruleNumber2ruleSize (ruleNumber);
            dotPosition = itemCode2dotPosition (itemCode);
            symbolString = symbolCode2symbolString (rulePos2symbolCode (ruleNumber, dotPosition + 1));
            symbolStringLength = strlen (symbolString);
            fprintf (dfaTextFilePt, "shift %s", symbolString);
            if (symbolStringLength <= 22) {
              for (iChar = (unsigned int) symbolStringLength; iChar < 22; iChar++)
                fprintf (dfaTextFilePt, " ");
              fprintf (dfaTextFilePt, " |");
            }
            REPORT_newLine (dfaTextFilePt, 1);
          }
        }
        fprintf (dfaTextFilePt, "+-------+------------------------------+\n");
      }
    }
  }
  else
    fprintf (dfaTextFilePt, "+--------------------------------------+\n");
  REPORT_newLine (dfaTextFilePt, 1);

  /* Sort transitions by origin state code */

  dfa_sortTransitions (t_transitionSortKey_origin);

  /* Now print DFA state transitions */

  (void) STRING_copy ((char **) &headerLine1, "+--------------------------");
  (void) STRING_extend ((char **) &headerLine1, '-', (size_t) symbolWidth-8);
  (void) STRING_concatenate ((char **) &headerLine1, "+");
  REPORT_newLine (dfaTextFilePt, 1);
  fprintf (dfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "| STATE TRANSITIONS: ");
  (void) STRING_clear ((char **) &auxString);
  (void) STRING_set ((char **) &auxString, 0, (size_t) 5);
  snprintf (auxString, 4, "%-3d", (int) totDFAtransitions);
  (void) STRING_justify ((char **) &auxString, 7, ' ', STRING_t_justify_left);
  (void) STRING_concatenate ((char **) &headerLine1, auxString);
  (void) STRING_concatenate ((char **) &headerLine1, "|");
  fprintf (dfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine2, "+-------+-------+----------");
  (void) STRING_extend ((char **) &headerLine2, '-', (size_t) symbolWidth-8);
  (void) STRING_concatenate ((char **) &headerLine2, "+");
  fprintf (dfaTextFilePt, "%s\n", headerLine2);

  (void) STRING_copy ((char **) &headerLine1, "|  From |    To | With   ");
  (void) STRING_extend ((char **) &headerLine1, ' ', (size_t) symbolWidth-6);
  (void) STRING_concatenate ((char **) &headerLine1, "|");
  fprintf (dfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "| state | state | symbol ");
  (void) STRING_extend ((char **) &headerLine1, ' ', (size_t) symbolWidth-6);
  (void) STRING_concatenate ((char **) &headerLine1, "|");
  fprintf (dfaTextFilePt, "%s\n", headerLine1);
  fprintf (dfaTextFilePt, "%s\n", headerLine2);

  for (iTransition = 1; iTransition <= totDFAtransitions; iTransition++) {
    (void) STRING_set ((char **) &detailLine, 0, (size_t) 20);
    snprintf (detailLine, 19, "| %5d | %5d | ",
              dfa_transitionNumber2originState (iTransition),
              dfa_transitionNumber2destState (iTransition) );
    (void) STRING_copy ((char **) &auxString, symbolCode2symbolString (dfa_transitionNumber2symbol (iTransition)));
    (void) STRING_justify ((char **) &auxString, symbolWidth, ' ', STRING_t_justify_left);
    (void) STRING_concatenate ((char **) &detailLine, auxString);
    (void) STRING_concatenate ((char **) &detailLine, " |");
    fprintf (dfaTextFilePt, "%s\n", detailLine);
  }
  fprintf (dfaTextFilePt, "%s\n", headerLine2);
  REPORT_newLine (dfaTextFilePt, 1);

  /* Print DFA state transitions summary statistics */

  fprintf (dfaTextFilePt, "+---------------------------------------------------------------------+\n");
  fprintf (dfaTextFilePt, "|                    DFA STATE TRANSITIONS SUMMARY                    |\n");
  fprintf (dfaTextFilePt, "+---------------------------------------------------------------------+\n");
  REPORT_newLine (dfaTextFilePt, 1);

  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */

  (void) STRING_copy ((char **) &headerLine1, "+-------+---------------+          +----------");
  (void) STRING_extend ((char **) &headerLine1, '-', (size_t) columnWidth);
  (void) STRING_concatenate ((char **) &headerLine1, "-+-------------+");
  fprintf (dfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "|       |  TRANSITIONS  |          | GRAMMAR SYMBOL");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine1, ' ', (size_t) columnWidth-5);
  (void) STRING_concatenate ((char **) &headerLine1, " | Number of   |");
  fprintf (dfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "| STATE +-------+-------+          +--------+-");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine1, '-', (size_t) columnWidth);
  (void) STRING_concatenate ((char **) &headerLine1, "-+ transitions |");
  fprintf (dfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "|       | From  |    To |          |   Code | String");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine1, ' ', (size_t) columnWidth-6);
  (void) STRING_concatenate ((char **) &headerLine1, " | with symbol |");
  fprintf (dfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "+-------+-------+-------+          +--------+-");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine1, '-', (size_t) columnWidth);
  (void) STRING_concatenate ((char **) &headerLine1, "-+-------------+");
  fprintf (dfaTextFilePt, "%s\n", headerLine1);

  (void) STRING_copy ((char **) &headerLine1, "+-------+-------+-------+");

  (void) STRING_copy ((char **) &headerLine2, "+--------+-");
  columnWidth = GREATEST (symbolWidth, 6);  /* 6 is the length of "String" */
  (void) STRING_extend ((char **) &headerLine2, '-', (size_t) columnWidth);
  (void) STRING_concatenate ((char **) &headerLine2, "-+-------------+");

  totLines = GREATEST (totDFAstates, totNonTerminals + totTerminals + 1);
  totLines++;
  for (iLine = 1; iLine <= totLines; iLine++) {
    /* Print the left hand side of the table with transition details by DFA state */

    if (iLine <= totDFAstates) {
      stateCode = dfa_stateNumber2stateCode (iLine);
      (void) STRING_set ((char **) &detailLine, 0, (size_t) 27);
      snprintf (detailLine, 26, "| %5d | %5d | %5d |",
                (int) stateCode,
                (int) dfa_stateCode2totTransitionsFromState (stateCode),
                (int) dfa_stateCode2totTransitionsToState (stateCode) );
    }
    else if (iLine == totDFAstates + 1)
      (void) STRING_copy ((char **) &detailLine, headerLine1);
    else
      (void) STRING_copy ((char **) &detailLine, "                         ");

    /* Print the right hand side of the table with transition details by grammar symbol */

    if (iLine <= totNonTerminals + totTerminals + 1) {
      (void) STRING_extend ((char **) &detailLine, ' ', (size_t) 10);
      (void) STRING_set ((char **) &auxString, 0, (size_t) 13);
      if (iLine <= totNonTerminals) {
        symbolNumber = iLine;
        symbolCode = symbolNumber2symbolCode (symbolNumber, t_nonTerminal);
        snprintf (auxString, 12, "|  %5d | ", (int) symbolCode);
        (void) STRING_concatenate ((char **) &detailLine, auxString);
      }
      else if (iLine <= totNonTerminals + totTerminals) {
        symbolNumber = iLine - totNonTerminals;
        symbolCode = symbolNumber2symbolCode (symbolNumber, t_terminal);
        snprintf (auxString, 12, "|  %5d | ", (int) symbolCode);
        (void) STRING_concatenate ((char **) &detailLine, auxString);
      }
      else {
        symbolCode = epsilon_code;
        snprintf (auxString, 12, "|        | ");
        (void) STRING_concatenate ((char **) &detailLine, auxString);
      }
      (void) STRING_copy ((char **) &auxString, symbolCode2symbolString (symbolCode));
      (void) STRING_justify ((char **) &auxString, columnWidth, ' ', STRING_t_justify_left);
      (void) STRING_concatenate ((char **) &detailLine, auxString);
      (void) STRING_set ((char **) &auxString, 0, (size_t) 18);
      snprintf (auxString, 17, " |    %5d    |", (int) dfa_symbolCode2totTransitionsWithSymbol (symbolCode));
      (void) STRING_concatenate ((char **) &detailLine, auxString);
    }
    else if (iLine == totNonTerminals + totTerminals + 2) {
      (void) STRING_extend ((char **) &detailLine, ' ', (size_t) 10);
      (void) STRING_concatenate ((char **) &detailLine, headerLine2);
    }
    fprintf (dfaTextFilePt, "%s\n", detailLine);
  }
  REPORT_newLine (dfaTextFilePt, 1);

  /* Print DFA states summary statistics */

  fprintf (dfaTextFilePt, "+----------------------------+\n");
  fprintf (dfaTextFilePt, "|     DFA STATES SUMMARY     |\n");
  fprintf (dfaTextFilePt, "+----------------------------+\n");
  fprintf (dfaTextFilePt, "|  UNAMBIGUOUS STATES: %5d |\n", (int) tot_DFA_shift_states+tot_DFA_1_reduce_states);
  fprintf (dfaTextFilePt, "|        Shift states: %5d |\n", (int) tot_DFA_shift_states);
  fprintf (dfaTextFilePt, "|       Reduce states: %5d |\n", (int) tot_DFA_1_reduce_states);
  fprintf (dfaTextFilePt, "|     CONFLICT STATES: %5d |\n", (int) tot_DFA_shift_1_reduce_states+tot_DFA_N_reduce_states+tot_DFA_shift_N_reduce_states);
  fprintf (dfaTextFilePt, "|        Shift-reduce: %5d |\n", (int) tot_DFA_shift_1_reduce_states);
  fprintf (dfaTextFilePt, "|       Reduce-reduce: %5d |\n", (int) tot_DFA_N_reduce_states);
  fprintf (dfaTextFilePt, "| Shift-reduce-reduce: %5d |\n", (int) tot_DFA_shift_N_reduce_states);
  fprintf (dfaTextFilePt, "|        TOTAL STATES: %5d |\n", (int) tot_DFA_shift_states+tot_DFA_1_reduce_states+tot_DFA_N_reduce_states+tot_DFA_shift_1_reduce_states+tot_DFA_shift_N_reduce_states);
  fprintf (dfaTextFilePt, "+----------------------------+\n");
  REPORT_newLine (nfaTextFilePt, 1);

  /* Now print DFA summary statistics */

  for (iState = 1, totItems = 0; iState <= totDFAstates; iState++)
    totItems += dfa_stateCode2totItems (dfa_stateNumber2stateCode (iState));

  fprintf (dfaTextFilePt, "+--------------------+\n");
  fprintf (dfaTextFilePt, "|    DFA SUMMARY     |\n");
  fprintf (dfaTextFilePt, "+--------------------+\n");
  fprintf (dfaTextFilePt, "|      States: %5d |\n", (int) totDFAstates);
  fprintf (dfaTextFilePt, "|       Items: %5d |\n", (int) totItems);
  fprintf (dfaTextFilePt, "| Transitions: %5d |\n", (int) totDFAtransitions);
  fprintf (dfaTextFilePt, "+--------------------+\n");
  REPORT_newLine (dfaTextFilePt, 1);

  /* That's all */

  STRING_release (&auxString);
  STRING_release (&detailLine);
  STRING_release (&headerLine2);
  STRING_release (&headerLine1);
}

/*
*-----------------------------------------------------------------------------
* Export the NFA to DOT format
*-----------------------------------------------------------------------------
*/

void print_nfa_dot (void) {
  unsigned int
    iSymbol,
    iState,
    iTransition,
    dotPosition,
    ruleSize;
  t_stateCode
    stateCode;
  t_itemCode
    itemCode;
  t_ruleNumber
    ruleNumber;
  t_symbolCode
    symbolCode;
  t_symbolType
    symbolType;
  
  /* Open Dot file */
  /* In geraLR.c the file is already open */

  /* If initialize_svg_attributes() is not called, SVG diagrams will use these default values */

  /* Header */
  fprintf (nfaDotFilePt, "%s %s %s", GRAPH_LABEL_TYPE, GRAPH_LABEL_NFA_NAME, GRAPH_LABEL_INIT);

  /* Canvas style */
  fprintf (nfaDotFilePt, "%s",         GRAPH_LABEL_START);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_graph_attribute_size],     FSA[0].graphAttribute.size);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_graph_attribute_ratio],    FSA[0].graphAttribute.ratio);
  fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_graph_attribute_margin],   FSA[0].graphAttribute.margin);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_graph_attribute_ordering], FSA[0].graphAttribute.ordering);
  fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_graph_attribute_rotate],   FSA[0].graphAttribute.rotate);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_graph_attribute_color],    FSA[0].graphAttribute.color);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_graph_attribute_bgcolor],  FSA[0].graphAttribute.bgcolor);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_graph_attribute_splines],  FSA[0].graphAttribute.splines);
  fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_graph_attribute_nodesep],  FSA[0].graphAttribute.nodesep);
  fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_graph_attribute_ranksep],  FSA[0].graphAttribute.ranksep);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_graph_attribute_rankdir],  FSA[0].graphAttribute.rankdir);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_graph_attribute_rank],     FSA[0].graphAttribute.rank);
  fprintf (nfaDotFilePt, "%s",         LABEL_END);

  /* Shift states */
  fprintf (nfaDotFilePt, "\n// SHIFT STATES: %d \n", (int) tot_NFA_shift_states);
  if (tot_NFA_shift_states > 0) {
    fprintf (nfaDotFilePt, "%s",         NODE_LABEL_START);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_shift_state_height],      FSA[0].state[0].height);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_shift_state_width],       FSA[0].state[0].width);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_shift_state_fixedsize],   FSA[0].state[0].fixedsize);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_shift_state_shape],       FSA[0].state[0].shape);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_shift_state_color],       FSA[0].state[0].color);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_shift_state_fillcolor],   FSA[0].state[0].fillcolor);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_shift_state_style],       FSA[0].state[0].style);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_shift_state_regular],     FSA[0].state[0].regular);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_shift_state_peripheries], FSA[0].state[0].peripheries);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_shift_state_sides],       FSA[0].state[0].sides);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_shift_state_orientation], FSA[0].state[0].orientation);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_shift_state_distortion],  FSA[0].state[0].distortion);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_shift_state_skew],        FSA[0].state[0].skew);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_shift_state_penwidth],    FSA[0].state[0].penwidth);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_shift_state_margin],      FSA[0].state[0].margin);
    fprintf (nfaDotFilePt, "%s",         LABEL_END);
    for (iState = 1; iState <= totNFAstates; iState++) {
      stateCode = nfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_NFA_shift_state) {
        fprintf (nfaDotFilePt, "%s%d[id=\"%s%d\" label=<<TABLE BORDER=\"%d\">", ID_LABEL_STATE, stateCode, ID_LABEL_STATE, stateCode, 0);
        fprintf (nfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d%s\" id=\"%s%d%s\" BORDER=\"%d\" SIDES=\"%s\" %s><b>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, NFA_TABLE_BORDER, NFA_TABLE_SIDES, BUG_FIXED);
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>", FSA[0].font.colour.state_label_prefix, FSA[0].font.name.state_label_prefix, FSA[0].font.size.state_label_prefix, FSA[0].stateLabel.prefix);
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%d</font></b></TD></TR>" ,FSA[0].font.colour.state_label_number, FSA[0].font.name.state_label_number, FSA[0].font.size.state_label_number, nfa_stateCode2stateNumber (stateCode));
        fprintf (nfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d_%s%d\" id=\"%s%d_%s%d\" %s>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, 1, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, 1, BUG_FIXED);
        itemCode = nfa_stateCode2itemCode (stateCode);
        ruleNumber = itemCode2ruleNumber (itemCode);
        ruleSize = ruleNumber2ruleSize (ruleNumber);
        dotPosition = itemCode2dotPosition (itemCode);
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s </font>", FSA[0].font.colour.non_terminal, FSA[0].font.name.non_terminal, FSA[0].font.size.non_terminal, symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>",  FSA[0].font.colour.rule_arrow,   FSA[0].font.name.rule_arrow,   FSA[0].font.size.rule_arrow,   FSA[0].metasymbol.arrow_string);
        if (dotPosition == 0)
          fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.item_dot, FSA[0].font.name.item_dot, FSA[0].font.size.item_dot, FSA[0].metasymbol.dot_string);
        for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
          symbolCode = rulePos2symbolCode (ruleNumber, iSymbol);
          symbolType = symbolCode2symbolType (symbolCode);
          switch (symbolType) {
            case (t_terminal)   : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.terminal,     FSA[0].font.name.terminal,     FSA[0].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
            case (t_nonTerminal): fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.non_terminal, FSA[0].font.name.non_terminal, FSA[0].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
            case (t_epsilon)    : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.epsilon,      FSA[0].font.name.epsilon,      FSA[0].font.size.epsilon,      FSA[0].metasymbol.epsilon_string);      break;
            case (t_endOfInput) : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.end_of_input, FSA[0].font.name.end_of_input, FSA[0].font.size.end_of_input, FSA[0].metasymbol.end_of_input_string); break;
          }
          if (dotPosition == iSymbol)
            fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.item_dot, FSA[0].font.name.item_dot, FSA[0].font.size.item_dot, FSA[0].metasymbol.dot_string);
        }
        fprintf (nfaDotFilePt, "</TD></TR>\n%s</TABLE>>];\n", INDENT);
      }
    }
  }

  /* Non deterministic states */
  fprintf (nfaDotFilePt, "\n// NON DETERMINISTIC STATES: %d \n", (int) tot_NFA_non_deterministic_states);
  if (tot_NFA_non_deterministic_states > 0) {
    fprintf (nfaDotFilePt, "%s",         NODE_LABEL_START);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_epsilon_state_height],      FSA[0].state[1].height);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_epsilon_state_width],       FSA[0].state[1].width);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_epsilon_state_fixedsize],   FSA[0].state[1].fixedsize);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_epsilon_state_shape],       FSA[0].state[1].shape);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_epsilon_state_color],       FSA[0].state[1].color);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_epsilon_state_fillcolor],   FSA[0].state[1].fillcolor);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_epsilon_state_style],       FSA[0].state[1].style);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_epsilon_state_regular],     FSA[0].state[1].regular);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_epsilon_state_peripheries], FSA[0].state[1].peripheries);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_epsilon_state_sides],       FSA[0].state[1].sides);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_epsilon_state_orientation], FSA[0].state[1].orientation);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_epsilon_state_distortion],  FSA[0].state[1].distortion);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_epsilon_state_skew],        FSA[0].state[1].skew);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_epsilon_state_penwidth],    FSA[0].state[1].penwidth);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_epsilon_state_margin],      FSA[0].state[1].margin);
    fprintf (nfaDotFilePt, "%s",         LABEL_END);
    for (iState = 1; iState <= totNFAstates; iState++) {
      stateCode = nfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_NFA_non_deterministic_state) {
        fprintf (nfaDotFilePt, "%s%d[id=\"%s%d\" label=<<TABLE BORDER=\"%d\">", ID_LABEL_STATE, stateCode, ID_LABEL_STATE, stateCode, 0);
        fprintf (nfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d%s\" id=\"%s%d%s\" BORDER=\"%d\" SIDES=\"%s\" %s><b>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, NFA_TABLE_BORDER, NFA_TABLE_SIDES, BUG_FIXED);
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>", FSA[0].font.colour.state_label_prefix, FSA[0].font.name.state_label_prefix, FSA[0].font.size.state_label_prefix, FSA[0].stateLabel.prefix);
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%d</font></b></TD></TR>" ,FSA[0].font.colour.state_label_number, FSA[0].font.name.state_label_number, FSA[0].font.size.state_label_number, nfa_stateCode2stateNumber (stateCode));
        fprintf (nfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d_%s%d\" id=\"%s%d_%s%d\" %s>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, 1, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, 1, BUG_FIXED);
        itemCode = nfa_stateCode2itemCode (stateCode);
        ruleNumber = itemCode2ruleNumber (itemCode);
        ruleSize = ruleNumber2ruleSize (ruleNumber);
        dotPosition = itemCode2dotPosition (itemCode);
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s </font>", FSA[0].font.colour.non_terminal, FSA[0].font.name.non_terminal, FSA[0].font.size.non_terminal, symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>",  FSA[0].font.colour.rule_arrow,   FSA[0].font.name.rule_arrow,   FSA[0].font.size.rule_arrow,   FSA[0].metasymbol.arrow_string);
        if (dotPosition == 0)
          fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.item_dot, FSA[0].font.name.item_dot, FSA[0].font.size.item_dot, FSA[0].metasymbol.dot_string);
        for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
          symbolCode = rulePos2symbolCode (ruleNumber, iSymbol);
          symbolType = symbolCode2symbolType (symbolCode);
          switch (symbolType) {
            case (t_terminal)   : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.terminal,     FSA[0].font.name.terminal,     FSA[0].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
            case (t_nonTerminal): fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.non_terminal, FSA[0].font.name.non_terminal, FSA[0].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
            case (t_epsilon)    : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.epsilon,      FSA[0].font.name.epsilon,      FSA[0].font.size.epsilon,      FSA[0].metasymbol.epsilon_string);      break;
            case (t_endOfInput) : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.end_of_input, FSA[0].font.name.end_of_input, FSA[0].font.size.end_of_input, FSA[0].metasymbol.end_of_input_string); break;
          }
          if (dotPosition == iSymbol)
            fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.item_dot, FSA[0].font.name.item_dot, FSA[0].font.size.item_dot, FSA[0].metasymbol.dot_string);
        }
        fprintf (nfaDotFilePt, "</TD></TR>\n%s</TABLE>>];\n", INDENT);
      }
    }
  }
  
  /* Reduction states */
  fprintf (nfaDotFilePt, "\n// REDUCTION STATES: %d \n", (int) tot_NFA_reduce_states);
  if (tot_NFA_reduce_states > 0) {
    fprintf (nfaDotFilePt, "%s",         NODE_LABEL_START);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_reduce_state_height],      FSA[0].state[2].height);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_reduce_state_width],       FSA[0].state[2].width);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_reduce_state_fixedsize],   FSA[0].state[2].fixedsize);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_reduce_state_shape],       FSA[0].state[2].shape);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_reduce_state_color],       FSA[0].state[2].color);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_reduce_state_fillcolor],   FSA[0].state[2].fillcolor);
    fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_reduce_state_style],       FSA[0].state[2].style);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_reduce_state_regular],     FSA[0].state[2].regular);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_reduce_state_peripheries], FSA[0].state[2].peripheries);
    fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_reduce_state_sides],       FSA[0].state[2].sides);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_reduce_state_orientation], FSA[0].state[2].orientation);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_reduce_state_distortion],  FSA[0].state[2].distortion);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_reduce_state_skew],        FSA[0].state[2].skew);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_reduce_state_penwidth],    FSA[0].state[2].penwidth);
    fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_reduce_state_margin],      FSA[0].state[2].margin);
    fprintf (nfaDotFilePt, "%s",         LABEL_END);
    for (iState = 1; iState <= totNFAstates; iState++) {
      stateCode = nfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_NFA_reduce_state) {
        fprintf (nfaDotFilePt, "%s%d[id=\"%s%d\" label=<<TABLE BORDER=\"%d\">", ID_LABEL_STATE, stateCode, ID_LABEL_STATE, stateCode, 0);
        fprintf (nfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d%s\" id=\"%s%d%s\" BORDER=\"%d\" SIDES=\"%s\" %s><b>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, NFA_TABLE_BORDER, NFA_TABLE_SIDES, BUG_FIXED);
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>", FSA[0].font.colour.state_label_prefix, FSA[0].font.name.state_label_prefix, FSA[0].font.size.state_label_prefix, FSA[0].stateLabel.prefix);
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%d</font></b></TD></TR>" ,FSA[0].font.colour.state_label_number, FSA[0].font.name.state_label_number, FSA[0].font.size.state_label_number, nfa_stateCode2stateNumber (stateCode));
        fprintf (nfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d_%s%d\" id=\"%s%d_%s%d\" %s>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, 1, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, 1, BUG_FIXED);
        itemCode = nfa_stateCode2itemCode (stateCode);
        ruleNumber = itemCode2ruleNumber (itemCode);
        ruleSize = ruleNumber2ruleSize (ruleNumber);
        dotPosition = itemCode2dotPosition (itemCode);
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s </font>", FSA[0].font.colour.non_terminal, FSA[0].font.name.non_terminal, FSA[0].font.size.non_terminal, symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
        fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>",  FSA[0].font.colour.rule_arrow,   FSA[0].font.name.rule_arrow,   FSA[0].font.size.rule_arrow,   FSA[0].metasymbol.arrow_string);
        if (dotPosition == 0)
          fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.item_dot, FSA[0].font.name.item_dot, FSA[0].font.size.item_dot, FSA[0].metasymbol.dot_string);
        for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
          symbolCode = rulePos2symbolCode (ruleNumber, iSymbol);
          symbolType = symbolCode2symbolType (symbolCode);
          switch (symbolType) {
            case (t_terminal)   : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.terminal,     FSA[0].font.name.terminal,     FSA[0].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
            case (t_nonTerminal): fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.non_terminal, FSA[0].font.name.non_terminal, FSA[0].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
            case (t_epsilon)    : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.epsilon,      FSA[0].font.name.epsilon,      FSA[0].font.size.epsilon,      FSA[0].metasymbol.epsilon_string);      break;
            case (t_endOfInput) : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.end_of_input, FSA[0].font.name.end_of_input, FSA[0].font.size.end_of_input, FSA[0].metasymbol.end_of_input_string); break;
          }
          if (dotPosition == iSymbol)
            fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.item_dot, FSA[0].font.name.item_dot, FSA[0].font.size.item_dot, FSA[0].metasymbol.dot_string);
        }       
        fprintf (nfaDotFilePt, "</TD></TR>\n%s</TABLE>>];\n", INDENT);
      }
    }
  }

  /* Sort transitions by origin state code */
  nfa_sortTransitions (t_transitionSortKey_origin);
  
  /* NFA state transitions */
  fprintf (nfaDotFilePt, "\n// TRANSITIONS: %d \n", (int) totNFAtransitions);
  fprintf (nfaDotFilePt, "%s",         EDGE_LABEL_START);
  fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_transition_arrow_weight],        FSA[0].transitionArrow.weight);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_transition_arrow_style],         FSA[0].transitionArrow.style);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_transition_arrow_color],         FSA[0].transitionArrow.color);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_transition_arrow_dir],           FSA[0].transitionArrow.dir);
  fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_transition_arrow_tailclip],      FSA[0].transitionArrow.tailclip);
  fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_transition_arrow_headclip],      FSA[0].transitionArrow.headclip);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_transition_arrow_arrowhead],     FSA[0].transitionArrow.arrowhead);
  fprintf (nfaDotFilePt, "%s=\"%s\" ", NFA_Attributes[nfa_transition_arrow_arrowtail],     FSA[0].transitionArrow.arrowtail);
  fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_transition_arrow_arrowsize],     FSA[0].transitionArrow.arrowsize);
  fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_transition_arrow_labeldistance], FSA[0].transitionArrow.labeldistance);
  fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_transition_arrow_decorate],      FSA[0].transitionArrow.decorate);
  fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_transition_arrow_constraint],    FSA[0].transitionArrow.constraint);
  fprintf (nfaDotFilePt, "%s=%d ",     NFA_Attributes[nfa_transition_arrow_minlen],        FSA[0].transitionArrow.minlen);
  fprintf (nfaDotFilePt, "%s=%.2f ",   NFA_Attributes[nfa_transition_arrow_penwidth],      FSA[0].transitionArrow.penwidth);
  fprintf (nfaDotFilePt, "%s",         LABEL_END);
  for (iTransition = 1; iTransition <= totNFAtransitions; iTransition++) {
    fprintf (nfaDotFilePt, "%s%d->%s%d [id=\"%s%d\" label=<", ID_LABEL_STATE, nfa_transitionNumber2originState (iTransition), ID_LABEL_STATE, nfa_transitionNumber2destState (iTransition), ID_LABEL_TRANSITION, iTransition);
    symbolCode = nfa_transitionNumber2symbol (iTransition);
    symbolType = symbolCode2symbolType (symbolCode);
    switch (symbolType) {
      case (t_terminal)   : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.terminal,     FSA[0].font.name.terminal,     FSA[0].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
      case (t_nonTerminal): fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.non_terminal, FSA[0].font.name.non_terminal, FSA[0].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
      case (t_epsilon)    : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.epsilon,      FSA[0].font.name.epsilon,      FSA[0].font.size.epsilon,      FSA[0].metasymbol.epsilon_string);      break;
      case (t_endOfInput) : fprintf (nfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[0].font.colour.end_of_input, FSA[0].font.name.end_of_input, FSA[0].font.size.end_of_input, FSA[0].metasymbol.end_of_input_string); break;
    }
    fprintf (nfaDotFilePt, ">];\n");
  }
  
  /* That's all */
  fprintf (nfaDotFilePt, "%s", GRAPH_LABEL_STOP);
  fflush (NULL);
}

/*
*-----------------------------------------------------------------------------
* Export the NFA to SVG format
*-----------------------------------------------------------------------------
*/

int print_nfa_svg (void) {
  graph_t *g;
  GVC_t *gvc = gvContext();

  /* Open dot file for reading */
  errno = 0;
  if ((nfaDotFilePt = fopen (nfaDotFileName, "r")) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for reading", nfaDotFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }

  /* Open svg file for writing */
  errno = 0;
  if ((strcpy (nfaSvgFileName, grammarFileName)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (nfaSvgFileName ,\"%s\") failed", grammarFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((strcat (nfaSvgFileName, FILE_EXTENSION_NFA_SVG)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (nfaSvgFileName ,\"%s\") failed", FILE_EXTENSION_NFA_SVG);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((nfaSvgFilePt = fopen (nfaSvgFileName, "w")) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", nfaSvgFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }

  /* Export graph in svg format */
  g = agread (nfaDotFilePt, 0);
  gvLayout (gvc, g, "dot");
  gvRender (gvc, g, "svg", nfaSvgFilePt);
  gvFreeLayout (gvc, g);
  agclose (g);

  /* That's all */
  fclose (nfaDotFilePt);
  fclose (nfaSvgFilePt);
  return (gvFreeContext (gvc));
}

int print_nfa_svg2 (void) {
  /* Open svg file for writing */
  errno = 0;
  if ((strcpy (nfaSvgFileName, grammarFileName)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (nfaSvgFileName ,\"%s\") failed", grammarFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((strcat (nfaSvgFileName, FILE_EXTENSION_NFA_SVG)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (nfaSvgFileName ,\"%s\") failed", FILE_EXTENSION_NFA_SVG);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  
  char argStr[600] = "";
  sprintf (argStr, "dot -Tsvg %s > %s", nfaDotFileName, nfaSvgFileName);
  /* printf ("%s\n", argStr); */
  return (system (argStr));
}

/*
*-----------------------------------------------------------------------------
* Export the DFA to DOT format
*-----------------------------------------------------------------------------
*/

void print_dfa_dot (void) {
  unsigned int
    iItem,
    iState,
    iSymbol,
    iTransition,
    dotPosition,
    ruleSize;
  t_stateCode
    stateCode;
  t_itemCode
    itemCode;
  t_ruleNumber
    ruleNumber;
  t_symbolCode
    symbolCode;
  t_symbolType
    symbolType;

  /* Open Dot file */
  /* In geraLR.c the file is already open */

  /* If initialize_svg_attributes() is not called, SVG diagrams will use these default values */

  /* Header */
  fprintf (dfaDotFilePt, "%s %s %s", GRAPH_LABEL_TYPE, GRAPH_LABEL_DFA_NAME, GRAPH_LABEL_INIT);

  /* Canvas style */
  fprintf (dfaDotFilePt, "%s",         GRAPH_LABEL_START);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_graph_attribute_size],     FSA[1].graphAttribute.size);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_graph_attribute_ratio],    FSA[1].graphAttribute.ratio);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_graph_attribute_margin],   FSA[1].graphAttribute.margin);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_graph_attribute_ordering], FSA[1].graphAttribute.ordering);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_graph_attribute_rotate],   FSA[1].graphAttribute.rotate);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_graph_attribute_color],    FSA[1].graphAttribute.color);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_graph_attribute_bgcolor],  FSA[1].graphAttribute.bgcolor);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_graph_attribute_splines],  FSA[1].graphAttribute.splines);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_graph_attribute_nodesep],  FSA[1].graphAttribute.nodesep);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_graph_attribute_ranksep],  FSA[1].graphAttribute.ranksep);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_graph_attribute_rankdir],  FSA[1].graphAttribute.rankdir);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_graph_attribute_rank],     FSA[1].graphAttribute.rank);
  fprintf (dfaDotFilePt, "%s",         LABEL_END);

  /* States containing shift items only, possibly more than one */
  fprintf (dfaDotFilePt, "\n// SHIFT STATES: %d \n", (int) tot_DFA_shift_states);
  if (tot_DFA_shift_states > 0) {
    fprintf (dfaDotFilePt, "%s", NODE_LABEL_START);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_shift_state_height],      FSA[1].state[0].height);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_shift_state_width],       FSA[1].state[0].width);
    fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_shift_state_fixedsize],   FSA[1].state[0].fixedsize);
    fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_shift_state_shape],       FSA[1].state[0].shape);
    fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_shift_state_color],       FSA[1].state[0].color);
    fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_shift_state_fillcolor],   FSA[1].state[0].fillcolor);
    fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_shift_state_style],       FSA[1].state[0].style);
    fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_shift_state_regular],     FSA[1].state[0].regular);
    fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_shift_state_peripheries], FSA[1].state[0].peripheries);
    fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_shift_state_sides],       FSA[1].state[0].sides);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_shift_state_orientation], FSA[1].state[0].orientation);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_shift_state_distortion],  FSA[1].state[0].distortion);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_shift_state_skew],        FSA[1].state[0].skew);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_shift_state_penwidth],    FSA[1].state[0].penwidth);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_shift_state_margin],      FSA[1].state[0].margin);
    fprintf (dfaDotFilePt, "%s",         LABEL_END);
    for (iState = 1; iState <= totDFAstates; iState++) {
      stateCode = dfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_DFA_shift_state) {
        fprintf (dfaDotFilePt, "%s%d[id=\"%s%d\" label=<<TABLE BORDER=\"%d\">", ID_LABEL_STATE, stateCode, ID_LABEL_STATE, stateCode, 0);
        fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d%s\" id=\"%s%d%s\" BORDER=\"%d\" SIDES=\"%s\" %s><b>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, DFA_TABLE_BORDER, DFA_TABLE_SIDES, BUG_FIXED);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>", FSA[1].font.colour.state_label_prefix, FSA[1].font.name.state_label_prefix, FSA[1].font.size.state_label_prefix, FSA[1].stateLabel.prefix);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%d</font></b></TD></TR>" ,FSA[1].font.colour.state_label_number, FSA[1].font.name.state_label_number, FSA[1].font.size.state_label_number, stateCode);
        for (iItem = 1; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
          fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d_%s%d\" id=\"%s%d_%s%d\" ALIGN=\"%s\" %s>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, DFA_TABLE_ALIGN, BUG_FIXED);
          itemCode = dfa_stateCode2itemCode (stateCode, iItem);
          ruleNumber = itemCode2ruleNumber (itemCode);
          ruleSize = ruleNumber2ruleSize (ruleNumber);
          dotPosition = itemCode2dotPosition (itemCode);
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s </font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>",  FSA[1].font.colour.rule_arrow,   FSA[1].font.name.rule_arrow,   FSA[1].font.size.rule_arrow,   FSA[1].metasymbol.arrow_string);
          if (dotPosition == 0)
            fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
            symbolCode = rulePos2symbolCode (ruleNumber, iSymbol);
            symbolType = symbolCode2symbolType (symbolCode);
            switch (symbolType) {
              case (t_terminal)   : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.terminal,     FSA[1].font.name.terminal,     FSA[1].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
              case (t_nonTerminal): fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
              case (t_epsilon)    : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.epsilon,      FSA[1].font.name.epsilon,      FSA[1].font.size.epsilon,      FSA[1].metasymbol.epsilon_string);      break;
              case (t_endOfInput) : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.end_of_input, FSA[1].font.name.end_of_input, FSA[1].font.size.end_of_input, FSA[1].metasymbol.end_of_input_string); break;
            }
            if (dotPosition == iSymbol)
              fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          }
          fprintf (dfaDotFilePt, "</TD></TR>");
        }
        fprintf (dfaDotFilePt, "\n%s</TABLE>>];\n", INDENT);
      }
    }
  }
  
  /* States containing exactly one reduction item */
  fprintf (dfaDotFilePt, "\n// REDUCTION STATES: %d \n", (int) tot_DFA_1_reduce_states);
  if (tot_DFA_1_reduce_states > 0) {
    fprintf (dfaDotFilePt, "%s",         NODE_LABEL_START);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_reduce_state_height],      FSA[1].state[2].height);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_reduce_state_width],       FSA[1].state[2].width);
    fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_reduce_state_fixedsize],   FSA[1].state[2].fixedsize);
    fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_reduce_state_shape],       FSA[1].state[2].shape);
    fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_reduce_state_color],       FSA[1].state[2].color);
    fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_reduce_state_fillcolor],   FSA[1].state[2].fillcolor);
    fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_reduce_state_style],       FSA[1].state[2].style);
    fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_reduce_state_regular],     FSA[1].state[2].regular);
    fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_reduce_state_peripheries], FSA[1].state[2].peripheries);
    fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_reduce_state_sides],       FSA[1].state[2].sides);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_reduce_state_orientation], FSA[1].state[2].orientation);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_reduce_state_distortion],  FSA[1].state[2].distortion);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_reduce_state_skew],        FSA[1].state[2].skew);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_reduce_state_penwidth],    FSA[1].state[2].penwidth);
    fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_reduce_state_margin],      FSA[1].state[2].margin);
    fprintf (dfaDotFilePt, "%s",         LABEL_END);
    for (iState = 1; iState <= totDFAstates; iState++) {
      stateCode = dfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_DFA_1_reduce_state) {
        fprintf (dfaDotFilePt, "%s%d[id=\"%s%d\" label=<<TABLE BORDER=\"%d\">", ID_LABEL_STATE, stateCode, ID_LABEL_STATE, stateCode, 0);
        fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d%s\" id=\"%s%d%s\" BORDER=\"%d\" SIDES=\"%s\" %s><b>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, DFA_TABLE_BORDER, DFA_TABLE_SIDES, BUG_FIXED);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>", FSA[1].font.colour.state_label_prefix, FSA[1].font.name.state_label_prefix, FSA[1].font.size.state_label_prefix, FSA[1].stateLabel.prefix);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%d</font></b></TD></TR>" ,FSA[1].font.colour.state_label_number, FSA[1].font.name.state_label_number, FSA[1].font.size.state_label_number, stateCode);
        for (iItem = 1; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
          fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d_%s%d\" id=\"%s%d_%s%d\" ALIGN=\"%s\" %s>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, DFA_TABLE_ALIGN, BUG_FIXED);
          itemCode = dfa_stateCode2itemCode (stateCode, iItem);
          ruleNumber = itemCode2ruleNumber (itemCode);
          ruleSize = ruleNumber2ruleSize (ruleNumber);
          dotPosition = itemCode2dotPosition (itemCode);
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s </font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>",  FSA[1].font.colour.rule_arrow,   FSA[1].font.name.rule_arrow,   FSA[1].font.size.rule_arrow,   FSA[1].metasymbol.arrow_string);
          if (dotPosition == 0)
            fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
            symbolCode = rulePos2symbolCode (ruleNumber, iSymbol);
            symbolType = symbolCode2symbolType (symbolCode);
            switch (symbolType) {
              case (t_terminal)   : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.terminal,     FSA[1].font.name.terminal,     FSA[1].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
              case (t_nonTerminal): fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
              case (t_epsilon)    : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.epsilon,      FSA[1].font.name.epsilon,      FSA[1].font.size.epsilon,      FSA[1].metasymbol.epsilon_string);      break;
              case (t_endOfInput) : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.end_of_input, FSA[1].font.name.end_of_input, FSA[1].font.size.end_of_input, FSA[1].metasymbol.end_of_input_string); break;
            }
            if (dotPosition == iSymbol)
              fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          }
          fprintf (dfaDotFilePt, "</TD></TR>");
        }
        fprintf (dfaDotFilePt, "\n%s</TABLE>>];\n", INDENT);
      }
    }
  }

  fprintf (dfaDotFilePt, "\n// STATES WHITH CONFLICTS \n");
  fprintf (dfaDotFilePt, "%s",         NODE_LABEL_START);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_conflict_state_height],      FSA[1].state[1].height);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_conflict_state_width],       FSA[1].state[1].width);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_conflict_state_fixedsize],   FSA[1].state[1].fixedsize);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_conflict_state_shape],       FSA[1].state[1].shape);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_conflict_state_color],       FSA[1].state[1].color);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_conflict_state_fillcolor],   FSA[1].state[1].fillcolor);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_conflict_state_style],       FSA[1].state[1].style);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_conflict_state_regular],     FSA[1].state[1].regular);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_conflict_state_peripheries], FSA[1].state[1].peripheries);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_conflict_state_sides],       FSA[1].state[1].sides);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_conflict_state_orientation], FSA[1].state[1].orientation);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_conflict_state_distortion],  FSA[1].state[1].distortion);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_conflict_state_skew],        FSA[1].state[1].skew);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_conflict_state_penwidth],    FSA[1].state[1].penwidth);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_conflict_state_margin],      FSA[1].state[1].margin);
  fprintf (dfaDotFilePt, "%s",         LABEL_END);

  /* States with shift-reduce conflicts */
  fprintf (dfaDotFilePt, "\n// SHIFT-REDUCE CONFLICTS: %d \n", (int) tot_DFA_shift_1_reduce_states);
  if (tot_DFA_shift_1_reduce_states > 0) {
    for (iState = 1; iState <= totDFAstates; iState++) {
      stateCode = dfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_DFA_shift_1_reduce_state) {
        fprintf (dfaDotFilePt, "%s%d[id=\"%s%d\" label=<<TABLE BORDER=\"%d\">", ID_LABEL_STATE, stateCode, ID_LABEL_STATE, stateCode, 0);
        fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d%s\" id=\"%s%d%s\" BORDER=\"%d\" SIDES=\"%s\" %s><b>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, DFA_TABLE_BORDER, DFA_TABLE_SIDES, BUG_FIXED);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>", FSA[1].font.colour.state_label_prefix, FSA[1].font.name.state_label_prefix, FSA[1].font.size.state_label_prefix, FSA[1].stateLabel.prefix);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%d</font></b></TD></TR>" ,FSA[1].font.colour.state_label_number, FSA[1].font.name.state_label_number, FSA[1].font.size.state_label_number, stateCode);
        for (iItem = 1; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
          fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d_%s%d\" id=\"%s%d_%s%d\" ALIGN=\"%s\" %s>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, DFA_TABLE_ALIGN, BUG_FIXED);
          itemCode = dfa_stateCode2itemCode (stateCode, iItem);
          ruleNumber = itemCode2ruleNumber (itemCode);
          ruleSize = ruleNumber2ruleSize (ruleNumber);
          dotPosition = itemCode2dotPosition (itemCode);
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s </font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>",  FSA[1].font.colour.rule_arrow,   FSA[1].font.name.rule_arrow,   FSA[1].font.size.rule_arrow,   FSA[1].metasymbol.arrow_string);
          if (dotPosition == 0)
            fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
            symbolCode = rulePos2symbolCode (ruleNumber, iSymbol);
            symbolType = symbolCode2symbolType (symbolCode);
            switch (symbolType) {
              case (t_terminal)   : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.terminal,     FSA[1].font.name.terminal,     FSA[1].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
              case (t_nonTerminal): fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
              case (t_epsilon)    : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.epsilon,      FSA[1].font.name.epsilon,      FSA[1].font.size.epsilon,      FSA[1].metasymbol.epsilon_string);      break;
              case (t_endOfInput) : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.end_of_input, FSA[1].font.name.end_of_input, FSA[1].font.size.end_of_input, FSA[1].metasymbol.end_of_input_string); break;
            }
            if (dotPosition == iSymbol)
              fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          }         
          fprintf (dfaDotFilePt, "</TD></TR>");
        }
        fprintf (dfaDotFilePt, "\n%s</TABLE>>];\n", INDENT);
      }
    }
  }

  /* States containing reduce-reduce conflicts */
  fprintf (dfaDotFilePt, "\n// REDUCE-REDUCE CONFLICTS: %d \n", (int) tot_DFA_N_reduce_states);
  if (tot_DFA_N_reduce_states > 0) {
    for (iState = 1; iState <= totDFAstates; iState++) {
      stateCode = dfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_DFA_N_reduce_state) {
        fprintf (dfaDotFilePt, "%s%d[id=\"%s%d\" label=<<TABLE BORDER=\"%d\">", ID_LABEL_STATE, stateCode, ID_LABEL_STATE, stateCode, 0);
        fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d%s\" id=\"%s%d%s\" BORDER=\"%d\" SIDES=\"%s\" %s><b>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, DFA_TABLE_BORDER, DFA_TABLE_SIDES, BUG_FIXED);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>", FSA[1].font.colour.state_label_prefix, FSA[1].font.name.state_label_prefix, FSA[1].font.size.state_label_prefix, FSA[1].stateLabel.prefix);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%d</font></b></TD></TR>" ,FSA[1].font.colour.state_label_number, FSA[1].font.name.state_label_number, FSA[1].font.size.state_label_number, stateCode);
        for (iItem = 1; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
          fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d_%s%d\" id=\"%s%d_%s%d\" ALIGN=\"%s\" %s>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, DFA_TABLE_ALIGN, BUG_FIXED);
          itemCode = dfa_stateCode2itemCode (stateCode, iItem);
          ruleNumber = itemCode2ruleNumber (itemCode);
          ruleSize = ruleNumber2ruleSize (ruleNumber);
          dotPosition = itemCode2dotPosition (itemCode);
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s </font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>",  FSA[1].font.colour.rule_arrow,   FSA[1].font.name.rule_arrow,   FSA[1].font.size.rule_arrow,   FSA[1].metasymbol.arrow_string);
          if (dotPosition == 0)
            fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
            symbolCode = rulePos2symbolCode (ruleNumber, iSymbol);
            symbolType = symbolCode2symbolType (symbolCode);
            switch (symbolType) {
              case (t_terminal)   : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.terminal,     FSA[1].font.name.terminal,     FSA[1].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
              case (t_nonTerminal): fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
              case (t_epsilon)    : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.epsilon,      FSA[1].font.name.epsilon,      FSA[1].font.size.epsilon,      FSA[1].metasymbol.epsilon_string);      break;
              case (t_endOfInput) : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.end_of_input, FSA[1].font.name.end_of_input, FSA[1].font.size.end_of_input, FSA[1].metasymbol.end_of_input_string); break;
            }
            if (dotPosition == iSymbol)
              fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          }         
          fprintf (dfaDotFilePt, "</TD></TR>");
        }
        fprintf (dfaDotFilePt, "\n%s</TABLE>>];\n", INDENT);
      }
    }
  }

  /* States containing shift-reduce-reduce conflicts */
  fprintf (dfaDotFilePt, "\n// SHIFT-REDUCE-REDUCE CONFLICTS: %d \n", (int) tot_DFA_shift_N_reduce_states);
  if (tot_DFA_shift_N_reduce_states > 0) {
    for (iState = 1; iState <= totDFAstates; iState++) {
      stateCode = dfa_stateNumber2stateCode (iState);
      if (stateCode2stateType (stateCode) == t_DFA_shift_N_reduce_state) {
        fprintf (dfaDotFilePt, "%s%d[id=\"%s%d\" label=<<TABLE BORDER=\"%d\">", ID_LABEL_STATE, stateCode, ID_LABEL_STATE, stateCode, 0);
        fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d%s\" id=\"%s%d%s\" BORDER=\"%d\" SIDES=\"%s\" %s><b>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, ID_LABEL_STATE, stateCode, ID_LABEL_TITLE, DFA_TABLE_BORDER, DFA_TABLE_SIDES, BUG_FIXED);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>", FSA[1].font.colour.state_label_prefix, FSA[1].font.name.state_label_prefix, FSA[1].font.size.state_label_prefix, FSA[1].stateLabel.prefix);
        fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%d</font></b></TD></TR>" ,FSA[1].font.colour.state_label_number, FSA[1].font.name.state_label_number, FSA[1].font.size.state_label_number, stateCode);
        for (iItem = 1; iItem <= dfa_stateCode2totItems (stateCode); iItem++) {
          fprintf (dfaDotFilePt, "\n%s<TR><TD TITLE=\"%s%d_%s%d\" id=\"%s%d_%s%d\" ALIGN=\"%s\" %s>", INDENT, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, ID_LABEL_STATE, stateCode, ID_LABEL_ITEM, iItem, DFA_TABLE_ALIGN, BUG_FIXED);
          itemCode = dfa_stateCode2itemCode (stateCode, iItem);
          ruleNumber = itemCode2ruleNumber (itemCode);
          ruleSize = ruleNumber2ruleSize (ruleNumber);
          dotPosition = itemCode2dotPosition (itemCode);
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s </font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (rulePos2symbolCode (ruleNumber, 0)));
          fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\">%s</font>",  FSA[1].font.colour.rule_arrow,   FSA[1].font.name.rule_arrow,   FSA[1].font.size.rule_arrow,   FSA[1].metasymbol.arrow_string);
          if (dotPosition == 0)
            fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          for (iSymbol = 1; iSymbol <= ruleSize; iSymbol++) {
            symbolCode = rulePos2symbolCode (ruleNumber, iSymbol);
            symbolType = symbolCode2symbolType (symbolCode);
            switch (symbolType) {
              case (t_terminal)   : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.terminal,     FSA[1].font.name.terminal,     FSA[1].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
              case (t_nonTerminal): fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
              case (t_epsilon)    : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.epsilon,      FSA[1].font.name.epsilon,      FSA[1].font.size.epsilon,      FSA[1].metasymbol.epsilon_string);      break;
              case (t_endOfInput) : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.end_of_input, FSA[1].font.name.end_of_input, FSA[1].font.size.end_of_input, FSA[1].metasymbol.end_of_input_string); break;
            }
            if (dotPosition == iSymbol)
              fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.item_dot, FSA[1].font.name.item_dot, FSA[1].font.size.item_dot, FSA[1].metasymbol.dot_string);
          }         
          fprintf (dfaDotFilePt, "</TD></TR>");
        }
        fprintf (dfaDotFilePt, "\n%s</TABLE>>];\n", INDENT);
      }
    }
  }

  /* Sort transitions by origin state code */
  dfa_sortTransitions (t_transitionSortKey_origin);

  /* State transitions */
  fprintf (dfaDotFilePt, "\n// TRANSITIONS: %d \n", (int) totDFAtransitions);
  fprintf (dfaDotFilePt, "%s",         EDGE_LABEL_START);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_transition_arrow_weight],        FSA[1].transitionArrow.weight);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_transition_arrow_style],         FSA[1].transitionArrow.style);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_transition_arrow_color],         FSA[1].transitionArrow.color);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_transition_arrow_dir],           FSA[1].transitionArrow.dir);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_transition_arrow_tailclip],      FSA[1].transitionArrow.tailclip);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_transition_arrow_headclip],      FSA[1].transitionArrow.headclip);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_transition_arrow_arrowhead],     FSA[1].transitionArrow.arrowhead);
  fprintf (dfaDotFilePt, "%s=\"%s\" ", DFA_Attributes[dfa_transition_arrow_arrowtail],     FSA[1].transitionArrow.arrowtail);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_transition_arrow_arrowsize],     FSA[1].transitionArrow.arrowsize);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_transition_arrow_labeldistance], FSA[1].transitionArrow.labeldistance);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_transition_arrow_decorate],      FSA[1].transitionArrow.decorate);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_transition_arrow_constraint],    FSA[1].transitionArrow.constraint);
  fprintf (dfaDotFilePt, "%s=%d ",     DFA_Attributes[dfa_transition_arrow_minlen],        FSA[1].transitionArrow.minlen);
  fprintf (dfaDotFilePt, "%s=%.2f ",   DFA_Attributes[dfa_transition_arrow_penwidth],      FSA[1].transitionArrow.penwidth);
  fprintf (dfaDotFilePt, "%s",         LABEL_END);
  for (iTransition = 1; iTransition <= totDFAtransitions; iTransition++) {
    fprintf (dfaDotFilePt, "%s%d->%s%d [id=\"%s%d\" label=<", ID_LABEL_STATE, dfa_transitionNumber2originState (iTransition), ID_LABEL_STATE, dfa_transitionNumber2destState (iTransition), ID_LABEL_TRANSITION, iTransition);
    symbolCode = dfa_transitionNumber2symbol (iTransition);
    symbolType = symbolCode2symbolType (symbolCode);
    switch (symbolType) {
      case (t_terminal)   : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.terminal,     FSA[1].font.name.terminal,     FSA[1].font.size.terminal,     symbolCode2symbolString (symbolCode));  break;
      case (t_nonTerminal): fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.non_terminal, FSA[1].font.name.non_terminal, FSA[1].font.size.non_terminal, symbolCode2symbolString (symbolCode));  break;
      case (t_epsilon)    : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.epsilon,      FSA[1].font.name.epsilon,      FSA[1].font.size.epsilon,      FSA[1].metasymbol.epsilon_string);      break;
      case (t_endOfInput) : fprintf (dfaDotFilePt, "<font color=\"%s\" face=\"%s\" point-size=\"%.1f\"> %s</font>", FSA[1].font.colour.end_of_input, FSA[1].font.name.end_of_input, FSA[1].font.size.end_of_input, FSA[1].metasymbol.end_of_input_string); break;
    }   
    fprintf (dfaDotFilePt, ">];\n");
  }

  /* That's all */
  fprintf (dfaDotFilePt, "%s", GRAPH_LABEL_STOP);
  
  fflush (NULL);
}

/*
*-----------------------------------------------------------------------------
* Export the DFA to SVG format
*-----------------------------------------------------------------------------
*/

int print_dfa_svg (void) {
  graph_t *g;
  GVC_t *gvc = gvContext();

  /* Open dot file for reading */
  errno = 0;
  if ((dfaDotFilePt = fopen (dfaDotFileName, "r")) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for reading", dfaDotFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }

  /* Open svg file for writing */
  errno = 0;
  if ((strcpy (dfaSvgFileName, grammarFileName)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (dfaSvgFileName ,\"%s\") failed", grammarFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((strcat (dfaSvgFileName, FILE_EXTENSION_DFA_SVG)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (dfaSvgFileName ,\"%s\") failed", FILE_EXTENSION_DFA_SVG);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((dfaSvgFilePt = fopen (dfaSvgFileName, "w")) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", dfaSvgFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }

  /* Graph render to svg */
  g = agread (dfaDotFilePt, 0);
  gvLayout (gvc, g, "dot");
  gvRender (gvc, g, "svg", dfaSvgFilePt);
  gvFreeLayout (gvc, g);
  agclose (g);

  /* That's all */
  fclose (dfaDotFilePt);
  fclose (dfaSvgFilePt);

  return (gvFreeContext (gvc));
}

int print_dfa_svg2 (void) {
  /* Open svg file to writing */
  errno = 0;
  if ((strcpy (dfaSvgFileName, grammarFileName)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (dfaSvgFileName ,\"%s\") failed", grammarFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((strcat (dfaSvgFileName, FILE_EXTENSION_DFA_SVG)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (dfaSvgFileName ,\"%s\") failed", FILE_EXTENSION_DFA_SVG);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  
  char argStr[600] = "";
  sprintf (argStr, "dot -Tsvg %s > %s", dfaDotFileName, dfaSvgFileName);
  /* printf ("%s\n", argStr); */
  return (system (argStr));
}

/*
*-----------------------------------------------------------------------------
* Print LIDAS comands for the NFA animation
*-----------------------------------------------------------------------------
*/

void print_nfa_lda (void) {
  bool
    visitedStates[totNFAstates],
    isFirst;
  struct {
    unsigned int destStateNumber;
    unsigned int transitionNumber;
  }
    queue [totNFAtransitions];
  unsigned int
    queueStart,
    queueEnd,
    iState,
    iTransition,
    transitionNumber,
    iBlink,
    currentStateNumber;
  t_stateCode
    currentStateCode,
    originStateCode;

  /* Open lda file for writing */
  errno = 0;
  if ((strcpy (nfaLdaFileName, grammarFileName)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (nfaLdaFileName ,\"%s\") failed", grammarFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((strcat (nfaLdaFileName, FILE_EXTENSION_NFA_LDA)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (nfaLdaFileName ,\"%s\") failed", FILE_EXTENSION_NFA_LDA);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((nfaLdaFilePt = fopen (nfaLdaFileName, "w")) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", nfaLdaFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }

  /* Set all states to unvisited */
  for (iState = 0; iState < totNFAstates; iState++) 
    visitedStates[iState] = false;
  
  /* Print commands in lda file */
  for (iState = 0; iState < totNFAstates; iState++) {
    if (! visitedStates[iState]) {
      queueStart = queueEnd = 0;
      queue[queueStart].destStateNumber = iState + 1;
      queue[queueStart].transitionNumber = 1;
      /* Breadth-first search */
      while (queueEnd >= queueStart) {
        currentStateNumber = queue[queueStart].destStateNumber;
        currentStateCode   = nfa_stateNumber2stateCode (currentStateNumber);
        transitionNumber   = queue[queueStart].transitionNumber;
        /* Show transitions */
        if (queueStart != 0) {
          originStateCode = nfa_transitionNumber2originState (transitionNumber);
          if (symbolCode2symbolType (nfa_transitionNumber2symbol (transitionNumber)) != t_epsilon) {
            /* Blink item if the transition symbol is not epsilon */
            for (iBlink = 0, isFirst = true; iBlink < ANIMATION_BLINKS; iBlink++) {
              if (isFirst) {
                fprintf (nfaLdaFilePt, "apague \"a_%s%d_%s%d\" apos tecla por %.2f segundos;\n", ID_LABEL_STATE, originStateCode, ID_LABEL_ITEM, 1, ANIMATION_DELAY);
                isFirst = false;
              } 
              else {
                fprintf (nfaLdaFilePt, "apague \"a_%s%d_%s%d\" apos %.2f segundos por %.2f segundos;\n", ID_LABEL_STATE, originStateCode, ID_LABEL_ITEM, 1, ANIMATION_DELAY, ANIMATION_DELAY);
              }
            }
            /* Transition */
            fprintf (nfaLdaFilePt, "\nmostre \"%s%d\" apos %.2f segundos;\n\n", ID_LABEL_TRANSITION, transitionNumber, ANIMATION_DELAY);      
          } 
          else {
            /* Transition */
            fprintf (nfaLdaFilePt, "mostre \"%s%d\" apos tecla;\n\n", ID_LABEL_TRANSITION, transitionNumber);    
          }
        }
        /* Print unvisited states */
        if (! visitedStates[currentStateNumber-1]) {
          /* Set current state to visited */
          visitedStates[currentStateNumber-1] = true;
          /* Define all items on state */
          fprintf (nfaLdaFilePt, "defina %s%d \"a_%s%d_%s.*\";\n", ID_DEFINE_ITEMS, currentStateCode, ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM);
          /* Show state box */
          fprintf (nfaLdaFilePt, "mostre \"%s%d\" apos tecla;\n", ID_LABEL_STATE, currentStateCode);
          /* Show title */
          fprintf (nfaLdaFilePt, "mostre \"a_%s%d%s\" apos tecla;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_TITLE);
          /* Show items set one by one */
          fprintf (nfaLdaFilePt, "mostre %s%d umAum;\n\n", ID_DEFINE_ITEMS, currentStateCode);
          /* Queue transitions */
          for (iTransition = 1; iTransition <= nfa_stateCode2totTransitionsFromState (currentStateCode); iTransition++) {
            transitionNumber = nfa_stateCode2transitionNumber (currentStateCode, iTransition);
            queueEnd++;
            queue[queueEnd].transitionNumber = transitionNumber;
            queue[queueEnd].destStateNumber = nfa_stateCode2stateNumber (nfa_transitionNumber2destState (transitionNumber));
          }
        }
        queueStart++;
      }
    }
  }

  /* That's all */
  fclose (nfaLdaFilePt);
  fflush (NULL);
}

void print_nfa_lda2 (void) {
  bool
    visitedStates[totNFAstates];
  struct {
    unsigned int destStateNumber;
    unsigned int transitionNumber;
  }
    queue [totNFAtransitions];
  unsigned int
    queueStart,
    queueEnd,
    iState,
    iTransition,
    transitionNumber,
    iBlink,
    currentStateNumber;
  t_stateCode
    currentStateCode,
    originStateCode;

  /* Open lda file for writing */
  errno = 0;
  if ((strcpy (nfaLdaFileName, grammarFileName)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (nfaLdaFileName ,\"%s\") failed", grammarFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((strcat (nfaLdaFileName, FILE_EXTENSION_NFA_LDA)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (nfaLdaFileName ,\"%s\") failed", FILE_EXTENSION_NFA_LDA);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((nfaLdaFilePt = fopen (nfaLdaFileName, "w")) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", nfaLdaFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }

  /* Set all states to unvisited */
  for (iState = 0; iState < totNFAstates; iState++) 
    visitedStates[iState] = false;
  
  /* Print commands in lda file */
  for (iState = 0; iState < totNFAstates; iState++) {
    if (! visitedStates[iState]) {
      queueStart = queueEnd = 0;
      queue[queueStart].destStateNumber = iState + 1;
      queue[queueStart].transitionNumber = 1;
      /* Breadth-first search */
      while (queueEnd >= queueStart) {
        currentStateNumber = queue[queueStart].destStateNumber;
        currentStateCode   = nfa_stateNumber2stateCode (currentStateNumber);
        transitionNumber   = queue[queueStart].transitionNumber;
        /* Show transitions */
        if (queueStart != 0) {
          originStateCode = nfa_transitionNumber2originState (transitionNumber);
          if (symbolCode2symbolType (nfa_transitionNumber2symbol (transitionNumber)) != t_epsilon) {
            /* Blink item if the transition symbol is not epsilon */
            for (iBlink = 0; iBlink < ANIMATION_BLINKS; iBlink++) {
              fprintf (nfaLdaFilePt, "apague \"a_%s%d_%s%d\" apos %.2f segundos por %.2f segundos;\n", ID_LABEL_STATE, originStateCode, ID_LABEL_ITEM, 1, ANIMATION_DELAY, ANIMATION_DELAY);
            }
            fprintf (nfaLdaFilePt, "\n");
          }
          /* Transition */
          fprintf (nfaLdaFilePt, "mostre \"%s%d\" apos %.2f segundos;\n\n", ID_LABEL_TRANSITION, transitionNumber, ANIMATION_DELAY);          
        }
        /* Print unvisited states */
        if (! visitedStates[currentStateNumber-1]) {
          /* Set current state to visited */
          visitedStates[currentStateNumber-1] = true;
          /* Define all items on state */
          fprintf (nfaLdaFilePt, "defina %s%d \"a_%s%d_%s.*\";\n", ID_DEFINE_ITEMS, currentStateCode, ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM);
          /* Show state box */
          fprintf (nfaLdaFilePt, "mostre \"%s%d\" apos %.2f segundos;\n", ID_LABEL_STATE, currentStateCode, ANIMATION_DELAY);
          /* Show title */
          fprintf (nfaLdaFilePt, "mostre \"a_%s%d%s\" apos %.2f segundos;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_TITLE, ANIMATION_DELAY);
          /* Show items set one by one */
          fprintf (nfaLdaFilePt, "mostre %s%d umAum apos %.2f segundos;\n\n", ID_DEFINE_ITEMS, currentStateCode, ANIMATION_DELAY);        
          /* Queue transitions */
          for (iTransition = 1; iTransition <= nfa_stateCode2totTransitionsFromState (currentStateCode); iTransition++) {
            transitionNumber = nfa_stateCode2transitionNumber (currentStateCode, iTransition);
            queueEnd++;
            queue[queueEnd].transitionNumber = transitionNumber;
            queue[queueEnd].destStateNumber = nfa_stateCode2stateNumber (nfa_transitionNumber2destState (transitionNumber));
          }
        }
        queueStart++;
      }
    }
  }
    
  /* That's all */
  fclose (nfaLdaFilePt);
  fflush (NULL);
}

/*
*-----------------------------------------------------------------------------
* Print LIDAS comands for the DFA animation
*-----------------------------------------------------------------------------
*/

void print_dfa_lda (void) {
  bool
    visitedStates[totDFAstates],
    isFirst;
  struct {
    unsigned int destStateNumber;
    unsigned int transitionNumber;
  }
    queue [totDFAtransitions];
  unsigned int
    queueStart,
    queueEnd,
    iState,
    iItem,
    iItem2,
    iTransition,
    transitionNumber,
    iBlink,
    currentStateNumber;
  t_stateCode
    currentStateCode,
    originStateCode;
  t_itemCode
    itemCode,
    newItemCode;
  
  /* Open lda file for writing */
  errno = 0;
  if ((strcpy (dfaLdaFileName, grammarFileName)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (dfaLdaFileName ,\"%s\") failed", grammarFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((strcat (dfaLdaFileName, FILE_EXTENSION_DFA_LDA)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (dfaLdaFileName ,\"%s\") failed", FILE_EXTENSION_DFA_LDA);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((dfaLdaFilePt = fopen (dfaLdaFileName, "w")) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", dfaLdaFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }

  /* Set all states to unvisited */
  for (iState = 0; iState < totDFAstates; iState++) 
    visitedStates[iState] = false;
  
  /* Print commands in lda file */
  for (iState = 1; iState <= totDFAstates; iState++) {
    if (! visitedStates[iState-1]) {
      queueStart = queueEnd = 0;
      queue[queueStart].destStateNumber = iState;
      queue[queueStart].transitionNumber = 1;
      /* Breadth-first search */
      while (queueEnd >= queueStart) {
        currentStateNumber = queue[queueStart].destStateNumber;
        currentStateCode   = dfa_stateNumber2stateCode (currentStateNumber);
        transitionNumber   = queue[queueStart].transitionNumber;
        /* Show transitions */
        if (queueStart != 0) {
          originStateCode = dfa_transitionNumber2originState (transitionNumber);
          /* Set items responsible for the transition */
          fprintf (dfaLdaFilePt, "defina %s%d_%d \"", ID_DEFINE_ITEMS, originStateCode, transitionNumber);
          /* Look for items from that state that consume the transition symbol */
          for (iItem = 1, isFirst = true; iItem <= dfa_stateCode2totItems (originStateCode); iItem++) {
            itemCode = dfa_stateCode2itemCode (originStateCode, iItem);
            if (isReductionItem (itemCode))
              continue;
            if (dfa_transitionNumber2symbol (transitionNumber) != itemCode2transitionSymbol (itemCode))
              continue;
            if (! isFirst)
              fprintf (dfaLdaFilePt, "|");
            fprintf (dfaLdaFilePt, "(a_%s%d_%s%d)",  ID_LABEL_STATE, originStateCode, ID_LABEL_ITEM, iItem);
            isFirst = false;
          }
          fprintf (dfaLdaFilePt, "\";\n");
          /* Blink items */
          for (iBlink = 0, isFirst = true; iBlink < ANIMATION_BLINKS; iBlink++) {
            if (isFirst) {
              fprintf (dfaLdaFilePt, "apague %s%d_%d juntos por %.2f segundos;\n", ID_DEFINE_ITEMS, originStateCode, transitionNumber, ANIMATION_DELAY);
              isFirst = false;
            } else {
              fprintf (dfaLdaFilePt, "apague %s%d_%d juntos apos %.2f segundos por %.2f segundos;\n", ID_DEFINE_ITEMS, originStateCode, transitionNumber, ANIMATION_DELAY, ANIMATION_DELAY);
            }
          }
          /* Transition */
          fprintf (dfaLdaFilePt, "\nmostre \"%s%d\" apos %.2f segundos;\n\n", ID_LABEL_TRANSITION, transitionNumber, ANIMATION_DELAY);          
        }
        /* Print unvisited states */
        if (! visitedStates[currentStateCode-1]) {
          /* Set current state to visited */
          visitedStates[currentStateCode-1] = true;
          /* Define all items on state */
          /* fprintf (dfaLdaFilePt, "defina %s%d \"a_%s%d_%s.*\";\n", ID_DEFINE_ITEMS, currentStateCode, ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM); */
          /* Show state box */
          fprintf (dfaLdaFilePt, "mostre \"%s%d\" apos tecla;\n", ID_LABEL_STATE, currentStateCode);
          /* Show title */
          fprintf (dfaLdaFilePt, "mostre \"a_%s%d%s\" apos tecla;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_TITLE);
          /* Show items set one by one */
          /* fprintf (dfaLdaFilePt, "mostre %s%d umAum;\n\n", ID_DEFINE_ITEMS, currentStateCode); */
          /* Building data structure for the animation to be able to present the closing of states */
          int
            itemsDisplayed[dfa_stateCode2totItems (currentStateCode)];
          unsigned int
            dfaNonTerminalItems[dfa_stateCode2totItems (currentStateCode)],
            dfaNonTerminalItemsEnd = 0,
            dfaNonTerminalItemsStart = 0,
            iDfaNonTerminalItems;
          for (iItem = 1; iItem <= dfa_stateCode2totItems (currentStateCode); iItem++){
            itemCode = dfa_stateCode2itemCode (currentStateCode, iItem);
            itemsDisplayed[iItem-1] = -1;
            if (isReductionItem (itemCode))
              continue;
            if (symbolCode2symbolType(itemCode2transitionSymbol (itemCode)) == t_nonTerminal){
              for (iDfaNonTerminalItems = 0; iDfaNonTerminalItems < dfaNonTerminalItemsEnd; iDfaNonTerminalItems++){
                if (itemCode2transitionSymbol (itemCode) == dfaNonTerminalItems[iDfaNonTerminalItems])
                  break;
              }
              if (iDfaNonTerminalItems == dfaNonTerminalItemsEnd)
                dfaNonTerminalItems[dfaNonTerminalItemsEnd++] = itemCode2transitionSymbol (itemCode);
            }
          }
          /* Show items */
          for (iItem = 1; iItem <= dfa_stateCode2totItems (currentStateCode); iItem++) {
            if(itemsDisplayed [iItem-1] <= 0){
              itemCode = dfa_stateCode2itemCode (currentStateCode, iItem);
              if (itemsDisplayed [iItem-1] == -1)
                fprintf (dfaLdaFilePt, "mostre \"a_%s%d_%s%d\" apos tecla;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM, iItem);
              itemsDisplayed [iItem-1] = 1;
              if (isReductionItem (itemCode))
                continue;
              if (symbolCode2symbolType(itemCode2transitionSymbol (itemCode)) != t_nonTerminal)
                continue;
              if (dfaNonTerminalItemsStart > dfaNonTerminalItemsEnd || itemCode2transitionSymbol (itemCode) != dfaNonTerminalItems[dfaNonTerminalItemsStart])
                continue;
              /* Blink items */
              for (iBlink = 0, isFirst = true; iBlink < ANIMATION_BLINKS; iBlink++) {
                if (isFirst) {
                  fprintf (dfaLdaFilePt, "apague\"a_%s%d_%s%d\" por %.2f segundos;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM, iItem, ANIMATION_DELAY);
                  isFirst = false;
                } else {
                  fprintf (dfaLdaFilePt, "apague \"a_%s%d_%s%d\" apos %.2f segundos por %.2f segundos;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM, iItem, ANIMATION_DELAY, ANIMATION_DELAY);
                }
              }
              /* Since the dot is before a non-terminal, the rules of that non-terminal are added with the point at the beginning */
              for (iItem2 = iItem+1; iItem2 <= dfa_stateCode2totItems (currentStateCode); iItem2++) {
                newItemCode = dfa_stateCode2itemCode (currentStateCode, iItem2);
                if(rulePos2symbolCode (itemCode2ruleNumber (newItemCode), 0) == dfaNonTerminalItems[dfaNonTerminalItemsStart]){
                  fprintf (dfaLdaFilePt, "mostre \"a_%s%d_%s%d\" apos tecla;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM, iItem2);
                  itemsDisplayed [iItem2-1] = 0;
                }
              }
              dfaNonTerminalItemsStart++;
            }
          }
          fprintf (dfaLdaFilePt, "\n");
          /* Queue transitions */
          for (iTransition = 1; iTransition <= dfa_stateCode2totTransitionsFromState (currentStateCode); iTransition++) {
            transitionNumber = dfa_stateCode2transitionNumber (currentStateCode, iTransition);
            queueEnd++;
            queue[queueEnd].transitionNumber = transitionNumber;
            queue[queueEnd].destStateNumber = dfa_stateCode2stateNumber (dfa_transitionNumber2destState (transitionNumber));
          }          
        }
        queueStart++;
      }
    }
  }
    
  /* That's all */
  fclose (dfaLdaFilePt);
  fflush (NULL);
}

void print_dfa_lda2 (void) {
  bool
    visitedStates[totDFAstates],
    isFirst;
  struct {
    unsigned int destStateNumber;
    unsigned int transitionNumber;
  }
    queue [totDFAtransitions];
  unsigned int
    queueStart,
    queueEnd,
    iState,
    iItem,
    iItem2,
    iTransition,
    transitionNumber,
    iBlink,
    currentStateNumber;
  t_stateCode
    currentStateCode,
    originStateCode;
  t_itemCode
    itemCode,
    newItemCode;
  
  /* Open lda file for writing */
  errno = 0;
  if ((strcpy (dfaLdaFileName, grammarFileName)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcpy (dfaLdaFileName ,\"%s\") failed", grammarFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((strcat (dfaLdaFileName, FILE_EXTENSION_DFA_LDA)) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "strcat (dfaLdaFileName ,\"%s\") failed", FILE_EXTENSION_DFA_LDA);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
  errno = 0;
  if ((dfaLdaFilePt = fopen (dfaLdaFileName, "w")) == NULL) {
    snprintf (ERROR_auxErrorMsg, ERROR_maxErrorMsgSize, "Cannot open \"%s\" for writing", dfaLdaFileName);
    ERROR_fatal_error (errno, progName, __func__, ERROR_auxErrorMsg);
  }
    
  /* Set all states to unvisited */
  for (iState = 0; iState < totDFAstates; iState++) 
    visitedStates[iState] = false;
  
  /* Print commands in lda file */
  for (iState = 1; iState <= totDFAstates; iState++) {
    if (! visitedStates[iState-1]) {
      queueStart = queueEnd = 0;
      queue[queueStart].destStateNumber = iState;
      queue[queueStart].transitionNumber = 1;
      /* Breadth-first search */
      while (queueEnd >= queueStart) {
        currentStateNumber = queue[queueStart].destStateNumber;
        currentStateCode   = dfa_stateNumber2stateCode (currentStateNumber);
        transitionNumber   = queue[queueStart].transitionNumber;
        /* Show transitions */
        if (queueStart != 0) {
          originStateCode = dfa_transitionNumber2originState (transitionNumber);
          /* Set items responsible for the transition */
          fprintf (dfaLdaFilePt, "defina %s%d_%d \"", ID_DEFINE_ITEMS, originStateCode, transitionNumber);
          /* Look for items from that state that consume the transition symbol */
          for (iItem = 1, isFirst = true; iItem <= dfa_stateCode2totItems (originStateCode); iItem++) {
            itemCode = dfa_stateCode2itemCode (originStateCode, iItem);
            if (isReductionItem (itemCode))
              continue;
            if (dfa_transitionNumber2symbol (transitionNumber) != itemCode2transitionSymbol (itemCode))
              continue;
            if (! isFirst)
              fprintf (dfaLdaFilePt, "|");
            fprintf (dfaLdaFilePt, "(a_%s%d_%s%d)",  ID_LABEL_STATE, originStateCode, ID_LABEL_ITEM, iItem);
            isFirst = false;
          }
          fprintf (dfaLdaFilePt, "\";\n");
          /* Blink items */
          for (iBlink = 0; iBlink < ANIMATION_BLINKS; iBlink++)
            fprintf (dfaLdaFilePt, "apague %s%d_%d juntos apos %.2f segundos por %.2f segundos;\n", ID_DEFINE_ITEMS, originStateCode, transitionNumber, ANIMATION_DELAY, ANIMATION_DELAY);
          /* Transition */
          fprintf (dfaLdaFilePt, "\nmostre \"%s%d\" apos %.2f segundos;\n\n", ID_LABEL_TRANSITION, transitionNumber, ANIMATION_DELAY);          
        }
        /* Print unvisited states */
        if (! visitedStates[currentStateCode-1]) {
          /* Set current state to visited */
          visitedStates[currentStateCode-1] = true;
          /* Define all items on state */
          /* fprintf (dfaLdaFilePt, "defina %s%d \"a_%s%d_%s.*\";\n", ID_DEFINE_ITEMS, currentStateCode, ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM); */
          /* Show state box */
          fprintf (dfaLdaFilePt, "mostre \"%s%d\" apos %.2f segundos;\n", ID_LABEL_STATE, currentStateCode, ANIMATION_DELAY);
          /* Show title */
          fprintf (dfaLdaFilePt, "mostre \"a_%s%d%s\" apos %.2f segundos;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_TITLE, ANIMATION_DELAY);
          /* Show items set one by one */
          /* fprintf (dfaLdaFilePt, "mostre %s%d umAum apos %.2f segundos;\n\n", ID_DEFINE_ITEMS, currentStateCode, ANIMATION_DELAY); */
          /* Building data structure for the animation to be able to present the closing of states */
          int
            itemsDisplayed[dfa_stateCode2totItems (currentStateCode)];
          unsigned int
            dfaNonTerminalItems[dfa_stateCode2totItems (currentStateCode)],
            dfaNonTerminalItemsEnd = 0,
            dfaNonTerminalItemsStart = 0,
            iDfaNonTerminalItems;
          for (iItem = 1; iItem <= dfa_stateCode2totItems (currentStateCode); iItem++){
            itemCode = dfa_stateCode2itemCode (currentStateCode, iItem);
            itemsDisplayed[iItem-1] = -1;
            if (isReductionItem (itemCode))
              continue;
            if (symbolCode2symbolType(itemCode2transitionSymbol (itemCode)) == t_nonTerminal){
              for (iDfaNonTerminalItems = 0; iDfaNonTerminalItems < dfaNonTerminalItemsEnd; iDfaNonTerminalItems++){
                if (itemCode2transitionSymbol (itemCode) == dfaNonTerminalItems[iDfaNonTerminalItems])
                  break;
              }
              if (iDfaNonTerminalItems == dfaNonTerminalItemsEnd)
                dfaNonTerminalItems[dfaNonTerminalItemsEnd++] = itemCode2transitionSymbol (itemCode);
            }
          }
          /* Show items */
          for (iItem = 1; iItem <= dfa_stateCode2totItems (currentStateCode); iItem++) {
            if(itemsDisplayed [iItem-1] <= 0){
              itemCode = dfa_stateCode2itemCode (currentStateCode, iItem);
              if (itemsDisplayed [iItem-1] == -1)
                fprintf (dfaLdaFilePt, "mostre \"a_%s%d_%s%d\" apos %.2f segundos;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM, iItem, ANIMATION_DELAY);
              itemsDisplayed [iItem-1] = 1;
              if (isReductionItem (itemCode))
                continue;
              if (symbolCode2symbolType(itemCode2transitionSymbol (itemCode)) != t_nonTerminal)
                continue;
              if (dfaNonTerminalItemsStart > dfaNonTerminalItemsEnd || itemCode2transitionSymbol (itemCode) != dfaNonTerminalItems[dfaNonTerminalItemsStart])
                continue;
              /* Blink items */
              for (iBlink = 0; iBlink < ANIMATION_BLINKS; iBlink++)
                fprintf (dfaLdaFilePt, "apague \"a_%s%d_%s%d\" apos %.2f segundos por %.2f segundos;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM, iItem, ANIMATION_DELAY, ANIMATION_DELAY);
              /* Since the dot is before a non-terminal, the rules of that non-terminal are added with the point at the beginning */
              for (iItem2 = iItem+1; iItem2 <= dfa_stateCode2totItems (currentStateCode); iItem2++) {
                newItemCode = dfa_stateCode2itemCode (currentStateCode, iItem2);
                if(rulePos2symbolCode (itemCode2ruleNumber (newItemCode), 0) == dfaNonTerminalItems[dfaNonTerminalItemsStart]){
                  fprintf (dfaLdaFilePt, "mostre \"a_%s%d_%s%d\" apos %.2f segundos;\n", ID_LABEL_STATE, currentStateCode, ID_LABEL_ITEM, iItem2, ANIMATION_DELAY);
                  itemsDisplayed [iItem2-1] = 0;
                }
              }
              dfaNonTerminalItemsStart++;
            }
          }
          fprintf (dfaLdaFilePt, "\n");
          /* Queue transitions */
          for (iTransition = 1; iTransition <= dfa_stateCode2totTransitionsFromState (currentStateCode); iTransition++) {
            transitionNumber = dfa_stateCode2transitionNumber (currentStateCode, iTransition);
            queueEnd++;
            queue[queueEnd].transitionNumber = transitionNumber;
            queue[queueEnd].destStateNumber = dfa_stateCode2stateNumber (dfa_transitionNumber2destState (transitionNumber));
          }
        }
        queueStart++;
      }
    }
  }
    
  /* That's all */
  fclose (dfaLdaFilePt);
  fflush (NULL);
}

/*
*-----------------------------------------------------------------------------
* Compile .lda file
*-----------------------------------------------------------------------------
*/

int compile_lda (const char * ldaFileName, const char * svgFileName) {
  char argStr[600] = "";
  sprintf (argStr, "%s -d LiDAS.mel -p %s %s", lidas, ldaFileName, svgFileName);
  printf ("%s\n", argStr);
  return (system (argStr));
}
