/*
 *  expr/tseitin.cpp
 *  solver
 *
 *  Created by Alexandre Martin on 16/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#include "tseitin.hpp"
#include "detail/tseitin.hpp"
#include <list>

namespace expr {
    std::pair<cnf, variable> tseitin_transform(const logical_expr & ex) {
        cnf result;

        auto simple_ex = expr::simplify(ex);
        auto current_variable = boost::apply_visitor(
            detail::find_max_variable { },
            simple_ex
        );

        auto lowest_fresh = current_variable + 1;

        auto tv = detail::tseitin_visitor { lowest_fresh, result };
        current_variable = boost::apply_visitor(tv, simple_ex);
        result.push_back({ current_variable });
        return std::make_pair(result, lowest_fresh);
    }

    valuation remove_trailing_variables(valuation val, variable v) {
        std::list<variable> to_remove;
        for (auto && p : val) {
            if (p.first >= v)
                to_remove.emplace_back(p.first);
        }

        for (auto && p : to_remove)
            val.erase(p);

        return val;
    }
}