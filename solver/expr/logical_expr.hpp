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
#include <boost/variant.hpp>
#include <ostream>
#include <unordered_map>
#include <vector>
#include <unordered_set>

namespace expr {
    /*
        tags for the template struct logical_binary: each tag identifies a binary logical operator
    */
    struct and_tag {
        constexpr static const char * text = "/\\";
    };

    struct or_tag {
        constexpr static const char * text = "\\/";
    };

    struct xor_tag {
        constexpr static const char * text = "X";
    };

    struct impl_tag {
        constexpr static const char * text = "=>";
    };

    struct eq_tag {
        constexpr static const char * text = "<=>";
    };

    struct none { };
    using variable = int;

    struct logical_not;
    template<class T> struct logical_binary;
    using logical_and = logical_binary<and_tag>;
    using logical_or = logical_binary<or_tag>;
    using logical_xor = logical_binary<xor_tag>;
    using logical_impl = logical_binary<impl_tag>;
    using logical_eq = logical_binary<eq_tag>;

    using logical_expr = boost::variant<
        none, // none in first position for default ctor
        variable,
        boost::recursive_wrapper<logical_not>,
        boost::recursive_wrapper<logical_and>,
        boost::recursive_wrapper<logical_or>,
        boost::recursive_wrapper<logical_xor>,
        boost::recursive_wrapper<logical_impl>,
        boost::recursive_wrapper<logical_eq>
    >;

    /*
        used as a result of expr::parse: expr_result is either a logical_expr (success)
        or a string (error)
    */
    using expr_result = boost::variant<logical_expr, std::string>;

    template<class T>
    struct logical_binary {
        logical_expr op_left, op_right;
    };

    struct logical_not {
        logical_expr op;
    };

    inline bool operator ==(const none &, const none &) {
        return true;
    }

    inline bool operator ==(const logical_not & a, const logical_not & b) {
        return a.op == b.op;
    }

    template<class T>
    inline bool operator ==(const logical_binary<T> & a, const logical_binary<T> & b) {
        return a.op_left == b.op_left && a.op_right == b.op_right;
    }

    inline std::ostream & operator <<(std::ostream & stream, const none &) {
        return stream << "[none]";
    }

    inline std::ostream & operator <<(std::ostream & stream, const logical_not & exp) {
        return stream << "~(" << exp.op << ")";
    }

    template<class T>
    inline std::ostream & operator <<(std::ostream & stream, const logical_binary<T> & exp) {
        return stream << "(" << exp.op_left << " " << T::text << " " << exp.op_right << ")";
    }

    // helper function for convenience
    template<class T>
    inline logical_expr make(T exp) {
        return logical_expr { exp };
    }

    bool eval(const logical_expr &, const valuation &);
    expr_result parse(const std::string &);
    logical_expr simplify(logical_expr);
    logical_expr cnf_to_expr(const cnf &);
}

#endif
