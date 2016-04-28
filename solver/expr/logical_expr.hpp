/*
 *  expr/logical_expr.hpp
 *  solver
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#ifndef LOGICAL_EXPR_HPP
#define LOGICAL_EXPR_HPP

#include "../cnf.hpp"
#include "atom.hpp"
#include "expr.hpp"

namespace expr {
    template<class Tag>
    using logical_binary = detail::binary_<Tag, atom::variable>;

    using logical_not = detail::not_<atom::variable>;
    using logical_and = detail::and_<atom::variable>;
    using logical_or = detail::or_<atom::variable>;
    using logical_xor = detail::xor_<atom::variable>;
    using logical_impl = detail::impl_<atom::variable>;
    using logical_equiv = detail::equiv_<atom::variable>;
    using logical_expr = detail::expr_<atom::variable>;


    // helper function for convenience
    template<class T>
    inline logical_expr make(T exp) {
        return logical_expr { std::move(exp) };
    }

    bool eval(const logical_expr &, const valuation &);
    result<logical_expr> parse(const std::string &);
    logical_expr simplify(logical_expr);
    logical_expr cnf_to_expr(const cnf &);
}

#endif
