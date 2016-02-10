/*
 *  expr/detail/logical_driver.cpp
 *  solver
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#include "logical_driver.hpp"
#include <sstream>

namespace expr { namespace detail {
    expr_result logical_driver::parse(const std::string & str) {
        begin_scan(str);
        logical_parser parser(*this);
        parser.parse();
        return std::move(m_root);
    }

    /*
        if an error occurs, we want to use the string part of expr_result
        as a return value for parse()
    */
    void logical_driver::error(const location & l, const std::string & m) {
        std::ostringstream ss;
        ss << l << ": " << m;
        m_root = expr_result { ss.str() };
    }
} }