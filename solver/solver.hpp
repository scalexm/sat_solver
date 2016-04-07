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
#include <list>

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
    detail::clause * (solver::*m_deduce_one)(int, int) = nullptr;
    int (solver::*m_guess)(size_t) = nullptr;
    bool m_first_propagation_round = true;

    std::vector<int> m_old_variables;

    size_t m_remaining_variables;

    std::list<detail::clause> m_clauses;
    guess_mode m_guess_mode;
    cdcl_mode m_cdcl;

    std::mt19937 m_rng;

    // the total number of clauses not yet satisfied
    size_t m_remaining_clauses;

    std::vector<std::vector<detail::clause *>> m_watches;

    // valuation stack
    std::vector<int> m_valuation;

    // current assignment of variables
    detail::assignment m_assignment;

    void enqueue(int, int, detail::clause *);
    int dequeue();

    detail::clause * deduce_one_wl(int, int);
    detail::clause * deduce_one_default(int, int);
    detail::clause * deduce(int);

    void backtrack_one_wl(int);
    void backtrack_one_default(int);
    int backtrack(int);

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

    detail::clause marry(const detail::clause &, const detail::clause &, int);
    std::pair<detail::clause, int> learn(detail::clause *, int);
    detail::clause * add_clause(detail::clause);
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
