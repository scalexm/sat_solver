//
//  curry.cpp
//  solver
//
//  Created by Alexandre Martin on 11/05/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "detail/curry.hpp"
#include "detail/flatten.hpp"
#include "expr/tseitin.hpp"

curry_env curry_transform(const expr::equality_expr & exp) {
    std::unordered_map<char, int> fun_seen;
    std::unordered_map<int, int> var_seen;
    int current_index = 0;
    detail::currying_visitor v_curry { fun_seen, var_seen, current_index };
    auto curry_exp = boost::apply_visitor(v_curry, exp);

    curry_env env;

    std::unordered_map<std::string, int> already_flatten;
    std::vector<curry_atom> new_eqs;
    detail::flatten_visitor v_flat { already_flatten, new_eqs, current_index };
    auto flat_exp = boost::apply_visitor(v_flat, curry_exp);
    env.max_constant = current_index;

    std::unordered_map<std::string, int> already_seen;
    detail::curry_to_logical_visitor v_logic { already_seen, env.atoms };
    auto logic_exp = boost::apply_visitor(v_logic, flat_exp);

    auto offset = new_eqs.size(); // leaving space for new_eqs
    auto tseitin = expr::tseitin_transform(logic_exp, offset);

    env.clauses = std::move(tseitin.first);
    env.tseitin_offset = tseitin.second;
    for (int i = env.atoms.size() + 1; i <= env.atoms.size() + offset; ++i)
        env.clauses.emplace_back(std::unordered_set<int> { i });

    env.atoms.insert(env.atoms.end(), new_eqs.begin(), new_eqs.end());
    return env;
}