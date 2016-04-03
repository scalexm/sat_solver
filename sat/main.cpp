//
//  main.cpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/solver.hpp"
#include "../solver/expr/tseitin.hpp"
#include <iostream>
#include "../solver/command_line.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>

#define MAKE_WARNING(lno, mess) ("warning line " + std::to_string(lno) + ": " + (mess))

solver parse(std::ifstream & file, guess_mode mode) {
    std::string line;
    int line_number = 0;
    bool header = false;
    int V = -1, C = -1;
    int clause_number = 0;
    std::vector<std::unordered_set<int>> res;

    while (std::getline(file, line)) {
        boost::algorithm::trim(line);
        ++line_number;
        if (line.empty())
            continue;
        if(line[0] == 'c')
            continue;
        else if(line[0] == 'p') {
            if (header) {
                std::cout
                    << MAKE_WARNING(line_number, "redefinition of header ignored")
                    << std::endl;
                continue;
            }
            header = true;

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

    return solver { std::move(res), mode };
}

void trim_expr(std::string & exp) {
    boost::algorithm::replace_all(exp, "\r\n", "\n"); // fuck windows
    boost::algorithm::trim(exp);
    if (boost::algorithm::ends_with(exp, " 0"))
        boost::algorithm::erase_tail(exp, 2);
    boost::algorithm::trim(exp);
}

int main(int argc, const char ** argv) {
    auto mode = guess_mode::LINEAR;
    bool tseitin = false;
    std::string file_name;

    parse_command_line(argc, argv, mode, tseitin, file_name);

    if (file_name.empty()) {
        std::cerr << "no input" << std::endl;
        return 1;
    }

    std::ifstream f { file_name };
    if (!f) {
        std::cerr << "file not found `" << file_name << "`" << std::endl;
        return 1;
    }

    bool sat;
    valuation val;
    if (tseitin) {
        std::ostringstream ss;
        ss << f.rdbuf();
        auto exp = ss.str();
        trim_expr(exp);

        auto res = expr::parse(exp);
        if (auto err = boost::get<std::string>(&res)) {
            std::cerr << *err << std::endl;
            return 1;
        }

        auto tseitin = expr::tseitin_transform(*boost::get<expr::logical_expr>(&res));
        solver s { std::move(tseitin.first), mode };
        val = expr::remove_trailing_variables(s.solve(), tseitin.second);
        sat = s.satisfiable();
    } else {
        auto s = parse(f, mode);
        val = s.solve();
        sat = s.satisfiable();
    }

    if (sat) {
        std::cout << "s SATISFIABLE" << std::endl;
        for (auto && v : val) {
            std::cout << (v.second ? v.first : -v.first) << " ";
        }
        std::cout << "0" << std::endl;
    }
    else
        std::cout << "s UNSATISFIABLE" << std::endl;

    return 0;
}
