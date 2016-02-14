/*
 *  expr/tseitin.hpp
 *  solver
 *
 *  Created by Nicolas Levy on 12/02/2016.
 *  Copyright © 2016 NicolasPierreLevy. All rights reserved.
 *
 */

 #include "./detail/tseitin.hpp"
 #include "logical_expr.hpp"
 #include <vector>
 #include <unordered_set>

namespace expr {

  cnf tseitin_transformation(logical_expr & ex) {

    cnf result;

    auto curentVariable = boost::apply_visitor(
      detail::find_max_variable { },
      ex
      );
    auto tv = detail::tseitin_visitor { curentVariable, result };
    curentVariable = boost::apply_visitor(tv, ex);
    result.push_back({ curentVariable });
    return result;
  }

}
