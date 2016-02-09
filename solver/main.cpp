//
//  main.cpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "solver.hpp"

#define MAKE_WARNING(lno, mess) ("warning line " + std::to_string(lno) + ": " + (mess))

solver parse(std::ifstream & file) {
    std::string line;
    int line_number = 0;
    bool header = false;
    int V = -1, C = -1;
    int clause_number = 0;
    std::vector<std::unordered_set<int>> res;

    while (std::getline(file, line)) {
        ++line_number;
        if(!line.empty() && line[0] == 'c')
            continue;
        else if(!line.empty() && line[0] == 'p') {
            if (header) {
                std::cout
                    << MAKE_WARNING(line_number, "redefinition of header ignored")
                    << std::endl;
            }

            std::string str1, str2;
            std::istringstream ss { line };
            if (!(ss >> str1 >> str2 >> V >> C) || str1 != "p" || str2 != "cnf")
                std::cout << MAKE_WARNING(line_number, "bad header `" + line + "`") << std::endl;
            else if (V < 0)
                std::cout << MAKE_WARNING(line_number, "negative V") << std::endl;
            else if (C < 0)
                std::cout << MAKE_WARNING(line_number, "negative C") << std::endl;
        } else {
            if (!header)
                std::cout << MAKE_WARNING(line_number, "header not defined") << std::endl;
            std::istringstream ss { line };

            int v;
            std::unordered_set<int> clause;
            while (ss >> v) {
                if (v == 0)
                    break;
                if (std::abs(v) > V) {
                    std::cout
                        << MAKE_WARNING(line_number, "out of range variable " + std::to_string(v))
                        << std::endl;
                }
                clause.emplace(v);
            }

            if (v != 0)
                std::cout << MAKE_WARNING(line_number, "line not terminated by 0") << std::endl;

            ++clause_number;
            if (C >= 0 && clause_number == C + 1)
                std::cout << MAKE_WARNING(line_number, "more clauses than announced") << std::endl;
            res.emplace_back(std::move(clause));
        }
    }

    return solver { std::move(res) };
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        std::cerr << "no input" << std::endl;
        return 1;
    } else if (std::string { argv[1] } == "-tseitin") {
        if (argc < 3) {
            std::cerr << "no input" << std::endl;
            return 1;
        }
        // TODO
    } else {
        std::ifstream f { argv[1] };
        if (!f) {
            std::cerr << "file not found" << std::endl;
            return 1;
        }

        if (parse(f).satisfiable())
            std::cout << "s SATISFIABLE" << std::endl;
        else
            std::cout << "s UNSATISFIABLE" << std::endl;
    }

    return 0;
}
