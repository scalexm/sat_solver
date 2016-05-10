//
//  theory_expr.cpp
//  solver
//
//  Created by Alexandre Martin on 28/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "theory_expr.hpp"
#include "detail/driver.hpp"

namespace expr {
    result<equality_expr> parse_equality(const std::string & str) {
        return detail::parse<atom::equality>(str, expr::detail::theory::EQUALITY);
    }

    result<congruence_expr> parse_congruence(const std::string & str) {
        return detail::parse<atom::congruence_equality>(str, expr::detail::theory::CONGRUENCE);
    }
}
