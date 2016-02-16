//
//  factor.cpp
//  tests
//
//  Created by Alexandre Martin on 12/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/solver.hpp"
#include <iostream>
#include <cmath>
#include <cassert>
#include <catch/catch.hpp>

cnf encode(const std::vector<int> & bits, long long int n) {
    auto l = bits.size();
    cnf f;

    for (auto i = 0; i < l; ++i) {
        auto bit = 1 << i;
        f.push_back({ (n & bit) == bit ? bits[i] : -bits[i] });
    }
    return f;
}

cnf encode_not_equal(const std::vector<int> & bits, long long int n) {
    auto l = bits.size();

    std::unordered_set<int> clause;
    for (auto i = 0; i < l; ++i) {
        auto bit = 1 << i;
        clause.emplace((n & bit) == bit ? -bits[i] : bits[i]);
    }

    return cnf { { clause } };
}

cnf equality(const std::vector<int> & bits_r, const std::vector<int> & bits_p) {
    auto l = bits_r.size();
    assert(l == bits_p.size());
    cnf f;

    for (auto i = 0; i < l; ++i) {
        f.push_back({ bits_r[i], -bits_p[i] });
        f.push_back({ bits_p[i], -bits_r[i] });
    }

    return f;
}

cnf equality_times2(const std::vector<int> & bits_r, const std::vector<int> & bits_p) {
    auto l = bits_r.size();
    assert(l == bits_p.size());
    cnf f;

    f.push_back({ -bits_r[0] });
    for (auto i = 1; i < l; ++i) {
        f.push_back({ bits_r[i], -bits_p[i - 1] });
        f.push_back({ -bits_r[i], bits_p[i - 1] });
    }

    return f;
}

cnf equality_or_zero(const std::vector<int> & bits_r, const std::vector<int> & bits_p, int b) {
    auto l = bits_r.size();
    assert(l == bits_p.size());
    cnf f;

    for (auto i = 0; i < l; ++i) {
        f.push_back({ b, -bits_r[i] });
        f.push_back({ bits_p[i], -bits_r[i] });
        f.push_back({ bits_r[i], -b, -bits_p[i] });
    }

    return f;
}

cnf equality_sum(const std::vector<int> & bits_r, const std::vector<int> & bits_p,
                 const std::vector<int> & bits_q, const std::vector<int> & bits_c) {
    auto l = bits_r.size();
    assert(l == bits_p.size() && l == bits_q.size() && l + 1 == bits_c.size());
    cnf f;

    f.push_back({ -bits_c[0] });
    f.push_back({ -bits_c[l] });
    for (auto i = 0; i < l; ++i) {
        f.push_back({ -bits_c[i + 1], bits_p[i], bits_c[i] });
        f.push_back({ -bits_c[i + 1], bits_p[i], bits_q[i] });
        f.push_back({ -bits_c[i + 1], bits_q[i], bits_c[i] });
        f.push_back({ bits_c[i + 1], -bits_p[i], -bits_c[i] });
        f.push_back({ bits_c[i + 1], -bits_p[i], -bits_q[i] });
        f.push_back({ bits_c[i + 1], -bits_q[i], -bits_c[i] });
    }

    for (auto i = 0; i < l; ++i) {
        f.push_back({ bits_r[i], bits_q[i], bits_p[i], -bits_c[i] });
        f.push_back({ bits_r[i], bits_q[i], -bits_p[i], bits_c[i] });
        f.push_back({ bits_r[i], -bits_q[i], bits_p[i], bits_c[i] });
        f.push_back({ bits_r[i], -bits_q[i], -bits_p[i], -bits_c[i] });
        f.push_back({ -bits_r[i], bits_q[i], bits_p[i], bits_c[i] });
        f.push_back({ -bits_r[i], bits_q[i], -bits_p[i], -bits_c[i] });
        f.push_back({ -bits_r[i], -bits_q[i], bits_p[i], -bits_c[i] });
        f.push_back({ -bits_r[i], -bits_q[i], -bits_p[i], bits_c[i] });
    }

    return f;
}

std::pair<std::vector<int>, int> make_bits(int l, int start) {
    std::vector<int> bits(l);
    for (auto i = 0; i < l; ++i)
        bits[i] = i + start;
    return std::make_pair(std::move(bits), l + start);
}

