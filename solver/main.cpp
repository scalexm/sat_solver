//
//  main.cpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include <iostream>
#include "solver.hpp"

int main(int argc, char ** argv) {
    solver s { { { -1, 2 }, { -3, 4 }, { -5, -6 }, { 6, -5, -2 } } };
    auto v = s.solve();
    for (auto && c : v)
        std::cout << c.first << " = " << std::boolalpha << c.second <<"; ";
    std::cout << std::endl;
    return 0;
}
