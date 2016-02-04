/*
 *  logical_expr.hpp
 *  expr
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#ifndef LOGICAL_EXPR_HPP
#define LOGICAL_EXPR_HPP

#include <boost/variant.hpp>
#include <ostream>
#include <unordered_map>

namespace expr {
    using valuation = std::unordered_map<int, bool>;

    struct and_tag { };
    struct or_tag { };
    struct xor_tag { };
    struct impl_tag { };
    struct eq_tag { };

    struct none { };
    using variable = int;

    struct logical_not;
    template<class T> struct logical_binary;
    using logical_and = logical_binary<and_tag>;
    using logical_or = logical_binary<or_tag>;
    using logical_xor = logical_binary<xor_tag>;
    using logical_impl = logical_binary<impl_tag>;
    using logical_eq = logical_binary<eq_tag>;

    using logical_expr = boost::variant<
        none, // none in first position for default ctor
        variable,
        boost::recursive_wrapper<logical_not>,
        boost::recursive_wrapper<logical_and>,
        boost::recursive_wrapper<logical_or>,
        boost::recursive_wrapper<logical_xor>,
        boost::recursive_wrapper<logical_impl>,
        boost::recursive_wrapper<logical_eq>
    >;

    using expr_result = boost::variant<logical_expr, std::string>;

    template<class T>
    struct logical_binary {
        logical_expr op_left, op_right;
    };

    struct logical_not {
        logical_expr op;
    };

    inline bool operator ==(const none &, const none &) {
        return true;
    }

    inline bool operator ==(const logical_not & a, const logical_not & b) {
        return a.op == b.op;
    }

    template<class T>
    inline bool operator ==(const logical_binary<T> & a, const logical_binary<T> & b) {
        return a.op_left == b.op_left && a.op_right == b.op_right;
    }

    template<class T, class U>
    inline bool operator ==(const logical_binary<T> &, const logical_binary<U> &) {
        return false;
    }

    template<class T>
    inline logical_expr make_expr(T exp) {
        return logical_expr { exp };
    }
}


/* --- detail --- */

namespace expr { namespace detail {
    template<class T>
    inline void print_connector(std::ostream &) {
    }

    template<>
    inline void print_connector<and_tag>(std::ostream & stream) {
        stream << " /\\ ";
    }

    template<>
    inline void print_connector<or_tag>(std::ostream & stream) {
        stream << " \\/ ";
    }

    template<>
    inline void print_connector<xor_tag>(std::ostream & stream) {
        stream << " X ";
    }

    template<>
    inline void print_connector<impl_tag>(std::ostream & stream) {
        stream << " => ";
    }

    template<>
    inline void print_connector<eq_tag>(std::ostream & stream) {
        stream << " <=> ";
    }

    class print_visitor : boost::static_visitor<> {
    private:
        std::ostream & m_stream;

    public:
        print_visitor(std::ostream & stream) : m_stream(stream) { }

        std::ostream & operator ()(const none &) const {
            return m_stream << "[none]";
        }

        template<class T>
        std::ostream & operator ()(const logical_binary<T> & exp) const {
            m_stream << "(";
            boost::apply_visitor(print_visitor { m_stream }, exp.op_left);
            print_connector<T>(m_stream);
            boost::apply_visitor(print_visitor { m_stream }, exp.op_right);
            return m_stream << ")";
        }

        std::ostream & operator ()(const variable & v) const {
            return m_stream << v;
        }

        std::ostream & operator ()(const logical_not & exp) const {
            m_stream << "~(";
            boost::apply_visitor(print_visitor { m_stream }, exp.op);
            return m_stream << ")";
        }
    };

    template<class T>
    inline bool eval_connector(bool, bool) {
        return false;
    }

    template<>
    inline bool eval_connector<and_tag>(bool a, bool b) {
        return a && b;
    }

    template<>
    inline bool eval_connector<or_tag>(bool a, bool b) {
        return a || b;
    }

    template<>
    inline bool eval_connector<xor_tag>(bool a, bool b) {
        return (a && !b) || (!a && b);
    }

    template<>
    inline bool eval_connector<impl_tag>(bool a, bool b) {
        return !a || b;
    }

    template<>
    inline bool eval_connector<eq_tag>(bool a, bool b) {
        return (!a || b) && (!b || a);
    }

    class eval_visitor : boost::static_visitor<bool> {
    private:
        const valuation & m_val;

    public:
        eval_visitor(const valuation & val) : m_val (val) { }

        bool operator ()(const none &) const {
            return true;
        }

        template<class T>
        bool operator ()(const logical_binary<T> & exp) const {
            return eval_connector<T>(
                boost::apply_visitor(eval_visitor { m_val }, exp.op_left),
                boost::apply_visitor(eval_visitor { m_val }, exp.op_right)
            );
        }

        bool operator ()(const variable & v) const {
            auto it = m_val.find(v);
            if (it == m_val.end())
                return false;
            return it->second;
        }
        
        bool operator ()(const logical_not & exp) const {
            return !boost::apply_visitor(eval_visitor { m_val }, exp.op);
        }
    };
} }

namespace expr {
    inline bool eval(const logical_expr & exp, const valuation & val) {
        return boost::apply_visitor(detail::eval_visitor { val }, exp);
    }

    expr_result parse(const std::string &);

    inline std::ostream & operator <<(std::ostream & stream, const expr::logical_expr & exp) {
        return boost::apply_visitor(expr::detail::print_visitor { stream }, exp);
    }
}

#endif
