//
//  backtrack.cpp
//  solver
//
//  Created by Alexandre Martin on 19/03/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "solver.hpp"
#include "detail/solver.hpp"
#include <iostream>

/* backtrack in default mode */
void solver::backtrack_one_default(int val) {
    for (auto && c : m_watches[val]) {
        if (c->satisfied_by() != val)
            continue;
#ifdef DEBUG
        std::cout << "adding back clause " << c->id() << std::endl;
#endif
        c->satisfy(-1);
        ++m_remaining_clauses;
    }

    auto neg_val = detail::neg(val);
    for (auto && c : m_watches[neg_val]) {
        if (c->satisfied_by() != -1)
            continue;
#ifdef DEBUG
        std::cout << "adding back lit " << new_to_old_lit(neg_val) << " to clause "
                  << c->id() << std::endl;
#endif
        c->increase_count();
    }
}

/* backtrack in wl mode */
void solver::backtrack_one_wl(int val) {
}

int solver::backtrack(int level) {
    int lit;
    assert(!m_valuation.empty());
    while (!m_valuation.empty() && m_assignment[detail::var(m_valuation.back())].level >= level) {
        lit = dequeue();
        (this->*m_backtrack_one)(lit);
    }
    return lit;
}