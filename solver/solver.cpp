//
//  instance.cpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "solver.hpp"
#include <cstdlib>
#include <iostream>

#define DEBUG

solver::solver(std::vector<std::unordered_set<int>> clauses) {
    for (auto i = 0; i < clauses.size(); ++i) {
        for (auto && l : clauses[i]) {
            m_remaining_variables.emplace(std::abs(l));
            m_occurences[l].emplace(i);
        }
        m_clauses.emplace_back(std::move(clauses[i]));
    }
    m_remaining_clauses = m_clauses.size();
}

bool solver::step(detail::litteral lit) {
    int value = lit.value();
    m_valuation.emplace_back(std::move(lit));
    m_remaining_variables.erase(m_remaining_variables.find(std::abs(value)));

    for (auto && c : m_occurences[value]) {
        if (m_clauses[c].is_satisfied() != 0)
            continue;
#ifdef DEBUG
        std::cout << "removing clause " << c << std::endl;
#endif
        --m_remaining_clauses;
        m_clauses[c].satisfy(value);
    }

    for (auto && c : m_occurences[-value]) {
#ifdef DEBUG
        std::cout << "removing " << (-value) << " from clause " << c << std::endl;
#endif
        m_clauses[c].remove(-value);
        if (m_clauses.empty())
            return false;
    }

    return true;
}

int solver::backtrack() {
    auto value = m_valuation.back().value();
    m_valuation.pop_back();
    m_remaining_variables.emplace(std::abs(value));

    for (auto && c : m_occurences[value]) {
        if (m_clauses[c].is_satisfied() != value)
            continue;
#ifdef DEBUG
        std::cout << "adding back clause " << c << std::endl;
#endif
        ++m_remaining_clauses;
        m_clauses[c].satisfy(0);
    }

    for (auto && c : m_occurences[-value]) {
#ifdef DEBUG
        std::cout << "adding back " << (-value) << "to clause " << c << std::endl;
#endif
        m_clauses[c].add(-value);
    }

    return value;
}

std::unordered_map<int, bool> solver::solve() {
    if (m_remaining_clauses == -1) // not satisfiable
        return { };

    auto found = false;
    detail::litteral lit;

    while (m_remaining_clauses > 0) {
        if (!found) {
            for (auto && c : m_clauses) {
                if (c.is_satisfied() != 0)
                    continue;
                else if (c.litterals().size() == 1) {
                    auto value = *c.litterals().begin();
#ifdef DEBUG
                    std::cout << "forcing " << value << std::endl;
#endif
                    lit = detail::litteral { value, true };
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            if (m_remaining_variables.empty()) {
#ifdef DEBUG
                std::cout << "empty clause" << std::endl;
#endif
                m_remaining_clauses = -1;
                return { };
            }

#ifdef DEBUG
            std::cout << "guessing " << *m_remaining_variables.begin() << std::endl;
#endif
            lit = detail::litteral { *m_remaining_variables.begin(), false };
            found = true;
        }

        if (!step(std::move(lit))) {
#ifdef DEBUG
            std::cout << "conflict" << std::endl;
#endif
            while (!m_valuation.empty() && m_valuation.back().forced())
                backtrack();

            if (m_valuation.empty()) {
                m_remaining_clauses = -1;
                return { };
            }

            auto value = backtrack();
#ifdef DEBUG
            std::cout << "forcing " << -value << std::endl;
#endif
            lit = detail::litteral { -value, true };
        } else
            found = false;
    }

    std::unordered_map<int, bool> result;
    for (auto && v : m_valuation)
        result.emplace(std::abs(v.value()), v.value() > 0);
    for (auto && v : m_remaining_variables)
        result.emplace(v, false);

    return result;
}