//
//  detail/clause.cpp
//  solver
//
//  Created by Alexandre Martin on 03/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "clause.hpp"
#include "solver.hpp"

namespace detail {
    int clause::first_unassigned(const assignment & a) const {
        for (auto && lit : m_litterals) {
            if (a[var(lit)].pol == polarity::VUNDEF)
                return lit;
        }
        return -1;
    }

    void clause::update_counts(const assignment & a, std::vector<size_t> & counts) const {
        for (auto && lit : m_litterals) {
            if (a[var(lit)].pol == polarity::VUNDEF)
                ++counts[lit];
        }
    }

    int detail::clause::make_new_watch(const assignment & a) {
        int found = -1;
        for (auto i = 2; i < m_litterals.size(); ++i) {
            if (polarity_lit(a, m_litterals[i]) != polarity::VFALSE) {
                std::swap(m_litterals[1], m_litterals[i]);
                found = m_litterals[1];
                break;
            }
        }
        return found;
    }
}