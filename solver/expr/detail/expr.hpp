//
//  expr/detail/expr.hpp
//  solver
//
//  Created by Alexandre Martin on 28/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef DETAIL_EXPR_HPP
#define DETAIL_EXPR_HPP

#include <ostream>
#include <boost/variant.hpp>

namespace expr { namespace detail {
    /*
        tags for the template struct binary: each tag identifies a binary logical operator
    */
    struct and_tag {
        constexpr static const char * text = "/\\";
    };

    struct or_tag {
        constexpr static const char * text = "\\/";
    };

    struct xor_tag {
        constexpr static const char * text = "X";
    };

    struct impl_tag {
        constexpr static const char * text = "=>";
    };

    struct equiv_tag {
        constexpr static const char * text = "<=>";
    };

    struct none_ { };
    template<class Atom> struct not_;
    template<class Tag, class Atom> struct binary_;

    template<class Atom>
    using and_ = binary_<detail::and_tag, Atom>;

    template<class Atom>
    using or_ = binary_<detail::or_tag, Atom>;

    template<class Atom>
    using xor_ = binary_<detail::xor_tag, Atom>;

    template<class Atom>
    using impl_ = binary_<detail::impl_tag, Atom>;

    template<class Atom>
    using equiv_ = binary_<detail::equiv_tag, Atom>;

    template<class Atom>
    using expr_ = boost::variant<
        none_, // none in first position for default ctor
        Atom,
        boost::recursive_wrapper<not_<Atom>>,
        boost::recursive_wrapper<and_<Atom>>,
        boost::recursive_wrapper<or_<Atom>>,
        boost::recursive_wrapper<xor_<Atom>>,
        boost::recursive_wrapper<impl_<Atom>>,
        boost::recursive_wrapper<equiv_<Atom>>
    >;

    template<class Tag, class Atom>
    struct binary_ {
        expr_<Atom> op_left, op_right;
    };

    template<class Atom>
    struct not_ {
        expr_<Atom> op;
    };

    inline bool operator ==(const none_ &, const none_ &) {
        return true;
    }

    template<class Atom>
    inline bool operator ==(const not_<Atom> & a, const not_<Atom> & b) {
        return a.op == b.op;
    }

    template<class Tag, class Atom>
    inline bool operator ==(const binary_<Tag, Atom> & a, const binary_<Tag, Atom> & b) {
        return a.op_left == b.op_left && a.op_right == b.op_right;
    }

    inline std::ostream & operator <<(std::ostream & stream, const none_ &) {
        return stream << "[none]";
    }

    template<class Atom>
    inline std::ostream & operator <<(std::ostream & stream, const not_<Atom> & exp) {
        return stream << "~(" << exp.op << ")";
    }

    template<class Tag, class Atom>
    inline std::ostream & operator <<(std::ostream & stream, const binary_<Tag, Atom> & exp) {
        return stream << "(" << exp.op_left << " " << Tag::text << " " << exp.op_right << ")";
    }
} }


#endif
