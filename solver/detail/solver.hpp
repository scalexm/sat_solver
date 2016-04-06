//
//  detail/solver.hpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef DETAIL_SOLVER_HPP
#define DETAIL_SOLVER_HPP

#include <cassert>
#include <queue>

namespace detail {
    enum class polarity {
        VTRUE,
        VFALSE,
        VUNDEF,
    };

    inline bool sign(int lit) {
        return lit & 1;
    }

    inline int var(int lit) {
        return lit >> 1;
    }

    inline int neg(int lit) {
        return lit ^ 1;
    }

    inline int lit(int var, bool sign) {
        return var + var + (sign ? 1 : 0);
    }

    inline polarity polarity_lit(const assignment & a, int lit) {
        auto var_polarity = a[var(lit)].polarity;
        if (var_polarity == polarity::VUNDEF)
            return polarity::VUNDEF;
        else if (sign(lit))
            return var_polarity;
        else
            return var_polarity == polarity::VTRUE ? polarity::VFALSE : polarity::VTRUE;
    }
}

#endif