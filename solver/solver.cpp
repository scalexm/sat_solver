//
//  solver.cpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "solver.hpp"
#include "detail/solver.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>

cnf remove_tautologies(const cnf & formula) {
    cnf ret;
    for (auto && clause : formula) {
        bool to_add = true;
        if (clause.empty()) {
            ret.push_back(std::unordered_set<int> { });
        }
        for (auto && x : clause) {
            if (clause.find(-x) != clause.end()) {
                to_add = false;
                break;
            }
        }
        if (to_add)
            ret.push_back(clause);
    }
    return ret;
}

solver::solver(cnf clauses, guess_mode mode, cdcl_mode cdcl) : m_guess_mode { mode },
                                                               m_cdcl { cdcl },
                                                               m_rng { std::random_device {}() } {
    if (m_guess_mode == guess_mode::WL) {
        m_backtrack_one = &solver::backtrack_one_wl;
        m_deduce_one = &solver::deduce_one_wl;
    } else {
        m_backtrack_one = &solver::backtrack_one_default;
        m_deduce_one = &solver::deduce_one_default;
    }

    switch (m_guess_mode) {
        case guess_mode::DLIS:
            m_guess = &solver::guess_dlis;
            break;
        case guess_mode::RAND:
            m_guess = &solver::guess_rand;
            break;
        case guess_mode::MOMS:
            m_guess = &solver::guess_moms;
            break;
        default:
            m_guess = &solver::guess_linear;
    }

    clauses = remove_tautologies(clauses);

    std::unordered_map<int, int> old_to_new;

    // rename variables so that they are consecutive integers
    for (auto && clause : clauses) {
        for (auto && l : clause) {
            auto abs_l = std::abs(l);
            if (old_to_new.find(abs_l) == old_to_new.end()) {
                old_to_new[abs_l] = (int) m_old_variables.size();
                m_old_variables.push_back(abs_l);
            }
        }
    }

    m_remaining_variables = m_old_variables.size();
    m_valuation.reserve(m_remaining_variables);
    m_assignment.resize(m_remaining_variables, std::make_pair(detail::polarity::VUNDEF, -1));
    m_watches.resize(2 * m_remaining_variables);

    // set up clauses and apply a first round of unit propagation in case of wl
    for (auto && cnf_clause : clauses) {
        detail::clause sat_clause;
        bool sat = false;

        for (auto && l : cnf_clause) {
            auto lit = detail::lit(old_to_new[std::abs(l)], l > 0);
            auto polarity = detail::polarity_lit(m_assignment, lit);
            if (polarity == detail::polarity::VUNDEF)
                sat_clause.emplace(lit);
            else if (polarity == detail::polarity::VTRUE) {
                sat = true;
                break;
            }
        }

        // clause has been already satisfied by previous unit propagation
        if (sat)
            continue;

        if (sat_clause.count() == 0) { // we have found a conflicting clause
            m_remaining_clauses = -1;
            return;
        } else if (sat_clause.count() == 1) { // unit clause => propagation
            auto first = sat_clause.first_unassigned(m_assignment);
            if (first != -1)
                enqueue(first, true, 0);
            continue;
        }

        sat_clause.set_id(m_clauses.size());
        m_clauses.emplace_back(std::move(sat_clause));
    }

    // set up watches now that m_clauses is fully allocated
    for (auto && sat_clause : m_clauses) {
        if (m_guess_mode == guess_mode::WL) {
            m_watches[sat_clause.watch_0()].emplace_back(&sat_clause);
            m_watches[sat_clause.watch_1()].emplace_back(&sat_clause);
        } else
            sat_clause.build_watches(m_watches);
    }

    m_remaining_clauses = m_clauses.size();
}

/* assume lit is TRUE, wether deduced or guessed, and push it on the valuation stack */
void solver::enqueue(int lit, bool force, int level) {
    auto var = detail::var(lit);
    assert(m_assignment[var].first == detail::polarity::VUNDEF);

#ifdef DEBUG
    if (force)
        std::cout << "forcing " << new_to_old_lit(lit) << std::endl;
    else
        std::cout << "guessing " << new_to_old_lit(lit) << std::endl;
#endif

    m_assignment[var].first =
        detail::sign(lit) ? detail::polarity::VTRUE : detail::polarity::VFALSE;
    m_assignment[var].second = level;
    m_valuation.emplace_back(lit);
    --m_remaining_variables;
}

/* removes the top of the valuation stack */
int solver::dequeue() {
    assert(!m_valuation.empty());

    int lit = m_valuation.back();
    m_valuation.pop_back();
    auto var = detail::var(lit);
    m_assignment[var].first = detail::polarity::VUNDEF;
    m_assignment[var].second = -1;
    ++m_remaining_variables;
    return lit;
}

int solver::new_to_old_lit(int lit) {
    auto old = m_old_variables[detail::var(lit)];
    return detail::sign(lit) ? old : -old;
}

valuation solver::solve() {
    if (m_remaining_clauses == -1) // not satisfiable
        return { { } };

    int level = 0;

    while (m_remaining_clauses > 0) {
        auto conflict = deduce(level);
        if (conflict != nullptr) {
#ifdef DEBUG
            std::cout << "conflict" << std::endl;
#endif
            if (level == 0) {
                m_remaining_clauses = -1;
                return { { } };
            }

            if (can_learn())
                learn(*conflict);
            auto lit = backtrack(level);
            --level;

            enqueue(detail::neg(lit), true, level);
        } else {
            ++level;
            // we have a full valuation
            if (m_remaining_variables == 0 || m_remaining_clauses == 0)
                break;

            size_t min_clause = 0;
            if (m_guess_mode == guess_mode::MOMS) {
                min_clause = m_remaining_variables;
                for (auto && c : m_clauses) {
                    if (c.satisfied_by() != -1)
                        continue;

                    auto size = c.count();
                    if (size <= min_clause)
                        min_clause = size;
                }
            }

            auto lit = (this->*m_guess)(min_clause);
            enqueue(lit, false, level);
        }
    }

    m_remaining_clauses = 0;


    /* compute a mapping int -> bool out of our valuation stack */

#ifdef DEBUG
    std::cout << "satisfiable" << std::endl;
#endif

    std::unordered_map<int, bool> result;

    for (auto v = 0; v < m_assignment.size(); ++v) {
        result.emplace(
            m_old_variables[v],
            m_assignment[v].first == detail::polarity::VTRUE ? true : false
        );
    }

    return result;
}
