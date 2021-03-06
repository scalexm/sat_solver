//
//  guess.cpp
//  solver
//
//  Created by Alexandre Martin on 19/03/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "solver.hpp"
#include "detail/solver.hpp"

int solver::guess_linear() {
    for (auto v = 0; v < m_assignment.size(); ++v)
        if (m_assignment[v].pol == detail::polarity::VUNDEF)
            return detail::lit(v, true);
    return -1;
}

int solver::guess_rand() {
    std::uniform_int_distribution<> dis(0, (int) m_remaining_variables - 1);
    auto offset = dis(m_rng);

    auto begin = 0;
    for (auto v = 0; v < m_assignment.size(); ++v) {
        if (m_assignment[v].pol != detail::polarity::VUNDEF)
            continue;
        else if (begin == offset)
            return detail::lit(v, true);
        else
            ++begin;
    }

    return -1;
}

double solver::calculate_score(int lit) {
    double score = 0;
    for (auto && c : m_watches[lit]) {
        if (c->satisfied_by() != -1)
            continue;
        score += 1. / ((double) (1 << c->count()));
    }
    return score;
}

int solver::guess_dlis() {
    int max_lit = -1;
    double max_score = 0;
    for (auto v = 0; v < m_assignment.size(); ++v) {
        if (m_assignment[v].pol != detail::polarity::VUNDEF)
            continue;

        auto lit = detail::lit(v, true);
        auto score = calculate_score(lit);
        if (score >= max_score) {
            max_lit = lit;
            max_score = score;
        }

        lit = detail::lit(v, false);
        score = calculate_score(lit);
        if (score >= max_score) {
            max_lit = lit;
            max_score = score;
        }
    }
    return max_lit;
}

int solver::guess_moms() {
    for (auto & c : m_moms_counts)
        c = 0;

    for (auto && clause : m_clauses) {
        if (clause.satisfied_by() != -1)
            continue;
        else if (clause.count() == m_min_clause)
            clause.update_counts(m_assignment, m_moms_counts);
    }

    int lit_max = 0;
    for (auto lit = 1; lit < m_moms_counts.size(); ++lit)
        if (m_moms_counts[lit] > m_moms_counts[lit_max])
            lit_max = lit;
    return lit_max;
}

int solver::guess_vsids() {
    auto var = -1;
    while (var == -1 || m_assignment[var].pol != detail::polarity::VUNDEF) {
        var = m_vsids_heap.top().var();
        m_vsids_heap.pop();
        m_vsids_score[var].second = vsids_heap::handle_type { };
    }
    return detail::lit(var, true);
}