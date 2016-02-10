/*
 *  expr/detail/logical_expr.hpp
 *  solver
 *
 *  Created by Alexandre Martin on 03/02/2016.
 *  Copyright © 2016 scalexm. All rights reserved.
 *
 */

#ifndef DETAIL_LOGICAL_EXPR_HPP
#define DETAIL_LOGICAL_EXPR_HPP

#include "../logical_expr.hpp"

namespace expr { namespace detail {
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

    class eval_visitor : public boost::static_visitor<bool> {
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

    class simplify_visitor : public boost::static_visitor<logical_expr> {
    public:
        logical_expr operator ()(none & exp) const {
            return std::move(exp);
        }

        template<class T>
        logical_expr operator ()(logical_binary<T> & exp) const {
            exp.op_left = boost::apply_visitor(simplify_visitor { }, exp.op_left);
            exp.op_right = boost::apply_visitor(simplify_visitor { }, exp.op_right);
            return std::move(exp);
        }

        logical_expr operator ()(variable & v) const {
            return std::move(v);
        }

        logical_expr operator ()(logical_not & exp) const {
            exp.op = boost::apply_visitor(simplify_visitor { }, exp.op);
            if (auto v = boost::get<int>(&exp.op)) { // ~v is -v
                return make(-*v);
            } else if (auto e = boost::get<logical_not>(&exp.op)) { // ~~F is F
                return std::move(e->op);
            }
            return std::move(exp);
        }
    };
} }

#endif