//
//  expr/expr.hpp
//  solver
//
//  Created by Alexandre Martin on 28/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef EXPR_HPP
#define EXPR_HPP

#include "detail/expr.hpp"

namespace expr {
    using none = detail::none_;

    /*
        result is either a Expr (success) or a string (error)
    */
    template<class Expr>
    using result = boost::variant<Expr, std::string>;
}

#endif
