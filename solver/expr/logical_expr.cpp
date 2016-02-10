/*
 *  expr/logical_expr.cpp
 *  solver
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#include "detail/logical_driver.hpp"
#include "detail/logical_expr.hpp"

namespace expr {
    bool eval(const logical_expr & exp, const valuation & val) {
        return boost::apply_visitor(detail::eval_visitor { val }, exp);
    }

    expr_result parse(const std::string & str) {
        detail::logical_driver driver;
        return driver.parse(str);
    }

    logical_expr simplify(logical_expr exp) {
        return boost::apply_visitor(detail::simplify_visitor { }, exp);
    }

    logical_expr cnf_to_expr(std::vector<std::unordered_set<int>> clauses) {
        if (clauses.empty())
            return none { };

        auto exp = make(none { });
        for (auto && c : clauses) {
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
