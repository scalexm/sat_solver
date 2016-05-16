//
//  curry.hpp
//  solver
//
//  Created by Alexandre Martin on 11/05/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef CURRY_HPP
#define CURRY_HPP

#include "cnf.hpp"
#include "expr/theory_expr.hpp"
#include <boost/variant.hpp>
#include <ostream>

constexpr char CURRY_SYMBOL = '$';

struct const_equality {
    int left, right;
};

struct curry_equality {
    int arg1, arg2, right;
};

inline std::ostream & operator <<(std::ostream & stream, const const_equality & eq) {
    return stream << eq.left << " = " << eq.right;
}

inline std::ostream & operator <<(std::ostream & stream, const curry_equality & eq) {
    return stream << CURRY_SYMBOL << '(' << eq.arg1 << ", " << eq.arg2 << ") = " << eq.right;
}

using curry_atom = boost::variant<const_equality, curry_equality>;

struct curry_env {
    cnf clauses;
    int max_constant;
    int tseitin_offset;
    std::vector<curry_atom> atoms; // offset + 1 <-> curry_atom
};

curry_env curry_transform(const expr::equality_expr &);

#endif
