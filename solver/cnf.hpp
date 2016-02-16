//
//  cnf.hpp
//  solver
//
//  Created by Alexandre Martin on 16/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef CNF_HPP
#define CNF_HPP

#include <vector>
#include <unordered_set>
#include <unordered_map>

using cnf = std::vector<std::unordered_set<int>>;
using valuation = std::unordered_map<int, bool>;

#endif