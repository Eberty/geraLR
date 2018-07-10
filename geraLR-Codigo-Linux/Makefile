################################################################################################
#
# Projeto   : geraLR
# Arquivo   : Makefile
# Criado    : 2012-06-14
# Modificado: 2018-04-29
#
# COMO USAR:
#
# Na linha de comando, digite:
# - "make geraLR"   para compilar o executável geraLR a partir do código fonte
# - "make tests"    para compilar geraLR e processar todas as gramáticas no diretório corrente
# - "make valgrind" para compilar geraLR e processar todas as gramáticas com valgrind
# - "make all"      para fazer tudo isso acima
# - "make clean"    para remover todos os arquivos gerados por makes anteriores
#
################################################################################################

#------------------------------------------------------------------------------
# Set ANSI_STANDARD to 'yes' to use the "-ansi" flag.
# Set DEBUG_INFO  to 'yes' to generate symbolic info for debugging.
# Set INPUT_SUFFIX to the suffix of grammar files (input to geraLR).
# Set OUTPUT_SUFFIX to the suffix of output files (after processing by geraLR).
# Set VALGRIND_SUFFIX to the suffix of valgrind output files.
#------------------------------------------------------------------------------

ANSI_STANDARD   = yes
DEBUG_INFO      = no
INPUT_SUFFIX    = .grm
OUTPUT_SUFFIX   = .out
VALGRIND_SUFFIX = -valgrind.txt
NEED_MATH       = yes

#----------------------------------------------------------------------


#----------------------------------------------------------------------
# Set the shell, the C compiler and the rm shell command
#----------------------------------------------------------------------

SHELL = /bin/bash
CC = gcc
RM = rm -f

#----------------------------------------------------------------------
# Graphviz
#----------------------------------------------------------------------

LIB=-L/usr/lib/x86_64-linux-gnu/
INC=-I/usr/include/graphviz

#----------------------------------------------------------------------
# GCC flags
#----------------------------------------------------------------------

ifeq ($(DEBUG_INFO), yes)
  DEBUG = -g3
else
  DEBUG = -O2
endif

ifeq ($(ANSI_STANDARD), yes)
  ANSI = -ansi -std=c99
else
  ANSI = -std=c99
endif

ifeq ($(NEED_MATH), yes)
  MATH = -lm
else
  MATH = 
endif

WARNINGS = -Wall -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs
FLAGS = $(DEBUG) $(ANSI) $(WARNINGS) $(LIB) $(INC) -pedantic

#----------------------------------------------------------------------
# All source, object, grammar and output files in the current directory
#----------------------------------------------------------------------

SOURCES  := $(wildcard *.c)
OBJECTS  := $(SOURCES:%.c=%.o)
INPUTS   := $(wildcard *$(INPUT_SUFFIX))
OUTPUTS  := $(INPUTS:%$(INPUT_SUFFIX)=%$(OUTPUT_SUFFIX))
VALGRIND := $(INPUTS:%$(INPUT_SUFFIX)=%$(VALGRIND_SUFFIX))

#----------------------------------------------------------------------
# Executable flags
#----------------------------------------------------------------------

BINFLAGS = -w 0 -yndfoxlsLSNDA -C bege_marrom.cfg

# BINFLAGS = -w 0 -yndfoxlsLSNDA

#----------------------------------------------------------------------
# valgrind flags
#----------------------------------------------------------------------

#VALGRINDFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all --partial-loads-ok=yes -v
VALGRINDFLAGS = --leak-check=full --track-origins=yes --partial-loads-ok=yes -v

#----------------------------------------------------------------------
# The main make targets
#----------------------------------------------------------------------

BIN = geraLR
all: $(BIN) tests valgrind

#----------------------------------------------------------------------
#	Rule: Phony targets (to improve make's performance)
#----------------------------------------------------------------------

.PHONY: all clean

#----------------------------------------------------------------------
#	Rule: Default
#----------------------------------------------------------------------

.DEFAULT:
	@echo "Sorry, no rule for \"$@\" in Makefile"

#----------------------------------------------------------------------
#	Rule: Update and include dependency files 
#----------------------------------------------------------------------

include $(SOURCES:.c=.dep)

%.dep: %.c 
	@echo "Updating \"$@\" ..."
	@$(SHELL) -ec '$(CC) -MM $< | \
	sed '\''s/$*.o/& $@/g'\'' > $@'

#----------------------------------------------------------------------
#	Rule: Make all object files
#----------------------------------------------------------------------

$(OBJECTS): %.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

#----------------------------------------------------------------------
#	Rule: Make the executable
#----------------------------------------------------------------------

$(BIN): $(OBJECTS)
	$(CC) $(FLAGS) $^ -o $@ $(MATH) -lgvc -lcgraph -lcdt -lconfig

#----------------------------------------------------------------------
#	Rule: Run the executable with test files
#----------------------------------------------------------------------

tests: $(BIN) $(OUTPUTS) 

$(OUTPUTS): %$(OUTPUT_SUFFIX): %$(INPUT_SUFFIX)
	./$(BIN) $(BINFLAGS) $<

#----------------------------------------------------------------------
#	Rule: Run the executable with test files under valgrind
#----------------------------------------------------------------------

valgrind: $(VALGRIND)

$(VALGRIND): %$(VALGRIND_SUFFIX): %$(INPUT_SUFFIX)
	valgrind $(VALGRINDFLAGS) ./$(BIN) $(BINFLAGS) $< >& $@

#----------------------------------------------------------------------
#	Rule: Clear some stuff
#----------------------------------------------------------------------

clean:
	$(RM) $(BIN) *$(OUTPUT_SUFFIX) *$(VALGRIND_SUFFIX) *.o *.dep *tbl *.set *.gab *.sym *.dot *.svg *.lda *.tok *.mtk *.err *.html *~
