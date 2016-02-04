/*
 *  detail/logical_driver.hpp
 *  expr
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#ifndef LOGICAL_DRIVER_HPP
#define LOGICAL_DRIVER_HPP

#include "../logical_expr.hpp"
#include "y.tab.h"
#include <string>

#define YY_DECL \
    expr::detail::logical_parser::symbol_type yylex(expr::detail::logical_driver & driver)
YY_DECL;

namespace expr { namespace detail {
    class logical_driver {
    private:
        expr_result m_root;

        void begin_scan(const std::string &); // in logical_scanner.lpp
    public:
        logical_driver() = default;
        expr_result parse(const std::string &);
        void error(const location &, const std::string &);

        void set_root(logical_expr root) {
            m_root = expr_result { std::move(root) };
        }
    };
} }

#endif