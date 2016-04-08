//
//  draw.cpp
//  solver
//
//  Created by Nicolas Levy on 08/04/2016.
//  Copyright © 2016 NicolasPierreLevy. All rights reserved.
//

#include "solver.hpp"
#include "detail/solver.hpp"
#include <list>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <boost/algorithm/string.hpp>

std::string node_name(int lit) {
    std::ostringstream name;
    if (lit < 0)
        name << "-X" << std::abs(lit);
    else
        name << "X" << std::abs(lit);
    return name.str();
}

void solver::draw(detail::clause * c, int level, int uip) {
    std::vector<std::string> edges;
    std::list<int> todo;
    std::ofstream graph { "conflict.dot" };
    graph << "digraph conflict_graph {\n";
    int current = 0;
    std::string dest;
    std::string source;

    dest = "conflict";
    for (auto && lit : c->litterals()) {
        source = node_name(new_to_old_lit(lit));
        if(m_assignment[detail::var(lit)].level == level) {
            todo.push_back(lit);
            graph << "    " << source << " -> " << dest <<";\n";
        }
        graph << "    " << "conflict [color=red];\n";
    }

    while (!todo.empty()) {
        current = todo.front();
        todo.pop_front();
        auto reason = m_assignment[detail::var(current)].reason;
        if (reason == nullptr)
            continue;
        dest = node_name(new_to_old_lit(current));
        for (auto && lit : reason->litterals()) {
            source = node_name(new_to_old_lit(lit));
            if (m_assignment[detail::var(lit)].level == level) {
                todo.push_back(lit);
                graph << "    " << source << " -> " << dest << ";\n";
            }
            else if (current == uip) {
                graph << "    " << source << " -> " << dest << ";\n";
                graph << "    " << source << " [color=purple];\n";
            }
        }
        if (current == uip)
            graph << "    " << dest << " [color=yellow];\n";
        else
            graph << "    " << dest << " [color=blue];\n";
    }
    graph << "}\n";
}

void solver::interac(detail::clause * c, int level, int uip) {
    std::cout << "CONFLICT: entering interactive mode" << std::endl;
    std::cout << "> ";
    std::string line;
    while (std::cin >> line) {
        boost::algorithm::trim(line);
        if (line == "g") {
            draw(c, level, uip);
            std::cout << "output conflict graph to conflict.dot" << std::endl;
        } else if (line == "c")
            break;
        else if (line == "t") {
            m_cdcl = cdcl_mode::NORMAL;
            break;
        } else
            std::cout << "unkown command `" << line << "`" << std::endl;
        std::cout << "> ";
    }
}