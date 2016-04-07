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
    m_assignment.resize(
        m_remaining_variables,
        detail::var_data { detail::polarity::VUNDEF, -1, nullptr }
    );
    m_watches.resize(2 * m_remaining_variables);

    // set up clauses
    m_remaining_clauses = 0;
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
                enqueue(first, 0, nullptr);
            continue;
        }

        add_clause(std::move(sat_clause));
    }
}

detail::clause* solver::add_clause(detail::clause clause) {
    clause.set_id(m_clauses.size());
    ++m_remaining_clauses;
    auto it = m_clauses.emplace(m_clauses.end(), std::move(clause));
    if (m_guess_mode == guess_mode::WL) {
        m_watches[(*it).watch_0()].emplace_back(&*it);
        m_watches[(*it).watch_1()].emplace_back(&*it);
    } else
        (*it).build_watches(m_watches);
    return &*it;
}

/* assume lit is TRUE, wether deduced or guessed, and push it on the valuation stack */
void solver::enqueue(int lit, int level, detail::clause * reason) {
    auto var = detail::var(lit);
    assert(m_assignment[var].pol == detail::polarity::VUNDEF);

#ifdef DEBUG
        std::cout << "enqueue " << new_to_old_lit(lit) << std::endl;
#endif

    m_assignment[var].pol =
        detail::sign(lit) ? detail::polarity::VTRUE : detail::polarity::VFALSE;
    m_assignment[var].level = level;
    m_assignment[var].reason = reason;
    m_valuation.emplace_back(lit);
    --m_remaining_variables;
}

/* removes the top of the valuation stack */
int solver::dequeue() {
    assert(!m_valuation.empty());

    int lit = m_valuation.back();
    m_valuation.pop_back();
    auto var = detail::var(lit);
    m_assignment[var].pol = detail::polarity::VUNDEF;
    m_assignment[var].level = -1;
    m_assignment[var].reason = nullptr;
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

            detail::clause learnt;
            if (can_learn()) {
                auto knowledge = learn(conflict, level);
                level = knowledge.second + 1;
                learnt = std::move(knowledge.first);
            }

            auto lit = backtrack(level);
            --level;
#ifdef DEBUG
            std::cout << "go back to level " << level << std::endl;
#endif

            if (can_learn()) {
                learnt.set_count(1);
                auto lit = learnt.first_unassigned(m_assignment);
                auto reason = learnt.litterals().size() != 1 ?
                    add_clause(std::move(learnt)) : nullptr;
                enqueue(lit, level, reason);
            } else
                enqueue(detail::neg(lit), level, nullptr);
        } else {
            ++level;
#ifdef DEBUG
            std::cout << "level " << level << std::endl;
#endif
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
#ifdef DEBUG
            std::cout << "guess " << new_to_old_lit(lit) << std::endl;
#endif
            enqueue(lit, level, nullptr);
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
            m_assignment[v].pol == detail::polarity::VTRUE ? true : false
        );
    }

    return result;
}
