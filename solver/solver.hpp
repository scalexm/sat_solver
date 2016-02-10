//
//  solver.hpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "detail/clause.hpp"
#include <vector>
#include <unordered_map>
#include <list>

class solver;

namespace detail {
    class litteral {
    private:
        int m_value;
        bool m_forced;

    public:
        litteral() = default;
        litteral(int value, bool forced) : m_value { value }, m_forced { forced } { }

        bool forced() const { return m_forced; }
        int value() const { return m_value; }
    };
}

class solver {
private:
    std::unordered_set<int> m_remaining_variables;
    std::vector<detail::clause> m_clauses;

    size_t m_remaining_clauses;
    std::unordered_map<int, std::unordered_set<size_t>> m_occurences;

    std::list<detail::litteral> m_valuation;

    bool step(detail::litteral lit);
    int backtrack();
public:
    solver(std::vector<std::unordered_set<int>>);
    std::unordered_map<int, bool> solve();

    bool satisfiable() {
        if (m_remaining_clauses > 0)
            solve();
        return m_remaining_clauses != -1;
    }
};

#endif
