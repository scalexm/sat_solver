//
//  command_line.cpp
//  solver
//
//  Created by Alexandre Martin on 18/03/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "command_line.hpp"
#include "solver.hpp"
#include <iostream>
#include <algorithm>

void parse_command_line(int argc, const char ** argv, std::string & file_name, options & opt,
                        bool & tseitin) {
    for (auto i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            auto arg = std::string(argv[i]);
            std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
            if (arg == "-tseitin")
                tseitin = true;
            else if (arg == "-wl")
                opt.wl = true;
            else if (arg == "-rand")
                opt.guess = guess_mode::RAND;
            else if (arg == "-moms")
                opt.guess = guess_mode::MOMS;
            else if (arg == "-dlis")
                opt.guess = guess_mode::DLIS;
            else if (arg == "-cl-interac")
                opt.cdcl = cdcl_mode::INTERACTIVE;
            else if (arg == "-cl")
                opt.cdcl = cdcl_mode::NORMAL;
            else
                std::cout << "unkown option `" << arg << "`" << std::endl;
        } else
            file_name = argv[i];

        if (opt.wl && (opt.guess == guess_mode::MOMS || opt.guess == guess_mode::DLIS)) {
            std::cout << "cannot use MOMS or DLIS with watched litterals: switched to LINEAR"
                      << std::endl;
            opt.guess = guess_mode::LINEAR;
        }

        if (opt.cdcl != cdcl_mode::NONE
            && (opt.guess == guess_mode::VSIDS || opt.guess == guess_mode::FORGET)) {
            std::cout << "VSIDS or FORGET can only be used with clause learning: switched to LINEAR"
                      << std::endl;
            opt.guess = guess_mode::LINEAR;
        }
    }
}