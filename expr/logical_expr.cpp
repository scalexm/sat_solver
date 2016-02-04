/*
 *  logical_expr.cpp
 *  expr
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#include "detail/logical_driver.hpp"

namespace expr {
    expr_result parse(const std::string & str) {
        detail::logical_driver driver;
        return driver.parse(str);
    }
}
