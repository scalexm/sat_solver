//
//  main.cpp
//  tests
//
//  Created by Alexandre Martin on 03/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#define CATCH_CONFIG_RUNNER
#include <catch/catch.hpp>
#include "../solver/solver.hpp"

bool gen(const std::string &, long long int);
guess_mode mode = guess_mode::LINEAR;

int main(int argc, const char ** argv) {
    if (argc > 1 && std::string { argv[1] } == "--gen") {
        if (argc < 3) {
            std::cerr << "please input a number" << std::endl;
            return EXIT_FAILURE;
        }

        auto number = std::stoll(argv[2]);
        std::string output = "output.cnf";

        if (argc > 3)
            output = argv[3];

        return gen(output, number) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    int result = Catch::Session().run( argc, argv );
    return result;
}