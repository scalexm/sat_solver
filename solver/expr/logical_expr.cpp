/*
 *  expr/logical_expr.cpp
 *  solver
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#include "detail/driver.hpp"
#include "detail/logical_expr.hpp"

namespace expr {
    bool eval(const logical_expr & exp, const valuation & val) {
        return boost::apply_visitor(detail::eval_visitor { val }, exp);
    }

    result<logical_expr> parse(const std::string & str) {
        return detail::parse<atom::variable>(str);
    }

    logical_expr simplify(logical_expr exp) {
        return boost::apply_visitor(detail::simplify_visitor { }, exp);
    }

    logical_expr cnf_to_expr(const cnf & cnf) {
        if (cnf.empty())
            return none { };

        auto exp = make(none { });
        for (auto && c : cnf) {
            if (c.empty())
                continue;

            auto clause = make(none { });;
            for (auto && v : c) {
                if (boost::get<none>(&clause))
                    clause = make(v);
                else {
                    clause = make(logical_or {
                        std::move(clause),
                        make(v),
                    });
                }
            }

            if (boost::get<none>(&exp))
                exp = std::move(clause);
            else {
                exp = make(logical_and {
                    std::move(exp),
                    std::move(clause)
                });
            }
        }

        return exp;
    }
}
