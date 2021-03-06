IDIR = ./include
CPP = g++
CXXFLAGS = -std=c++11 -I$(IDIR) -O2
DEBUG = -DDEBUG
LEX=flex
LIBLEX=-lfl
YACC=bison
BIN_DIR= bin
MKDIR_P= mkdir -p

.PHONY: dir

all : dir clean detail ./bin/resol ./bin/tests

debug : CXXFLAGS += -DDEBUG
debug : all

fast : CPP = clang++
fast : all


./bin/resol: ./solver/*.cpp ./sat/main.cpp  ./solver/expr/tseitin.cpp ./solver/expr/logical_expr.cpp ./solver/expr/detail/*.cpp ./solver/expr/theory_expr.cpp ./solver/expr/detail/y.tab.c ./solver/expr/detail/scanner.tab.cpp ./solver/detail/clause.cpp
	$(CPP) $(CXXFLAGS) -o $@ $^

./bin/tests: ./solver/*.cpp ./solver/expr/*.cpp ./solver/expr/detail/*.cpp ./solver/expr/detail/y.tab.c ./tests/*.cpp ./solver/expr/detail/scanner.tab.cpp ./solver/detail/clause.cpp
	$(CPP) $(CXXFLAGS) -o $@ $^


detail:  ./solver/expr/detail/scanner.tab.cpp ./solver/expr/detail/y.tab.c

./solver/expr/detail/scanner.tab.cpp: ./solver/expr/detail/scanner.lpp
	$(LEX) -o $@ $^

./solver/expr/detail/y.tab.c: ./solver/expr/detail/parser.ypp
	$(YACC) -o $@ $^

clean :
	rm -rf ./bin/resol
	rm -rf ./bin/tests

dir : $(BIN_DIR)

$(BIN_DIR):
	$(MKDIR_P) $@
