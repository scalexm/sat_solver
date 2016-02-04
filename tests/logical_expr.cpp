//
//  logical_expr.cpp
//  tests
//
//  Created by Alexandre Martin on 04/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include <catch/catch.hpp>
#include <iostream>
#include "../expr/logical_expr.hpp"

expr::logical_expr unwrap(expr::expr_result res) {
    if (auto err = boost::get<std::string>(&res))
        FAIL(*err);
    return boost::get<expr::logical_expr>(res);
}

TEST_CASE("Testing logical expressions", "[logical_expr]") {
    auto exp = expr::logical_expr { expr::none { } };

    SECTION("parsing some logical expressions" ) {
        exp = unwrap(expr::parse("3"));
        REQUIRE(exp == expr::make_expr(3));

        exp = unwrap(expr::parse("3 \\/ 5"));
        REQUIRE(exp == expr::make_expr(
            expr::logical_or {
                expr::make_expr(3),
                expr::make_expr(5)
            }
        ));

    }
}