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
#include <boost/heap/d_ary_heap.hpp>

enum class guess_mode {
    LINEAR,
    RAND,
    MOMS,
    DLIS,
    VSIDS,
};

enum class cdcl_mode {
    NONE,
    NORMAL,
    INTERACTIVE,
};

struct options {
    guess_mode guess = guess_mode::LINEAR;
    cdcl_mode cdcl = cdcl_mode::NONE;
    bool wl = false;
    bool forget = false;
};

class solver {
private:
    void (solver::*m_backtrack_one)(int) = nullptr;
    detail::clause * (solver::*m_deduce_one)(int, int) = nullptr;
    int (solver::*m_guess)() = nullptr;
    bool m_first_propagation_round = true;
    options m_options;

    std::vector<int> m_old_variables;

    /* member so as to avoid allocations in clause learning
       (we can't bear the use of an unordered_set here, profiling has shown
       that it was less performant) */
    std::vector<bool> m_already_seen;

    // preallocated vector for MOMS
    std::vector<size_t> m_moms_counts;
    int m_min_clause;

    class heap_data {
    private:
        int m_var;
        solver * m_father;

    public:
        heap_data(int var, solver * father) : m_var { var }, m_father { father } { }

        int var() const {
            return m_var;
        }

        bool operator <(heap_data const & rhs) const {
            return m_father->m_vsids_score[m_var].first < m_father->m_vsids_score[rhs.m_var].first;
        }
    };

    using vsids_heap = boost::heap::d_ary_heap<
        heap_data,
        boost::heap::mutable_<true>,
        boost::heap::arity<2>
    >;

    vsids_heap m_vsids_heap;
    std::vector<std::pair<double, vsids_heap::handle_type>> m_vsids_score;

    size_t m_remaining_variables;

    std::list<detail::clause> m_clauses;
    guess_mode m_guess_mode;
    cdcl_mode m_cdcl;

    std::mt19937 m_rng;

    // the total number of clauses not yet satisfied
    int m_remaining_clauses;

    std::vector<std::vector<detail::clause *>> m_watches;

    // for clause removal
    using clause_it = std::list<detail::clause>::iterator;
    std::vector<clause_it> m_learnt;

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
    int guess_linear();
    int guess_moms();
    int guess_dlis();
    int guess_rand();
    int guess_vsids();

    bool can_learn() const {
        return m_options.cdcl != cdcl_mode::NONE;
    }

    void interac(detail::clause *, int, int);
    void draw(detail::clause *, int, int);

    std::pair<detail::clause, int> learn(detail::clause *, int);
    detail::clause * add_clause(detail::clause);
    void remove_learnt(size_t);
public:
    solver() = default;
    solver(cnf, options);
    valuation solve();

    bool satisfiable() {
        if (m_remaining_clauses > 0)
            solve();
        return m_remaining_clauses != -1;
    }
};

#endif
