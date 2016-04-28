/*
 *  expr/detail/driver.cpp
 *  solver
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#include "driver.hpp"
#include <sstream>

namespace expr { namespace detail {
    result<generic_expr> driver::parse(const std::string & str) {
        begin_scan(str);
        parser p(*this);
        p.parse();
        return std::move(m_root);
    }

    /*
        if an error occurs, we want to use the string part of expr_result
        as a return value for parse()
    */
    void driver::error(const location & l, const std::string & m) {
        std::ostringstream ss;
        ss << l << ": " << m;
        m_root = result<generic_expr> { ss.str() };
    }
} }