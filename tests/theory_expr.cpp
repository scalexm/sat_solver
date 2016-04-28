//
//  theory_expr.cpp
//  tests
//
//  Created by Alexandre Martin on 28/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/expr/theory_expr.hpp"
#include "unwrap.hpp"

TEST_CASE("Testing equality expressions parser", "[equality_expr]") {
    std::ostringstream s;
    s << unwrap(expr::parse_equality("3=5"));
    REQUIRE(s.str() == "3 = 5");
}