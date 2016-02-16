/*
 *  expr/tseitin.hpp
 *  solver
 *
 *  Created by Nicolas Levy on 12/02/2016.
 *  Copyright © 2016 NicolasPierreLevy. All rights reserved.
 *
 */

 #include "logical_expr.hpp"

namespace expr {
    cnf tseitin_transform(logical_expr &);
}
