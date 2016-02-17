//
//  gen_tests.cpp
//  tests
//
//  Created by Alexandre Martin on 17/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "../solver/cnf.hpp"
#include <fstream>
#include <iostream>

cnf factor_to_cnf(long long int);
bool is_prime(long long int);


bool gen(const std::string & output_name, long long int n) {
    std::ofstream file { output_name };
    if (!file) {
        std::cerr << "cannot create file " << output_name << std::endl;
        return false;
    }

    auto cnf = factor_to_cnf(n);
    std::unordered_set<int> variables;
    for (auto && c : cnf)
        for (auto && lit : c)
            variables.emplace(std::abs(lit));

    auto V = variables.size();
    auto C = cnf.size();

    file << "c " << n << "\n";
    if (is_prime(n))
        file << "c UNSATISFIABLE\n";
    else
        file << "c SATISFIABLE\n";

    file << "p cnf " << V << " " << C << "\n";
    for (auto && c : cnf) {
        for (auto && lit : c)
            file << lit << " ";
        file << "0\n";
    }

    return true;
}