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

namespace expr { namespace detail {
    // specialization for not_<equality> with atomic formula
    template<>
    inline std::ostream & operator <<(std::ostream & stream, const not_<atom::equality> & exp) {
        if (auto at = boost::get<atom::equality>(&exp.op))
            return stream << at->left << " != " << at->right;
        return stream << "~(" << exp.op << ")";
    }

    template<>
    inline std::ostream & operator <<(std::ostream & stream,
                                      const not_<atom::congruence_equality> & exp) {
        if (auto at = boost::get<atom::congruence_equality>(&exp.op))
            return stream << at->left << " != " << at->right;
        return stream << "~(" << exp.op << ")";
    }
    
} }

namespace expr {
    using equality_or = detail::or_<atom::equality>;
    using equality_and = detail::and_<atom::equality>;
    using equality_xor = detail::xor_<atom::equality>;
    using equality_not = detail::not_<atom::equality>;
    using equality_impl = detail::impl_<atom::equality>;
    using equality_equiv = detail::equiv_<atom::equality>;
    using equality_expr = detail::expr_<atom::equality>;

    template<class T>
    inline equality_expr make_equality(T exp) {
        return equality_expr { std::move(exp) };
    }

    result<equality_expr> parse_equality(const std::string &);

    using congruence_or = detail::or_<atom::congruence_equality>;
    using congruence_and = detail::and_<atom::congruence_equality>;
    using congruence_xor = detail::xor_<atom::congruence_equality>;
    using congruence_not = detail::not_<atom::congruence_equality>;
    using congruence_impl = detail::impl_<atom::congruence_equality>;
    using congruence_equiv = detail::equiv_<atom::congruence_equality>;
    using congruence_expr = detail::expr_<atom::congruence_equality>;

    template<class T>
    inline congruence_expr make_congruence(T exp) {
        return congruence_expr { std::move(exp) };
    }

    result<congruence_expr> parse_congruence(const std::string &);
}

#endif
