//
//  deduce.cpp
//  solver
//
//  Created by Alexandre Martin on 19/03/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "solver.hpp"
#include <iostream>

/* deduce for one litteral in default mode */
detail::clause * solver::deduce_one_default(int lit, size_t level) {
     detail::clause * conflict = nullptr;

    for (auto && c : m_watches[lit]) {
        if (c->satisfied_by() != -1)
            continue;
#ifdef DEBUG
        std::cout << "removing clause " << c->id() << std::endl;
#endif
        c->satisfy(lit);
        --m_remaining_clauses;
    }

    auto neg_lit = detail::neg(lit);
    for (auto && c : m_watches[neg_lit]) {
        if (c->satisfied_by() != -1)
            continue;
#ifdef DEBUG
        std::cout << "removing lit " << new_to_old_lit(neg_lit) << " from clause "
                  << c->id() << std::endl;
#endif
        c->decrease_count();
        if (c->count() == 0) // empty clause: conflict
            conflict = c;
        else if (c->count() == 1) { // unit clause: we enqueue the litteral for further deducing
            auto first = c->first_unassigned(m_assignment);
            if (first != -1)
                enqueue(first, true, level);
        }
    }

    return conflict;
}

/* deduce for one litteral in wl mode */
detail::clause * solver::deduce_one_wl(int lit, size_t level) {
    auto neg_lit = detail::neg(lit);
    auto it1 = m_watches[neg_lit].begin(), end = m_watches[neg_lit].end();
    auto it2 = it1;
    detail::clause * conflict = nullptr;

    for (; it2 != end;) {
        auto c = *it2;
#ifdef DEBUG
        std::cout << new_to_old_lit(neg_lit) << " is watched in clause " << c->id() << std::endl;
#endif
        ++it2;

        if (c->watch_0() == neg_lit)
            c->swap_watches();

        auto w0 = c->watch_0();
        auto w0_polarity = detail::polarity_lit(m_assignment, w0);

        // clause already satisfied
        if (w0_polarity == detail::polarity::VTRUE) {
            *it1++ = c;
            continue;
        }

        // else we search for a new watched litteral
        auto new_watch = c->make_new_watch(m_assignment);

        // can't find one: it is a unit clause
        if (new_watch == -1) {
            *it1++ = c;
            if (w0_polarity == detail::polarity::VFALSE) { // conflict
                conflict = c;
                while (it2 != end)
                    *it1++ = *it2++;
                break;
            }
            enqueue(w0, true, level);
        } else
            m_watches[new_watch].emplace_back(c);
    }

    // remove the watched clauses of the old watched litteral
    m_watches[neg_lit].erase(it1, it2);
    return conflict;
}

/* deduce until no more deductions can be done */
detail::clause * solver::deduce(size_t level) {
    detail::clause * conflict = nullptr;
    if (!m_valuation.empty()) {
        auto top = level == 0 ? 0 : m_valuation.size() - 1;
        while (top < m_valuation.size()) {
            auto c = (this->*m_deduce_one)(m_valuation[top].value(), level);
            if (c != nullptr)
                conflict = c;
            ++top;
        }
    }
    return conflict;
}