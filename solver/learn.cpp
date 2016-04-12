//
//  learn.cpp
//  solver
//
//  Created by Alexandre Martin on 30/03/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "solver.hpp"
#include "detail/solver.hpp"
#include "detail/conflict_graph.hpp"

std::pair<detail::clause, int> solver::learn(detail::clause * reason, int level) {
    std::vector<int> clause(1);

    auto index = m_valuation.size();
    int count = 0;
    auto backtrack_level = 0;
    int uip = -1;

    do {
        --index;

        if (reason->learnt())
            reason->inc_score();

        // marry reason with current learnt clause and count number of litterals assigned at current level
        for (auto && l : reason->litterals()) {
            auto var = detail::var(l);
            if (var != uip && !m_already_seen[var]) {
                m_already_seen[var] = true;
                auto l_level = m_assignment[var].level;
                if (l_level == level)
                    ++count;
                else {
                    if (m_options.guess == guess_mode::VSIDS) {
                        m_vsids_score[var].first += 1.0;
                        if (m_vsids_score[var].second != vsids_heap::handle_type { })
                            m_vsids_heap.increase(m_vsids_score[var].second);
                    }

                    clause.push_back(l);
                    if (l_level > backtrack_level) {
                        backtrack_level = l_level;

                        // litteral at next highest level should be watched because we will backtrack at this level
                        std::swap(clause[1], clause[clause.size() - 1]);
                    }
                }
            }
        }

        uip = detail::var(m_valuation[index]);
        while (!m_already_seen[uip]) { // find most recent litteral assigned at current level (*** bonus rendu 3 ***)
            --index;
            uip = detail::var(m_valuation[index]);
        }
        m_already_seen[uip] = false;
        --count;
        reason = m_assignment[uip].reason; // next clause to marry
    } while (count > 0);

    clause[0] = detail::neg(m_valuation[index]); // UIP!!!

    // don't forget to clear m_already_seen (it is not reallocated each time, so we have to clear it)
    for (auto && l : clause)
        m_already_seen[detail::var(l)] = false;

    if (clause.size() == 1)
        backtrack_level = 0;

    return std::make_pair(detail::clause { std::move(clause) }, backtrack_level);
}