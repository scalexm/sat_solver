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
#include <boost/variant.hpp>
#include <ostream>

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
    return stream << "f(" << eq.arg1 << ", " << eq.arg2 << ") = " << eq.right;
}

using curry_atom = boost::variant<const_equality, curry_equality>;

struct curry_env {
    cnf clauses;
    std::vector<curry_atom> atoms;
};

#include "expr/atom.hpp"

expr::atom::term currify(const expr::atom::term &);

#endif
