//
//  expr/theory_expr.hpp
//  solver
//
//  Created by Alexandre Martin on 28/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef THEORY_EXPR_HPP
#define THEORY_EXPR_HPP

#include "expr.hpp"
#include "atom.hpp"

namespace expr {
    using equality_expr = detail::expr_<atom::equality>;

    template<class T>
    inline equality_expr make_equality(T exp) {
        return equality_expr { std::move(exp) };
    }

    result<equality_expr> parse_equality(const std::string &);
}

#endif
