//
//  solver.cpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "solver.hpp"
#include <cstdlib>
#include <iostream>
#include <map>

cnf remove_tautologies(const cnf & formula) {
    cnf ret;
    for(auto && clause : formula) {
        bool to_add = true;
        if (clause.empty()) {
            ret.push_back(std::unordered_set<int>());
        }
        for(auto && x : clause) {
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

solver::solver(cnf clauses, guess_mode mode) : m_guess_mode { mode },
                                               m_rng { std::random_device {}()} {
    clauses = remove_tautologies(clauses);

    std::unordered_map<int, int> old_to_new;
    int count = 0;

    for (auto && clause : clauses) {
        if (clause.empty()) {
            m_remaining_clauses = -1;
            return;
        }

        for (auto && l : clause) {
            auto abs_l = std::abs(l);
            if (old_to_new.find(abs_l) == old_to_new.end()) {
                ++count;
                old_to_new[abs_l] = count;
            }
        }
    }

    m_valuation.reserve(count);
    m_variables.resize(count + 1, true); // index 0 is not used
    m_remaining_variables = count;
    m_old_variables.resize(count + 1);
    m_occurences.resize(2 * count);

    for (auto i = 0; i < clauses.size(); ++i) {
        detail::litterals_container litterals(count);
        for (auto && l : clauses[i]) {
            auto abs_l = std::abs(l);
            int new_ind = old_to_new[abs_l];
            auto new_lit = l > 0 ? new_ind : -new_ind;

            m_occurences[detail::lit_to_ind(new_lit)].push_back(i);
            m_old_variables[new_ind] = abs_l;
            litterals.emplace(new_lit);
        }
        m_clauses.emplace_back(std::move(litterals));
    }

    m_remaining_clauses = m_clauses.size();
}

bool solver::deduce(detail::litteral lit) {
    // adding lit to our valuation stack and removing |lit| from the remaining variables
    int value = lit.value();
    m_valuation.emplace_back(std::move(lit));
    m_variables[std::abs(value)] = false;
    --m_remaining_variables;

    // we mark all clauses containing lit as satisfied
    auto ind = detail::lit_to_ind(value);
    for (auto && c : m_occurences[ind]) {
        if (m_clauses[c].is_satisfied() != 0)
            continue;
#ifdef DEBUG
        std::cout << "removing clause " << c << std::endl;
#endif
        --m_remaining_clauses;
        m_clauses[c].satisfy(value);
    }

    // we remove -lit from other clauses
    ind = detail::lit_to_ind(-value);
    for (auto && c : m_occurences[ind]) {
        if (m_clauses[c].is_satisfied() != 0)
            continue;
#ifdef DEBUG
        std::cout << "removing " << -new_to_old_value(value) << " from clause " << c << std::endl;
#endif
        m_clauses[c].remove(-value);
        if (m_clauses[c].empty())
            return false;
    }

    return true;
}

int solver::backtrack() {
    // removing the head of our valuation stack, and pushing it back to the remaining variables
    auto value = m_valuation.back().value();
    m_valuation.pop_back();
    m_variables[std::abs(value)] = true;
    ++m_remaining_variables;

    // clauses previously statisfied by a step with lit are now unsatisfied
    auto ind = detail::lit_to_ind(value);
    for (auto && c : m_occurences[ind]) {
        if (m_clauses[c].is_satisfied() != value)
            continue;
#ifdef DEBUG
        std::cout << "adding back clause " << c << std::endl;
#endif
        ++m_remaining_clauses;
        m_clauses[c].satisfy(0);
    }

    // we add back -lit to corresponding clauses
    ind = detail::lit_to_ind(-value);
    for (auto && c : m_occurences[ind]) {
        if (m_clauses[c].is_satisfied() != 0)
            continue;
#ifdef DEBUG
        std::cout << "adding back " << -new_to_old_value(value) << " to clause " << c << std::endl;
#endif
        m_clauses[c].add(-value);
    }

    return value;
}

double solver::calculate_score(int v) {
    double score = 0;
    auto ind = detail::lit_to_ind(v);
    for (auto && c_id : m_occurences[ind]) {
        auto && clause = m_clauses[c_id];
        if (clause.is_satisfied() != 0)
            continue;
        score += 1. / ((double) (1 << clause.size()));
    }
    return score;
}

int solver::guess(size_t min_clause) {
    assert(m_remaining_variables != 0);

    switch (m_guess_mode) {
        case guess_mode::RAND: {
            std::uniform_int_distribution<> dis(0, (int) m_remaining_variables - 1);
            auto offset = dis(m_rng);

            auto begin = 0;
            for (auto v = 1; v < m_variables.size(); ++v) {
                if (!m_variables[v])
                    continue;
                else if (begin == offset)
                    return v;
                else
                    ++begin;
            }

            return 0; // not reached
        }

        case guess_mode::MOMS: {
            std::vector<size_t> counts(2 * (m_variables.size() - 1), 0);
            for (auto && clause : m_clauses) {
                if (clause.is_satisfied() != 0)
                    continue;
                else if (clause.size() == min_clause)
                    clause.update_counts(counts);
            }

            int i_max = 0;
            for (auto i = 1; i < counts.size(); ++i)
                if (counts[i] > counts[i_max])
                    i_max = i;
            return detail::ind_to_lit(i_max);
        }

        case guess_mode::DLIS: {
            int max_lit = 0;
            double max_score = 0;
            for (auto v = 1; v < m_variables.size(); ++v) {
                if (!m_variables[v])
                    continue;

                auto score = calculate_score(v);
                if (score >= max_score) {
                    max_lit = v;
                    max_score = score;
                }

                score = calculate_score(-v);
                if (score >= max_score) {
                    max_lit = -v;
                    max_score = score;
                }
            }
            return max_lit;
        }

        default: {
            for (auto v = 1; v < m_variables.size(); ++v)
                if (m_variables[v])
                    return v;
            return 0;
        }
    }
}

int solver::new_to_old_value(int value) {
    auto old = m_old_variables[std::abs(value)];
    return value > 0 ? old : -old;
}

valuation solver::solve() {
    if (m_remaining_clauses == -1) // not satisfiable
        return { { } };

    auto found = false;
    detail::litteral lit;

    while (m_remaining_clauses > 0) {
#ifdef DEBUG
        auto not_satisfied_count = 0;
        for (auto && c : m_clauses)
            if (c.is_satisfied() == 0)
                ++not_satisfied_count;
#endif

        size_t min_clause = m_remaining_variables;
        if (!found) {
            /* we start by searching a necessary truth */

            for (auto && c : m_clauses) {
                if (c.is_satisfied() != 0)
                    continue;

                auto size = c.size();
                if (size <= min_clause)
                    min_clause = size;

                if (size == 1) {
                    auto value = c.first();
#ifdef DEBUG
                    std::cout << "forcing " << new_to_old_value(value) << std::endl;
#endif
                    lit = detail::litteral { value, true };
                    found = true;
                    break;
                }
            }
        }

#ifdef DEBUG
        std::cout << "remaining clauses: " << not_satisfied_count << std::endl;
#endif

        if (!found) {
            /* if we haven't found any, we make a guess */
            int val = guess(min_clause);
#ifdef DEBUG
            std::cout << "guessing " << new_to_old_value(val) << std::endl;
#endif
            lit = detail::litteral { val, false };
            found = true;
        }

        /* we now try to deduce from our litteral set to true */
        if (!deduce(std::move(lit))) {
#ifdef DEBUG
            std::cout << "conflict" << std::endl;
#endif
            while (!m_valuation.empty() && m_valuation.back().forced())
                backtrack();

            if (m_valuation.empty()) {
                m_remaining_clauses = -1;
                return { { } };
            }

            auto value = backtrack();
#ifdef DEBUG
            std::cout << "forcing " << -new_to_old_value(value) << std::endl;
#endif
            lit = detail::litteral { -value, true };
        } else // no conflict encountered, we can search for a new litteral
            found = false;
    }


    /* compute a mapping int -> bool out of our valuation stack */

    std::unordered_map<int, bool> result;
    for (auto && v : m_valuation)
        result.emplace(m_old_variables[std::abs(v.value())], v.value() > 0);
    for (auto v = 1; v < m_variables.size(); ++v)
        if (m_variables[v])
            result.emplace(m_old_variables[v], false);

    return result;
}
