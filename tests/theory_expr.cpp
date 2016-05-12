//
//  theory_expr.cpp
//  tests
//
//  Created by Alexandre Martin on 28/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/expr/theory_expr.hpp"
#include "unwrap.hpp"
#include <sstream>

TEST_CASE("Testing equality expressions parser", "[equality_expr]") {
    auto exp = expr::equality_expr { expr::none { } };

    SECTION("parsing basic equality expressions") {
        exp = unwrap(expr::parse_equality("3 = 5"));
        REQUIRE(exp == expr::make_equality(expr::atom::equality { 3, 5 }));

        exp = unwrap(expr::parse_equality("3 = 5 /\\ ~4 = 2"));
        REQUIRE(exp == expr::make_equality(
            expr::equality_and {
                expr::atom::equality { 3, 5 },
                expr::equality_not {
                    expr::atom::equality { 4, 2 }
                }
            }
        ));

        exp = unwrap(expr::parse_equality("3 = 2 => (1 = 3 X 2 != 3)"));
        REQUIRE(exp == expr::make_equality(
            expr::equality_impl {
                expr::atom::equality { 3, 2 },
                expr::equality_xor {
                    expr::atom::equality { 1, 3 },
                    expr::equality_not { expr::atom::equality { 2, 3 } }
                }
            }
        ));
    }

    SECTION("testing textual representation for equality") {
        std::ostringstream s;
        s << unwrap(expr::parse_equality("3=5 /\\ ~4= 2"));
        REQUIRE(s.str() == "(3 = 5 /\\ 4 != 2)");

        s.str("");
        s << unwrap(expr::parse_equality("3 = 2 =>(1 =3 X2!= 3)"));
        REQUIRE(s.str() == "(3 = 2 => (1 = 3 X 2 != 3))");
    }

    SECTION("parsing basic congruence expressions") {
        exp = unwrap(expr::parse_equality("f(1, 2) = a => g(1) = f(3, 4)"));
        REQUIRE(exp == expr::make_equality(
            expr::equality_impl {
                expr::atom::equality {
                    expr::atom::fun { 'f', { 1, 2 } },
                    expr::atom::fun { 'a', { } }
                },
                expr::atom::equality {
                    expr::atom::fun { 'g', { 1 } },
                    expr::atom::fun { 'f', { 3, 4 } }
                }
            }
        ));

        exp = unwrap(expr::parse_equality("f(f(1, 2), c) = g(f(3, 4))"));
        REQUIRE(exp == expr::make_equality(
            expr::atom::equality {
                expr::atom::fun {
                    'f',
                    { expr::atom::fun { 'f', { 1, 2 } }, expr::atom::fun { 'c', { } } }
                },
                expr::atom::fun {
                    'g',
                    { expr::atom::fun { 'f', { 3, 4 } } }
                }
            }
        ));
    }

    SECTION("testing textual representation for congruence") {
        std::ostringstream s;
        s << unwrap(expr::parse_equality("f(1,2) !=a => g(1) =f(3, 4)"));
        REQUIRE(s.str() == "(f(1, 2) != a => g(1) = f(3, 4))");

        s.str("");
        s << unwrap(expr::parse_equality("a = bX z =f(1, g(3,4)) /\\ 1 = g(2,4)"));
        REQUIRE(s.str() == "(a = b X (z = f(1, g(3, 4)) /\\ 1 = g(2, 4)))");
    }
}