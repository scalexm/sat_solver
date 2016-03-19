//
//  solver.cpp
//  tests
//
//  Created by Alexandre Martin on 10/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/solver.hpp"
#include "../solver/expr/logical_expr.hpp"
#include <catch/catch.hpp>
#include <iostream>

extern guess_mode mode;

TEST_CASE("Testing SAT solver", "[solver]") {
    SECTION("satisfying basic CNF formulas") {
        solver s { { { -1, 2 }, { -3, 4 }, { -5, -6 }, { 6, -5, -2 } }, mode };
        REQUIRE(s.satisfiable());

        s = solver { { { -1, 2, 3 }, { 1 }, { -2, 3 }, { -2, -3 }, { 2, 3 }, { 2, -3 } }, mode };
        REQUIRE(!s.satisfiable());

        s = solver { { }, mode };
        REQUIRE(s.satisfiable());

        s = solver { { { 1, -2 }, { } }, mode };
        REQUIRE(!s.satisfiable());

        s = solver { { { 1 }, { 2 }, { -1, -2 } }, mode };
        REQUIRE(!s.satisfiable());
    }

    SECTION("calling `solve` twice") {
        solver s { { { -1, 2 }, { -3, 4 }, { -5, -6 }, { 6, -5, -2 } }, mode };
        REQUIRE(s.solve() == s.solve());
    }

    SECTION("testing valuations returned from solver") {
        cnf c = {
            { -1, 2 }, { -3, 4 }, { -5, -6 }, { 6, -5, -2 }
        };

        solver s { c, mode };
        REQUIRE(expr::eval(expr::cnf_to_expr(c), s.solve()));
    }
}
