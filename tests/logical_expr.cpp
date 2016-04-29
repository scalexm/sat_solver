//
//  logical_expr.cpp
//  tests
//
//  Created by Alexandre Martin on 04/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/expr/logical_expr.hpp"
#include "unwrap.hpp"
#include <sstream>
#include <iostream>

TEST_CASE("Testing logical expressions parser", "[logical_expr]") {
    auto exp = expr::logical_expr { expr::none { } };

    SECTION("parsing basic logical expressions") {
        exp = unwrap(expr::parse("3"));
        REQUIRE(exp == expr::make(3));

        exp = unwrap(expr::parse("~3"));
        REQUIRE(exp == expr::make(expr::logical_not { 3 }));

        exp = unwrap(expr::parse("3 \\/ 5"));
        REQUIRE(exp == expr::make(
            expr::logical_or { 3, 5 }
        ));

        exp = unwrap(expr::parse("3 /\\ 5"));
        REQUIRE(exp == expr::make(
            expr::logical_and { 3, 5 }
        ));

        exp = unwrap(expr::parse("3 X 5"));
        REQUIRE(exp == expr::make(
            expr::logical_xor { 3, 5 }
        ));

        exp = unwrap(expr::parse("3 => 5"));
        REQUIRE(exp == expr::make(
            expr::logical_impl { 3, 5 }
        ));

        exp = unwrap(expr::parse("3 <=> -5"));
        REQUIRE(exp == expr::make(
            expr::logical_equiv { 3, -5 }
        ));

        exp = unwrap(expr::parse("(-3 <=> 5)"));
        REQUIRE(exp == expr::make(
            expr::logical_equiv { -3, 5 }
        ));
    }

    SECTION("testing that 0 is not recognized") {
        auto res = expr::parse("7 => 0 /\\ 2");
        REQUIRE(boost::get<std::string>(&res) != nullptr);
    }
    
    SECTION("testing that line return is a conjunction") {
        exp = unwrap(expr::parse("3 \\/ 5 \n 6 \\/ 7 \n 2"));
        REQUIRE(exp == expr::make(
            expr::logical_and {
                expr::logical_and {
                    expr::logical_or { 3, 5 },
                    expr::logical_or { 6, 7 }
                },
                2
            }
        ));
    }

    SECTION("testing priority and left associativity") {
        exp = unwrap(expr::parse("3 /\\ 5 /\\ 6"));
        REQUIRE(exp == expr::make(
            expr::logical_and {
                expr::logical_and { 3, 5 },
                6
            }
        ));

        exp = unwrap(expr::parse("3 /\\ (5 /\\ 6)"));
        REQUIRE(exp == expr::make(
            expr::logical_and {
                3,
                expr::logical_and { 5, 6 }
            }
        ));

        exp = unwrap(expr::parse("3 /\\ 5 /\\6 \\/ 7"));
        REQUIRE(exp == expr::make(
            expr::logical_or {
                expr::logical_and {
                    expr::logical_and { 3, 5 },
                    6
                },
                7
            }
        ));

        exp = unwrap(expr::parse("3 => 5 <=> 6 \\/ ~7 \\/ 8 X 9"));
        REQUIRE(exp == expr::make(
            expr::logical_equiv {
                expr::logical_impl { 3, 5 },
                expr::logical_xor {
                    expr::logical_or {
                        expr::logical_or {
                            6,
                            expr::logical_not { 7 }
                        },
                        8,
                    },
                    9
                }
            }
        ));
    }

    SECTION("testing textual representation") {
        std::ostringstream s;
        s << unwrap(expr::parse("3 /\\5\\/ 2"));
        REQUIRE(s.str() == "((3 /\\ 5) \\/ 2)");

        s.str("");
        s << unwrap(expr::parse("3 X(~5) X -2"));
        REQUIRE(s.str() == "((3 X ~(5)) X -2)");

        s.str("");
        s << unwrap(expr::parse(" 3=> 5 <=>6"));
        REQUIRE(s.str() == "((3 => 5) <=> 6)");
    }
}

