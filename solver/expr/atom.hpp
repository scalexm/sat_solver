//
//  expr/atom.hpp
//  solver
//
//  Created by Alexandre Martin on 28/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef ATOM_HPP
#define ATOM_HPP

#include <ostream>

namespace expr { namespace atom {
    using variable = int;

    struct equality {
        variable left, right;
    };

    inline bool operator ==(const equality & a, const equality & b) {
        return a.left == b.left && a.right == b.right;
    }

    inline std::ostream & operator <<(std::ostream & stream, const equality & exp) {
        return stream << exp.left << " = " << exp.right;
    }
} }

#endif
