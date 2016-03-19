//
//  command_line.hpp
//  solver
//
//  Created by Alexandre Martin on 12/03/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef COMMAND_LINE_HPP
#define COMMAND_LINE_HPP

#include <string>
enum class guess_mode;

void parse_command_line(int, const char **, guess_mode &, bool &, std::string &);

#endif
