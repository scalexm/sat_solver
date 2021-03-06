//
//  tseitin.cpp
//  tests
//
//  Created by Alexandre Martin on 16/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/expr/tseitin.hpp"
#include "../solver/solver.hpp"
#include "unwrap.hpp"

extern options opt;

TEST_CASE("Testing tseitin transform", "[logical_expr][solver]") {
    auto exp = unwrap(expr::parse("(3 X 5) /\\ (6 => ~(7 <=> 3 \\/ 5))"));
    auto tseitin = expr::tseitin_transform(exp);

    solver s { std::move(tseitin.first), opt };
    auto val = s.solve();

    REQUIRE(expr::eval(exp, val));
    REQUIRE(expr::eval(exp, expr::remove_trailing_variables(val, tseitin.second)));

    exp = unwrap(expr::parse("7 <=> -7"));
    tseitin = expr::tseitin_transform(exp);

    s = solver { std::move(tseitin.first), opt };
    REQUIRE(!s.satisfiable());

    exp = unwrap(expr::parse("7 <=> ~7"));
    tseitin = expr::tseitin_transform(exp);

    s = solver { std::move(tseitin.first), opt };
    REQUIRE(!s.satisfiable());

    exp = unwrap(expr::parse("7 <=> 7"));
    tseitin = expr::tseitin_transform(exp);

    s = solver { std::move(tseitin.first), opt };
    REQUIRE(s.satisfiable());

    exp = unwrap(expr::parse("1 => -1"));
    tseitin = expr::tseitin_transform(exp);

    s = solver { std::move(tseitin.first), opt };
    REQUIRE(s.satisfiable());
}