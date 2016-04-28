//
//  unwrap.hpp
//  tests
//
//  Created by Alexandre Martin on 28/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef UNWRAP_HPP
#define UNWRAP_HPP

#include <catch/catch.hpp>

template<class Exp>
Exp unwrap(expr::result<Exp> res) {
    if (auto err = boost::get<std::string>(&res))
        FAIL(*err);
    return boost::get<Exp>(res);
}

#endif