cnf encode_to_cnf(long long int n) {
    auto l = std::floor(std::log2(n)) + 1;
    auto start = 1;

    std::vector<int> p;
    std::tie(p, start) = make_bits(l, start);
    return encode(p, n);
}

cnf equality_to_cnf(int l) {
    auto start = 1;
    std::vector<int> p, q;
    std::tie(p, start) = make_bits(l, start);
    std::tie(q, start) = make_bits(l, start);
    return equality(p, q);
}

cnf equality_times2_to_cnf(int l) {
    auto start = 1;
    std::vector<int> p, q;
    std::tie(p, start) = make_bits(l, start);
    std::tie(q, start) = make_bits(l, start);
    return equality_times2(p, q);
}

cnf equality_sum_to_cnf(int l) {
    auto start = 1;
    std::vector<int> r, p, q, c;
    std::tie(r, start) = make_bits(l, start);
    std::tie(p, start) = make_bits(l, start);
    std::tie(q, start) = make_bits(l, start);
    std::tie(c, start) = make_bits(l + 1, start);

    return equality_sum(r, p, q, c);
}

cnf factor_to_sat(long long int n) {
    auto l = std::floor(std::log2(n)) + 1;
    auto start = 1;

    std::vector<std::vector<int>> s(l), m(l), r(l);
    std::vector<int> p, q;

    std::tie(p, start) = make_bits(l, start);
    std::tie(q, start) = make_bits(l, start);
    std::tie(s[0], start) = make_bits(l, start);

    auto cnf = equality(s[0], p);
    for (auto i = 1; i < l; ++i) {
        std::tie(s[i], start) = make_bits(l, start);
        auto cnff = equality_times2(s[i], s[i - 1]);
        cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    }

    for (auto i = 0; i < l; ++i) {
        std::tie(m[i], start) = make_bits(l, start);
        auto cnff = equality_or_zero(m[i], s[i], q[i]);
        cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    }

    std::tie(r[0], start) = make_bits(l, start);
    auto cnff = equality(r[0], m[0]);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    for (auto i = 1; i < l; ++i) {
        std::vector<int> c;
        std::tie(c, start) = make_bits(l + 1, start);
        std::tie(r[i], start) = make_bits(l, start);
        cnff = equality_sum(r[i], r[i - 1], m[i], c);
        cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    }

    cnff = encode(r[l - 1], n);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());

    cnff = encode_not_equal(p, 1);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    cnff = encode_not_equal(p, n);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());

    return cnf;
}

long long int val_to_number(const std::unordered_map<int, bool> & val, int begin, int end) {
    auto number = 0;
    for (auto && v : val) {
        if (v.first >= begin && v.first <= end)
            number += v.second * (1 << (v.first - begin));
    }

    return number;
}

TEST_CASE("Testing SAT factor encoder") {
    SECTION("testing encode") {
        auto cnf = encode_to_cnf(2347862);
        solver s { cnf };
        auto val = s.solve();
        REQUIRE(val_to_number(val, 1, val.size() + 1) == 2347862);

        cnf = encode_to_cnf(982478);
        s = solver { cnf };
        val = s.solve();
        REQUIRE(val_to_number(val, 1, val.size() + 1) == 982478);
    }

    SECTION("testing encode not equal") {
    }

    SECTION("testing equality") {
        auto cnf = equality_to_cnf(10);
        solver s { cnf };
        auto val = s.solve();
        REQUIRE(val_to_number(val, 1, 10) == val_to_number(val, 11, 20));

        cnf = equality_to_cnf(20);
        s = solver { cnf };
        val = s.solve();
        REQUIRE(val_to_number(val, 1, 20) == val_to_number(val, 21, 40));
    }

    SECTION("testing equality times two") {
        auto cnf = equality_times2_to_cnf(10);
        solver s { cnf };
        auto val = s.solve();
        REQUIRE(val_to_number(val, 1, 10) == 2 * val_to_number(val, 11, 20));

        cnf = equality_times2_to_cnf(20);
        s = solver { cnf };
        val = s.solve();
        REQUIRE(val_to_number(val, 1, 20) == 2 * val_to_number(val, 21, 40));
    }

    SECTION("testing equality sum") {
        auto cnf = equality_sum_to_cnf(10);
        solver s { cnf };
        auto val = s.solve();
        REQUIRE(val_to_number(val, 1, 10) == val_to_number(val, 11, 20) + val_to_number(val, 21, 30));
    }
}
