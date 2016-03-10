//
//  solver.hpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "cnf.hpp"
#include "detail/clause.hpp"
#include <vector>
#include <unordered_map>
#include <list>
#include <random>

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

enum class guess_mode {
    LINEAR,
    RAND,
    MOMS,
    DLIS,
};

class solver {
private:
    std::unordered_set<int> m_remaining_variables;
    std::vector<detail::clause> m_clauses;
    guess_mode m_guess_mode;

    std::mt19937 m_rng;
    //std::uniform_int_distribution<> m_dis;

    size_t m_remaining_clauses;
    std::unordered_map<int, std::unordered_set<size_t>> m_occurences;

    std::list<detail::litteral> m_valuation;

    bool deduce(detail::litteral lit);
    int backtrack();
    double calculate_score(int);
    int guess(size_t min_clause = 0);
public:
    solver() = default;
    solver(cnf, guess_mode mode = guess_mode::RAND);
    valuation solve();

    bool satisfiable() {
        if (m_remaining_clauses > 0)
            solve();
        return m_remaining_clauses != -1;
    }
};

#endif
