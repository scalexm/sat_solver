IDIR = ./include
CPP = g++
CXXFLAGS = -std=c++11 -I$(IDIR) 
DEBUG = -DDEBUG
LEX=flex
LIBLEX=-lfl
YACC=bison


all : clean detail resol ./tests/tests

debug : CXXFLAGS += -DDEBUG
debug : all

resol: ./solver/solver.cpp ./sat/main.cpp  ./solver/expr/tseitin.cpp ./solver/expr/logical_expr.cpp ./solver/expr/detail/*.cpp ./solver/expr/detail/y.tab.c
	$(CPP) $(CXXFLAGS) -o $@ $^

./tests/tests: ./solver/solver.cpp ./solver/expr/*.cpp ./solver/expr/detail/*.cpp ./solver/expr/detail/*.c ./tests/*.cpp
	$(CPP) $(CXXFLAGS) -o $@ $^


detail:  ./solver/expr/detail/logical_scanner.tab.cpp ./solver/expr/detail/y.tab.c

./solver/expr/detail/logical_scanner.tab.cpp: ./solver/expr/detail/logical_scanner.lpp
	$(LEX) -o $@ $^
	
./solver/expr/detail/y.tab.c: ./solver/expr/detail/logical_parser.ypp
	$(YACC) -o $@ $^

clean :
	rm -rf resol
	rm -rf ./tests/tests
