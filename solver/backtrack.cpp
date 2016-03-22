//
//  backtrack.cpp
//  solver
//
//  Created by Alexandre Martin on 19/03/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "solver.hpp"
#include <iostream>

/* backtrack in default mode */
detail::litteral solver::backtrack_one_default() {
    auto lit = dequeue();
    auto val = lit.value();

    if (val == -1)
        return lit;

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

    return lit;
}

/* backtrack in wl mode */
detail::litteral solver::backtrack_one_wl() {
    return dequeue();
}

detail::litteral solver::backtrack() {
    auto lit = (this->*m_backtrack_one)();
    while (lit.forced())
        lit = (this->*m_backtrack_one)();
    return lit;
}