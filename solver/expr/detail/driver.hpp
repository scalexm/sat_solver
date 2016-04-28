/*
 *  expr/detail/driver.hpp
 *  solver
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "../logical_expr.hpp"

namespace expr { namespace detail {

    /* I would have liked to use a templated driver, but I can't see how to integrate it
       with Bison and Flex */
    using generic_atom = boost::variant<
        atom::variable,
        atom::equality
    >;

    using generic_expr = expr_<generic_atom>;
} }

#include "y.tab.h"
#include <string>
#include <sstream>

#define YY_DECL \
    expr::detail::parser::symbol_type yylex(expr::detail::driver & driver)
YY_DECL;

namespace expr { namespace detail {

    template<class Atom>
    class convert_visitor : public boost::static_visitor<expr_<Atom>> {
    public:
        expr_<Atom> operator ()(none_ & exp) const {
            return std::move(exp);
        }

        template<class Tag>
        expr_<Atom> operator ()(binary_<Tag, generic_atom> & exp) const {
            return binary_<Tag, Atom> {
                boost::apply_visitor(convert_visitor { }, exp.op_left),
                boost::apply_visitor(convert_visitor { }, exp.op_right)
            };
        }

        expr_<Atom> operator ()(generic_atom & at) const {
            if (auto v = boost::get<Atom>(&at))
                return std::move(*v);
            std::ostringstream error;
            error << "bad atom: " << at;

            /* I usually don't like exceptions very much, but using
               expr::result would have been quite annoying, as I would have been
               forced to define monadic operations on it (map, map_err). But here, as it is
               detail-level code we can afford exceptions
            */
            throw error.str();
        }

        expr_<Atom> operator ()(not_<generic_atom> & exp) const {
            return not_<Atom> { boost::apply_visitor(convert_visitor { }, exp.op) };
        }
    };

    class driver {
    private:
        result<generic_expr> m_root;

        void begin_scan(const std::string &); // in logical_scanner.lpp
    public:
        driver() = default;
        result<generic_expr> parse(const std::string &);
        void error(const location &, const std::string &);

        /*
            will be called by the parser after having computed the result
        */
        void set_root(generic_expr root) {
            m_root = { std::move(root) };
        }
    };

    template<class Atom>
    result<expr_<Atom>> parse(const std::string & str) {
        detail::driver driver;
        auto res = driver.parse(str);
        if (auto err = boost::get<std::string>(&res))
            return std::move(*err);

        try {
            return boost::apply_visitor(
                detail::convert_visitor<Atom> { },
                boost::get<detail::generic_expr>(res)
            );
        } catch (std::string err) {
            return std::move(err);
        }
    }

} }

#endif