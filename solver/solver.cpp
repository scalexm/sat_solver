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
    for (auto i = 0; i < clauses.size(); ++i) {
        if (clauses[i].empty()) {
            m_remaining_clauses = -1;
            return;
        }
        for (auto && l : clauses[i]) {
            m_remaining_variables.emplace(std::abs(l)); // to keep track of variables we haven't tried yet
            m_occurences[l].emplace(i); // to know in O(1) in which clauses each litteral appear
        }
        m_clauses.emplace_back(std::move(clauses[i]));
    }
    m_remaining_clauses = m_clauses.size(); // the total number of clauses not yet satisfied
}

bool solver::deduce(detail::litteral lit) {
    // adding lit to our valuation stack and removing |lit| from the remaining variables
    int value = lit.value();
    m_valuation.emplace_back(std::move(lit));
    m_remaining_variables.erase(m_remaining_variables.find(std::abs(value)));

    // we mark all clauses containing lit as satisfied
    for (auto && c : m_occurences[value]) {
        if (m_clauses[c].is_satisfied() != 0)
            continue;
#ifdef DEBUG
        std::cout << "removing clause " << c << std::endl;
#endif
        --m_remaining_clauses;
        m_clauses[c].satisfy(value);
    }

    // we remove -lit from other clauses
    for (auto && c : m_occurences[-value]) {
        if (m_clauses[c].is_satisfied() != 0)
            continue;
#ifdef DEBUG
        std::cout << "removing " << (-value) << " from clause " << c << std::endl;
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
    m_remaining_variables.emplace(std::abs(value));

    // clauses previously statisfied by a step with lit are now unsatisfied
    for (auto && c : m_occurences[value]) {
        if (m_clauses[c].is_satisfied() != value)
            continue;
#ifdef DEBUG
        std::cout << "adding back clause " << c << std::endl;
#endif
        ++m_remaining_clauses;
        m_clauses[c].satisfy(0);
    }

    // we add back -lit to corresponding clauses
    for (auto && c : m_occurences[-value]) {
        if (m_clauses[c].is_satisfied() != 0)
            continue;
#ifdef DEBUG
        std::cout << "adding back " << (-value) << " to clause " << c << std::endl;
#endif
        m_clauses[c].add(-value);
    }

    return value;
}

double solver::calculate_score(int v) {
    double score = 0;
    for (auto && c_id : m_occurences[v]) {
        auto && clause = m_clauses[c_id];
        if (clause.is_satisfied() != 0)
            continue;
        score += 1. / ((double) (1 << clause.litterals().size()));
    }
    return score;
}

int solver::guess(size_t min_clause) {
    switch (m_guess_mode) {
        case guess_mode::RAND: {
            auto begin = m_remaining_variables.begin();
            std::uniform_int_distribution<> dis(0, m_remaining_variables.size() - 1);
            auto offset = dis(m_rng);
            for (auto i = 0; i < offset; ++i)
                ++begin;
            return *begin;
        }
        case guess_mode::MOMS: {
            std::map<int, size_t> counts;
            for (auto && c : m_clauses) {
                if (c.is_satisfied() != 0)
                    continue;
                else if (c.litterals().size() == min_clause) {
                    for (auto && lit : c.litterals()) {
                        if (counts.find(lit) == counts.end())
                            counts[lit] = 1;
                        else
                            counts[lit] += 1;
                    }
                }
            }
            return counts.begin()->first;
        }
        case guess_mode::DLIS: {
            int max_lit = 0;
            double max_score = 0;
            for (auto && v : m_remaining_variables) {
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
            return *m_remaining_variables.begin();
        }
    }
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

        size_t min_clause = m_remaining_variables.size();
        if (!found) {
            /* we start by searching a necessary truth */

            for (auto && c : m_clauses) {
                if (c.is_satisfied() != 0)
                    continue;

                auto size = c.litterals().size();
                if (size <= min_clause)
                    min_clause = size;

                if (size == 1) {
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

#ifdef DEBUG
        std::cout << "remaining clauses: " << not_satisfied_count << std::endl;
#endif

        if (!found) {
            /* if we haven't found any, we make a guess */
            int val = guess(min_clause);
#ifdef DEBUG
            std::cout << "guessing " << val << std::endl;
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
            std::cout << "forcing " << -value << std::endl;
#endif
            lit = detail::litteral { -value, true };
        } else // no conflict encountered, we can search for a new litteral
            found = false;
    }


    /* compute a mapping int -> bool out of our valuation stack */

    std::unordered_map<int, bool> result;
    for (auto && v : m_valuation)
        result.emplace(std::abs(v.value()), v.value() > 0);
    for (auto && v : m_remaining_variables)
        result.emplace(v, false);

    return result;
}