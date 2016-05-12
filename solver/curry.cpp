//
//  curry.cpp
//  solver
//
//  Created by Alexandre Martin on 11/05/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "detail/curry.hpp"

expr::atom::term currify(const expr::atom::term & t) {
    std::unordered_map<char, int> fun_seen;
    std::unordered_map<int, int> var_seen;
    int current_index = 0;
    detail::currying_visitor v { fun_seen, var_seen, current_index };
    return boost::apply_visitor(v, t);
}