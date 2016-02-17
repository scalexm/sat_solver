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


./bin/resol: ./solver/solver.cpp ./sat/main.cpp  ./solver/expr/tseitin.cpp ./solver/expr/logical_expr.cpp ./solver/expr/detail/*.cpp ./solver/expr/detail/y.tab.c
	$(CPP) $(CXXFLAGS) -o $@ $^

./bin/tests: ./solver/solver.cpp ./solver/expr/*.cpp ./solver/expr/detail/*.cpp ./solver/expr/detail/*.c ./tests/*.cpp
	$(CPP) $(CXXFLAGS) -o $@ $^


detail:  ./solver/expr/detail/logical_scanner.tab.cpp ./solver/expr/detail/y.tab.c

./solver/expr/detail/logical_scanner.tab.cpp: ./solver/expr/detail/logical_scanner.lpp
	$(LEX) -o $@ $^

./solver/expr/detail/y.tab.c: ./solver/expr/detail/logical_parser.ypp
	$(YACC) -o $@ $^

clean :
	rm -rf ./bin/resol
	rm -rf ./bin/tests

dir : $(BIN_DIR)

$(BIN_DIR):
	$(MKDIR_P) $@
