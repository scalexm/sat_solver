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
#include <boost/variant.hpp>
#include <vector>

namespace expr { namespace atom {
    using variable = int;

    struct fun;

    using term = boost::variant<variable, fun>;

    struct fun {
        char name;
        std::vector<term> args;
    };

    inline bool operator ==(const fun & a, const fun & b) {
        return a.name == b.name && a.args == b.args;
    }

    inline std::ostream & operator <<(std::ostream & stream, const fun & f) {
        stream << f.name;
        if (f.args.size() > 0) {
            stream << '(';
            for (auto i = 0; i < f.args.size(); ++i) {
                if (i != 0)
                    stream << ", ";
                stream << f.args[i];
            }
            stream << ')';
        }
        return stream;
    }

    struct equality {
        term left, right;
    };

    inline bool operator ==(const equality & a, const equality & b) {
        return a.left == b.left && a.right == b.right;
    }

    inline std::ostream & operator <<(std::ostream & stream, const equality & exp) {
        return stream << exp.left << " = " << exp.right;
    }
} }

#endif