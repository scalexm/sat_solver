//
//  factor.cpp
//  tests
//
//  Created by Alexandre Martin on 12/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/solver.hpp"
#include <cmath>
#include <cassert>
#include <catch/catch.hpp>
#include <random>

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

cnf equality_pow2_or_zero(const std::vector<int> & bits_r, const std::vector<int> & bits_p, int pow, int b) {
    auto l = bits_r.size();
    assert(l == bits_p.size());
    cnf f;

    for (auto i = 0; i < pow; ++i) {
        f.push_back({ -bits_r[i] });
        f.push_back({ -b, -bits_p[l - 1 - i] });
    }

    for (auto i = pow; i < l; ++i) {
        f.push_back({ b, -bits_r[i] });
        f.push_back({ bits_r[i], -b, -bits_p[i - pow] });
        f.push_back({ -bits_r[i], bits_p[i - pow] });
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

cnf equation_to_cnf(int l, int x, int y) {
    auto start = 1;
    std::vector<int> r, p, q ,c;
    std::tie(q, start) = make_bits(l, start);
    std::tie(p, start) = make_bits(l, start);
    std::tie(r, start) = make_bits(l, start);
    std::tie(c, start) = make_bits(l + 1, start);

    auto cnf = equality_sum(r, p, q, c);
    auto cnff = encode(r, x);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    cnff = encode(p, y);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    return cnf;
}

cnf check_pow2_or_zero_to_cnf(int l, int x, int y, int pow, bool b) {
    auto start = 1;
    std::vector<int> r, p;
    std::tie(r, start) = make_bits(l, start);
    std::tie(p, start) = make_bits(l, start);

    auto cnf = equality_pow2_or_zero(r, p, pow, start);
    auto cnff = encode(r, x);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    cnff = encode(p, y);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    cnf.push_back({ b ? start : -start });
    return cnf;
}

cnf check_sum_to_cnf(int l, int x, int y, int z) {
    auto start = 1;
    std::vector<int> r, p, q ,c;
    std::tie(r, start) = make_bits(l, start);
    std::tie(p, start) = make_bits(l, start);
    std::tie(q, start) = make_bits(l, start);
    std::tie(c, start) = make_bits(l + 1, start);

    auto cnf = equality_sum(r, p, q, c);
    auto cnff = encode(r, x);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    cnff = encode(p, y);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    cnff = encode(q, z);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    return cnf;

}

cnf factor_to_cnf(long long int n) {
    auto l = std::floor(std::log2(n)) + 1;
    auto start = 1;

    std::vector<int> p, q;
    std::vector<std::vector<int>> s(l), r(l);

    std::tie(p, start) = make_bits(l, start);
    std::tie(q, start) = make_bits(l, start);
    for (auto i = 0; i < l; ++i)
        std::tie(s[i], start) = make_bits(l, start);
    for (auto i = 0; i < l; ++i)
        std::tie(r[i], start) = make_bits(l, start);

    cnf cnf;
    for (auto i = 0; i < l; ++i) {
        auto cnff = equality_pow2_or_zero(s[i], p, i, q[i]);
        cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    }

    auto cnff = equality(r[0], s[0]);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    for (auto i = 1; i < l; ++i) {
        std::vector<int> c;
        std::tie(c, start) = make_bits(l + 1, start);
        cnff = equality_sum(r[i], r[i - 1], s[i], c);
        cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    }

    cnff = encode(r[l - 1], n);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());

    cnff = encode_not_equal(p, 1);
    cnf.insert(cnf.end(), cnff.begin(), cnff.end());
    cnff = encode_not_equal(q, 1);
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

bool is_prime(long long int number) {
    for (int i = 2; i < number; ++i) {
        if (number % i == 0)
            return false;
	}
	return true;	
}

extern guess_mode mode;
extern cdcl_mode cdcl;

TEST_CASE("Testing arithmetic operations encoder") {
    SECTION("testing encode") {
        solver s { encode_to_cnf(2347862), mode, cdcl };
        auto val = s.solve();
        REQUIRE(val_to_number(val, 1, val.size() + 1) == 2347862);

        s = solver { encode_to_cnf(982478), mode, cdcl };
        val = s.solve();
        REQUIRE(val_to_number(val, 1, val.size() + 1) == 982478);
    }

    SECTION("testing equality pow2") {
        solver s { check_pow2_or_zero_to_cnf(4, 0, 3, 3, false), mode, cdcl };
        REQUIRE(s.satisfiable());

        s = solver { check_pow2_or_zero_to_cnf(4, 8, 2, 3, false), mode, cdcl };
        REQUIRE(!s.satisfiable());

        s = solver { check_pow2_or_zero_to_cnf(4, 8, 1, 3, true), mode, cdcl };
        REQUIRE(s.satisfiable());
    }

    SECTION("testing sum") {
        solver s { check_sum_to_cnf(10, 20, 10, 10), mode, cdcl };
        REQUIRE(s.satisfiable());

        s = solver { check_sum_to_cnf(10, 20, 8, 9), mode, cdcl };
        REQUIRE(!s.satisfiable());

        s = solver { check_sum_to_cnf(4, 10, 5, 5), mode, cdcl };
        REQUIRE(s.satisfiable());

        s = solver { check_sum_to_cnf(4, 10, 13, 13), mode, cdcl };
        REQUIRE(!s.satisfiable());
    }

    SECTION("solving more specific equations") {
        solver s { equation_to_cnf(2, 3, 0), mode, cdcl };
        auto val = s.solve();
        REQUIRE(val_to_number(val, 1, 2) + 0 == 3);

        s = solver { equation_to_cnf(20, 243243, 123123), mode, cdcl };
        val = s.solve();
        REQUIRE(val_to_number(val, 1, 20) + 123123 == 243243);
    }
}

TEST_CASE("Testing factor to SAT encoder", "[factor][solver][.]") {
    SECTION("factoring numbers from 2 to 10000") {
        for (auto i = 2; i <= 1000; ++i) {
            solver s { factor_to_cnf(i), mode, cdcl };
            if (is_prime(i))
                REQUIRE(!s.satisfiable());
            else {
                auto val = s.solve();
                auto l = std::floor(std::log2(i)) + 1;
                REQUIRE(val_to_number(val, 1, l) * val_to_number(val, l + 1, 2 * l) == i);
            }
        }
    }

    SECTION("factoring random numbers") {
        std::random_device rd;
        std::mt19937 gen(rd());
        for (auto i = 0; i < 10; ++i) {
            std::uniform_int_distribution<> dis(100, 10000);
            auto x = dis(gen), y = dis(gen);
            auto n = x * y;

            solver s { factor_to_cnf(n), mode, cdcl };
            auto val = s.solve();
            auto l = std::floor(std::log2(n)) + 1;
            REQUIRE(val_to_number(val, 1, l) * val_to_number(val, l + 1, 2 * l) == n);
        }
    }
}