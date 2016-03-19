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

void parse_command_line(int argc, const char ** argv, guess_mode & mode, bool & tseitin,
                        std::string & file_name) {
    for (auto i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            auto arg = std::string(argv[i]);
            std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
            if (arg == "-tseitin")
                tseitin = true;
            else if (arg == "-wl")
                mode = guess_mode::WL;
            else if (arg == "-rand")
                mode = guess_mode::RAND;
            else if (arg == "-moms")
                mode = guess_mode::MOMS;
            else if (arg == "-dlis")
                mode = guess_mode::DLIS;
            else
                std::cout << "unkown option " << arg << std::endl;
        } else
            file_name = argv[i];
    }
}