TEST_CASE("Testing operations on logical expressions", "[logical_expr]") {
    auto exp = expr::logical_expr { expr::none { } };

    SECTION("testing evaluation") {
        exp = unwrap(expr::parse("3"));
        REQUIRE(!expr::eval(exp, { { 3, false } }));
        REQUIRE(expr::eval(exp, { { 3, true } }));

        exp = unwrap(expr::parse("~3"));
        REQUIRE(expr::eval(exp, { { 3, false } }));
        REQUIRE(!expr::eval(exp, { { 3, true } }));

        exp = unwrap(expr::parse("3 /\\ ~3"));
        REQUIRE(!expr::eval(exp, { { 3, false } }));
        REQUIRE(!expr::eval(exp, { { 3, true } }));

        exp = unwrap(expr::parse("3 \\/ ~3"));
        REQUIRE(expr::eval(exp, { { 3, false } }));
        REQUIRE(expr::eval(exp, { { 3, true } }));

        exp = unwrap(expr::parse("3 /\\ 5"));
        REQUIRE(!expr::eval(exp, { { 3, false }, { 5, false } }));
        REQUIRE(!expr::eval(exp, { { 3, false }, { 5, true } }));
        REQUIRE(!expr::eval(exp, { { 3, true }, { 5, false } }));
        REQUIRE(expr::eval(exp, { { 3, true }, { 5, true } }));

        exp = unwrap(expr::parse("3 \\/ 5"));
        REQUIRE(!expr::eval(exp, { { 3, false }, { 5, false } }));
        REQUIRE(expr::eval(exp, { { 3, false }, { 5, true } }));
        REQUIRE(expr::eval(exp, { { 3, true }, { 5, false } }));
        REQUIRE(expr::eval(exp, { { 3, true }, { 5, true } }));

        exp = unwrap(expr::parse("-3 X 5"));
        REQUIRE(!expr::eval(exp, { { 3, true }, { 5, false } }));
        REQUIRE(expr::eval(exp, { { 3, true }, { 5, true } }));
        REQUIRE(expr::eval(exp, { { 3, false }, { 5, false } }));
        REQUIRE(!expr::eval(exp, { { 3, false }, { 5, true } }));

        exp = unwrap(expr::parse("3 => 5"));
        REQUIRE(expr::eval(exp, { { 3, false }, { 5, false } }));
        REQUIRE(expr::eval(exp, { { 3, false }, { 5, true } }));
        REQUIRE(!expr::eval(exp, { { 3, true }, { 5, false } }));
        REQUIRE(expr::eval(exp, { { 3, true }, { 5, true } }));

        exp = unwrap(expr::parse("3 <=> 5"));
        REQUIRE(expr::eval(exp, { { 3, false }, { 5, false } }));
        REQUIRE(!expr::eval(exp, { { 3, false }, { 5, true } }));
        REQUIRE(!expr::eval(exp, { { 3, true }, { 5, false } }));
        REQUIRE(expr::eval(exp, { { 3, true }, { 5, true } }));

        exp = unwrap(expr::parse("(3 X 5) /\\ (6 => ~(7 <=> 3 \\/ 5))"));
        REQUIRE(expr::eval(exp, { { 3, false }, { 5, true }, { 6, true }, { 7, false } }));
        REQUIRE(!expr::eval(exp, { { 3, false }, { 5, true }, { 6, true }, { 7, true } }));
    }

    SECTION("testing simplification") {
        exp = unwrap(expr::parse("~3"));
        REQUIRE(expr::simplify(std::move(exp)) == unwrap(expr::parse("-3")));

        exp = unwrap(expr::parse("~-3"));
        REQUIRE(expr::simplify(std::move(exp)) == unwrap(expr::parse("3")));

        exp = unwrap(expr::parse("~~-3"));
        REQUIRE(expr::simplify(std::move(exp)) == unwrap(expr::parse("-3")));

        exp = unwrap(expr::parse("~~~3"));
        REQUIRE(expr::simplify(std::move(exp)) == unwrap(expr::parse("-3")));

        exp = unwrap(expr::parse("3 /\\ ~(~(5 => 8))"));
        REQUIRE(expr::simplify(std::move(exp)) == unwrap(expr::parse("3 /\\ (5 => 8)")));
    }
}