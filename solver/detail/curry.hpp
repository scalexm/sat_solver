//
//  curry.hpp
//  solver
//
//  Created by Alexandre Martin on 11/05/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef DETAIL_CURRY_HPP
#define DETAIL_CURRY_HPP

#include "../curry.hpp"
#include "../expr/detail/logical_expr.hpp"
#include "../expr/theory_expr.hpp"
#include <sstream>

namespace detail {

    constexpr char CURRY_SYMBOL = '$';

    class currying_visitor : public boost::static_visitor<expr::atom::term> {
    private:
        std::unordered_map<char, int> & m_fun_seen;
        std::unordered_map<int, int> & m_var_seen;
        int & m_current_index;

    public:
        currying_visitor(std::unordered_map<char, int> & fun_seen,
                        std::unordered_map<int, int> & var_seen,
                        int & current_index) : m_fun_seen(fun_seen),
                                               m_var_seen(var_seen),
                                               m_current_index(current_index) { }

        expr::atom::term operator ()(const expr::atom::fun & f) {
            auto it = m_fun_seen.find(f.name);
            int ind;
            if (it == m_fun_seen.end()) {
                ind = m_current_index;
                ++m_current_index;
                m_fun_seen.emplace(f.name, ind);
            } else
                ind = it->second;

            if (f.args.size() == 0)
                return ind;
            else {
                auto i = 0;
                expr::atom::fun curry;
                do {
                    currying_visitor v { m_fun_seen, m_var_seen, m_current_index };
                    auto curry_arg = boost::apply_visitor(v, f.args[i]);
                    curry.name = CURRY_SYMBOL;
                    curry.args = {
                        curry.args.empty() ?
                            expr::atom::term { ind } : expr::atom::term { std::move(curry) },
                        std::move(curry_arg)
                    };
                    ++i;
                } while (i < f.args.size());
                return curry;
            }
        }

        expr::atom::term operator ()(const expr::atom::variable & v) {
            auto it = m_var_seen.find(v);
            if (it == m_var_seen.end()) {
                auto ind = m_current_index;
                ++m_current_index;
                m_var_seen.emplace(v, ind);
                return ind;
            } else
                return it->second;
        }
    };

    class flatten_visitor : public boost::static_visitor<expr::detail::expr_<curry_atom>> {
    };

    class curry_to_logical_visitor : public boost::static_visitor<expr::logical_expr> {
    private:
        std::unordered_map<std::string, int> & m_seen;
        std::vector<curry_atom> & m_atoms;

    public:
        curry_to_logical_visitor(std::unordered_map<std::string, int> & seen,
                                 std::vector<curry_atom> & atoms) : m_seen(seen),
                                                                    m_atoms(atoms) { }

        expr::logical_expr operator ()(const expr::none &) {
            return expr::none { };
        }

        template<class Tag>
        expr::logical_expr operator ()(const expr::detail::binary_<Tag, curry_atom> & exp) {
            curry_to_logical_visitor v_left { m_seen, m_atoms };
            auto left = boost::apply_visitor(v_left, exp.op_left);

            curry_to_logical_visitor v_right { m_seen, m_atoms };
            auto right = boost::apply_visitor(v_right, exp.op_right);

            return expr::logical_binary<Tag> { std::move(left), std::move(right) };
        }

        expr::logical_expr operator ()(const expr::detail::not_<curry_atom> & exp) {
            curry_to_logical_visitor v { m_seen, m_atoms };
            return expr::logical_not { boost::apply_visitor(v, exp.op) };
        }

        expr::logical_expr operator ()(const curry_atom & at) {
            std::ostringstream ss;
            ss << at;
            auto str_repr = ss.str();
            auto it = m_seen.find(str_repr);
            int ind;
            if (it == m_seen.end()) {
                ind = m_atoms.size();
                m_atoms.emplace_back(at);
                m_seen.emplace(std::move(str_repr), ind);
            } else
                ind = it->second;
            return expr::make(ind);
        }
    };

}

#endif
