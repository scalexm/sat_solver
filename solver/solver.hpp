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
#include <random>

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
    WL,
};

enum class cdcl_mode {
    NONE,
    NORMAL,
    INTERACTIVE,
};

class solver {
private:
    void (solver::*m_backtrack_one)(int) = nullptr;
    detail::clause * (solver::*m_deduce_one)(int, size_t) = nullptr;
    int (solver::*m_guess)(size_t) = nullptr;

    std::vector<int> m_old_variables;

    size_t m_remaining_variables;

    std::vector<detail::clause> m_clauses;
    guess_mode m_guess_mode;
    cdcl_mode m_cdcl;

    std::mt19937 m_rng;

    // the total number of clauses not yet satisfied
    size_t m_remaining_clauses;

    std::vector<std::vector<detail::clause *>> m_watches;

    // valuation stack
    std::vector<detail::litteral> m_valuation;

    // current assignment of variables
    detail::assignment m_assignment;

    void enqueue(int, bool, size_t);
    detail::litteral dequeue();

    detail::clause * deduce_one_wl(int, size_t);
    detail::clause * deduce_one_default(int, size_t);
    detail::clause * deduce(size_t);

    void backtrack_one_wl(int);
    void backtrack_one_default(int);
    detail::litteral backtrack(size_t);

    int guess(size_t min_clause = 0);
    int new_to_old_lit(int);

    double calculate_score(int);
    int guess_linear(size_t);
    int guess_moms(size_t);
    int guess_dlis(size_t);
    int guess_rand(size_t);

    bool can_learn() const {
        return m_cdcl != cdcl_mode::NONE;
    }
    void learn(const detail::clause &);
public:
    solver() = default;
    solver(cnf, guess_mode mode = guess_mode::RAND, cdcl_mode cdcl = cdcl_mode::NONE);
    valuation solve();

    bool satisfiable() {
        if (m_remaining_clauses > 0)
            solve();
        return m_remaining_clauses != -1;
    }
};

#endif
