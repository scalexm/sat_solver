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

#include <set>
#include <sstream>
#include <fstream>

solver::solver(cnf clauses, options opt) : m_options { opt },
                                           m_rng { std::random_device {}() } {
    if (m_options.wl) {
        assert(m_options.guess != guess_mode::DLIS && m_options.guess != guess_mode::MOMS);
        m_backtrack_one = &solver::backtrack_one_wl;
        m_deduce_one = &solver::deduce_one_wl;
    } else {
        m_backtrack_one = &solver::backtrack_one_default;
        m_deduce_one = &solver::deduce_one_default;
    }

    switch (m_options.guess) {
        case guess_mode::DLIS:
            m_guess = &solver::guess_dlis;
            break;
        case guess_mode::RAND:
            m_guess = &solver::guess_rand;
            break;
        case guess_mode::MOMS:
            m_guess = &solver::guess_moms;
            break;
        case guess_mode::VSIDS:
            assert(can_learn());
            m_guess = &solver::guess_vsids;
            break;
        default:
            m_guess = &solver::guess_linear;
    }

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
    m_watches.resize(2 * m_assignment.size());

    if (can_learn())
        m_already_seen.resize(m_assignment.size(), false);

    if (m_options.guess == guess_mode::MOMS)
        m_moms_counts.resize(2 * m_assignment.size(), 0);

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

        auto count = sat_clause.count();
        if (count == 0) { // we have found a conflicting clause
            m_remaining_clauses = -1;
            return;
        } else if (count == 1) { // unit clause => propagation
            auto first = sat_clause.first_unassigned(m_assignment);
            if (first != -1)
                enqueue(first, 0, nullptr);
            continue;
        }

        add_clause(std::move(sat_clause));
    }
}

detail::clause * solver::add_clause(detail::clause clause) {
    clause.set_id(m_clauses.size());
    ++m_remaining_clauses;
    bool learnt = clause.learnt();
    auto it = m_clauses.emplace(m_clauses.end(), std::move(clause));
    if (learnt && m_options.forget)
        m_learnt.emplace_back(it);
    if (m_options.wl) {
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
    if (reason != nullptr)
        reason->inc_reason();
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
    if (m_assignment[var].reason != nullptr)
        m_assignment[var].reason->dec_reason();
    m_assignment[var].reason = nullptr;

    if (m_options.guess == guess_mode::VSIDS
        && m_vsids_score[var].second == vsids_heap::handle_type { }) {
        m_vsids_score[var].second = m_vsids_heap.push({ var, this });
    }

    ++m_remaining_variables;
    return lit;
}

int solver::new_to_old_lit(int lit) {
    auto old = m_old_variables[detail::var(lit)];
    return detail::sign(lit) ? old : -old;
}

void solver::remove_learnt(size_t count) {
#ifdef DEBUG
    std::cout << "size before forgetting: " << m_clauses.size() << std::endl;
#endif

    std::sort(m_learnt.begin(), m_learnt.end(), [](const clause_it & it1, const clause_it & it2) {
        return (*it1).score() < (*it2).score();
    });

    // remove clauses with lowest score, but don't remove clauses which are a reason for someone
    auto it = std::remove_if(m_learnt.begin(), m_learnt.end(), [&count, this](const clause_it & it) {
        if (count == 0 || (*it).is_reason())
            return false;
        --count;

        auto clause = &*it;

        // remove watches
        if (m_options.wl) {
            auto watch = clause->watch_0();
            m_watches[watch].erase(
                std::remove(m_watches[watch].begin(), m_watches[watch].end(), clause)
            );
            watch = clause->watch_1();
            m_watches[watch].erase(
                std::remove(m_watches[watch].begin(), m_watches[watch].end(), clause)
            );
        } else {
            for (auto && lit : clause->litterals()) {
                m_watches[lit].erase(
                    std::remove(m_watches[lit].begin(), m_watches[lit].end(), clause)
                );
            }
        }

        // remove clause
        if (clause->satisfied_by() == -1)
            --m_remaining_clauses;
        m_clauses.erase(it);

        return true;
    });

    m_learnt.erase(it, m_learnt.end());
#ifdef DEBUG
    std::cout << "size after forgetting: " << m_clauses.size() << std::endl;
#endif
}

valuation solver::solve() {
    if (m_remaining_clauses == -1) // not satisfiable
        return { { } };

    // solver has to be fully constructed in order to store pointer to `this` somewhere
    if (m_options.guess == guess_mode::VSIDS) {
        m_vsids_score.resize(m_assignment.size());
        for (auto v = 0; v < m_assignment.size(); ++v)
            m_vsids_score[v] = std::make_pair(0.0, m_vsids_heap.push({ v, this }));
    }

    int level = 0;
    auto conflict_nb = 0;
    auto max_learnt = m_clauses.size() / 3;

    while (m_first || m_remaining_clauses > 0) {
        m_first = false;
        auto conflict = deduce(level);
        if (conflict != nullptr) {
#ifdef DEBUG
            std::cout << "conflict" << std::endl;
#endif
            ++conflict_nb;

            if (level == 0) {
                m_remaining_clauses = -1;
                return { { } };
            }

            auto old_level = level;

            detail::clause learnt;
            if (conflict != &m_dummy_clause && can_learn()) {
                auto knowledge = learn(conflict, level);
                level = knowledge.second + 1;
                learnt = std::move(knowledge.first);
            }

            if (conflict_nb % 100 == 0 && m_options.guess == guess_mode::VSIDS) {
                for (auto && v : m_vsids_score)
                    v.first /= 2.0;
            }

            if (conflict != &m_dummy_clause && m_options.cdcl == cdcl_mode::INTERACTIVE)
                interac(conflict, old_level, learnt.watch_0());

            auto lit = backtrack(level);
            --level;
#ifdef DEBUG
            std::cout << "go back to level " << level << std::endl;
#endif

            if (conflict != &m_dummy_clause && can_learn()) {
                lit = learnt.watch_0();
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

            if (m_options.forget && m_learnt.size() > max_learnt)
                remove_learnt(m_learnt.size() / 2); // remove half of the learnt clauses

            if (m_options.guess == guess_mode::MOMS) {
                m_min_clause = 2 * m_remaining_variables;
                for (auto && c : m_clauses) {
                    if (c.satisfied_by() != -1)
                        continue;
                    auto size = c.count();
                    if (size <= m_min_clause)
                        m_min_clause = size;
                }
            }

            auto lit = (this->*m_guess)();
            assert(lit != -1);

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
