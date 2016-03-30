//
//  learn.cpp
//  solver
//
//  Created by Alexandre Martin on 30/03/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "solver.hpp"
#include "detail/conflict_graph.hpp"

detail::clause marry(const detail::clause & c1, const detail::clause & c2, int x) {
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

void solver::learn(const detail::clause &) {
}