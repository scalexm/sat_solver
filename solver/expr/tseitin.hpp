/*
 *  expr/tseitin.hpp
 *  solver
 *
 *  Created by Nicolas Levy on 12/02/2016.
 *  Copyright © 2016 NicolasPierreLevy. All rights reserved.
 *
 */

 #include "logical_expr.hpp"
 #include <utility>

namespace expr {
    std::pair<cnf, variable> tseitin_transform(const logical_expr &);
    valuation remove_trailing_variables(valuation, variable);
}
