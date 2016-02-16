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

namespace expr {
    cnf tseitin_transform(logical_expr & ex) {
        cnf result;

        auto current_variable = boost::apply_visitor(
            detail::find_max_variable { },
            ex
        );

        auto tv = detail::tseitin_visitor { current_variable, result };
        current_variable = boost::apply_visitor(tv, ex);
        result.push_back({ current_variable });
        return result;
    }
}