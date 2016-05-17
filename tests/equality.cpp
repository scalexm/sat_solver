//
//  equality.cpp
//  tests
//
//  Created by Alexandre Martin on 16/05/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/curry.hpp"
#include "../solver/equality.hpp"
#include "../solver/solver.hpp"
#include "unwrap.hpp"

extern options opt;

bool satisfiable(const std::string & str) {
    auto exp = unwrap(expr::parse_equality(str));
    auto env = curry_transform(exp);

    equality_solver eq_s { env.max_constant, std::move(env.atoms) };
    options local_opt = opt;
    local_opt.eq_solver = &eq_s;
    solver s { std::move(env.clauses), local_opt };
    return s.satisfiable();
}

TEST_CASE("Testing equality solver", "[equality_solver]") {
    REQUIRE(!satisfiable("a != a"));
    REQUIRE(satisfiable("a = b"));
    REQUIRE(satisfiable("a = b /\\ b = c"));
    REQUIRE(!satisfiable("a = b /\\ b = c /\\ c != a"));
    REQUIRE(satisfiable("f(a, b) = f(c, d) \\/ a != c \\/ b != d"));
    REQUIRE(!satisfiable("(f(f(a)) != b \\/ f(f(f(b))) != b) /\\ f(a) = a /\\ a = b"));
    REQUIRE(satisfiable("b = d /\\ f(b) = d /\\ f(d) = a"));
    REQUIRE(!satisfiable("b = d /\\ f(b) = d /\\ f(d) = a /\\ a != b"));
    REQUIRE(satisfiable("(a = b /\\ b = c) => a = c"));
    REQUIRE(satisfiable("a = b => f(a) = f(b)"));
    REQUIRE(satisfiable("f(a) != f(b) => a != b"));
    REQUIRE(!satisfiable(("f(f(a, b), b) = c /\\ f(a, b) = c /\\ c = d /\\ f(d, b) != c")));
    REQUIRE(satisfiable("(a = b => c = d) /\\ a != b"));
}