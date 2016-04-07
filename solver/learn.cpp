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
#include <tuple>

detail::clause solver::marry(const detail::clause & c1, const detail::clause & c2, int x) {
    std::unordered_set<int> new_lits;
    new_lits.reserve(c1.litterals().size() + c2.litterals().size());

    for (auto && l : c1.litterals()) {
        if (l != x)
            new_lits.insert(l);
    }

    for (auto && l : c2.litterals()) {
        if (l != x)
            new_lits.insert(l);
    }

    detail::clause new_clause { new_lits.size() };
    for (auto && l : new_lits)
        new_clause.emplace(l);
    return new_clause;
}

/*std::pair<detail::clause, int> solver::learn(detail::clause * conflict, int level) {
    std::vector<int> clause;
    clause.reserve(conflict->litterals().size() - 1);
    static std::vector<bool> already_seen(2 * m_assignment.size(), false);
    while (true) {
        auto count = 0;
    }
}*/

std::pair<detail::clause, int> solver::learn(detail::clause * conflict, int level) {
    detail::clause new_clause;
#ifdef DEBUG
    bool first_try = true;
#endif
    while (true) {
        auto count = 0;
        int lit = -1;
        int max_level = -1;
        for (auto && l : conflict->litterals()) {
            auto l_level = m_assignment[detail::var(l)].level;
            if (l_level == level) {
                ++count;
                if (m_assignment[detail::var(l)].reason != nullptr)
                    lit = l;
            } else if (l_level > max_level)
                max_level = l_level;
        }
        if (count == 1) {
#ifdef DEBUG
            assert(!first_try);
#endif
            if (new_clause.litterals().size() == 1)
                max_level = 0;
            return std::make_pair(std::move(new_clause), max_level);
        }
#ifdef DEBUG
        first_try = false;
#endif
        assert(lit != -1);
        auto reason = m_assignment[detail::var(lit)].reason;
        new_clause = marry(*conflict, *reason, lit);
        conflict = &new_clause;
    }

    // not reached
    return std::make_pair(std::move(new_clause), -1);
}