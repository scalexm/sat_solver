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

TEST_CASE("Testing SAT solver", "[solver]") {
    SECTION("satisfying basic CNF formulas") {
        solver s { { { -1, 2 }, { -3, 4 }, { -5, -6 }, { 6, -5, -2 } } };
        REQUIRE(s.satisfiable());

        s = solver { { { -1, 2, 3 }, { 1 }, { -2, 3 }, { -2, -3 }, { 2, 3 }, { 2, -3 } } };
        REQUIRE(!s.satisfiable());

        s = solver { { } };
        REQUIRE(s.satisfiable());

        s = solver { { { 1, -2 }, { } } };
        REQUIRE(!s.satisfiable());
    }

    SECTION("calling `solve` two times") {
        solver s { { { -1, 2 }, { -3, 4 }, { -5, -6 }, { 6, -5, -2 } } };
        REQUIRE(s.solve() == s.solve());
    }

    SECTION("testing valuations returned from solver") {
        cnf c = {
            { -1, 2 }, { -3, 4 }, { -5, -6 }, { 6, -5, -2 }
        };

        solver s { c };
        REQUIRE(expr::eval(expr::cnf_to_expr(c), s.solve()));
    }
}
