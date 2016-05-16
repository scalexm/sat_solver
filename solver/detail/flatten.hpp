//
//  detail/flatten.hpp
//  solver
//
//  Created by Alexandre Martin on 13/05/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef DETAIL_FLATTEN_HPP
#define DETAIL_FLATTEN_HPP

#include "../curry.hpp"
#include "../expr/theory_expr.hpp"
#include <sstream>

namespace detail {

    inline bool is_flat_fun(const expr::atom::fun & f) {
        assert(f.args.size() == 2);
        return boost::get<expr::atom::variable>(&f.args[0]) != nullptr &&
               boost::get<expr::atom::variable>(&f.args[1]) != nullptr;
    }

    class term_flatten_visitor : public boost::static_visitor<expr::atom::variable> {
    private:
        std::unordered_map<std::string, int> & m_already_flatten;
        std::vector<curry_atom> & m_new_eqs;
        int & m_current_index;

    public:
        term_flatten_visitor(std::unordered_map<std::string, int> & already_flatten,
                             std::vector<curry_atom> & m_new_eqs,
                             int & current_index) : m_already_flatten(already_flatten),
                                                    m_new_eqs(m_new_eqs),
                                                    m_current_index(current_index) { }

        expr::atom::variable operator ()(const expr::atom::variable & v) {
            return v;
        }

        expr::atom::variable operator ()(expr::atom::fun f) {
            if (!is_flat_fun(f)) {
                term_flatten_visitor v { m_already_flatten, m_new_eqs, m_current_index };
                f.args[0] = boost::apply_visitor(v, f.args[0]);
                f.args[1] = boost::apply_visitor(v, f.args[1]);
            }

            assert(is_flat_fun(f));
            std::ostringstream ss;
            ss << f;
            auto str_repr = ss.str();
            auto it = m_already_flatten.find(str_repr);
            if (it == m_already_flatten.end()) {
                auto ind = m_current_index;
                ++m_current_index;
                m_already_flatten.emplace(std::move(str_repr), ind);
                m_new_eqs.emplace_back(curry_equality {
                    boost::get<expr::atom::variable>(f.args[0]),
                    boost::get<expr::atom::variable>(f.args[1]),
                    ind
                });
                return ind;
            } else
                return it->second;
        }
    };

    class flatten_visitor : public boost::static_visitor<expr::detail::expr_<curry_atom>> {
    private:
        std::unordered_map<std::string, int> & m_already_flatten;
        std::vector<curry_atom> & m_new_eqs;
        int & m_current_index;

    public:
        flatten_visitor(std::unordered_map<std::string, int> & already_flatten,
                        std::vector<curry_atom> & m_new_eqs,
                        int & current_index) : m_already_flatten(already_flatten),
                                               m_new_eqs(m_new_eqs),
                                               m_current_index(current_index) { }

        expr::detail::expr_<curry_atom> operator ()(const expr::none &) {
            return expr::none { };
        }

        template<class Tag>
        expr::detail::expr_<curry_atom> operator ()(const expr::equality_binary<Tag> & exp) {
            flatten_visitor v { m_already_flatten, m_new_eqs, m_current_index };
            return expr::detail::binary_<Tag, curry_atom> {
                boost::apply_visitor(v, exp.op_left),
                boost::apply_visitor(v, exp.op_right)
            };
        }

        expr::detail::expr_<curry_atom> operator ()(const expr::equality_not & exp) {
            flatten_visitor v { m_already_flatten, m_new_eqs, m_current_index };
            return expr::detail::not_<curry_atom> {
                boost::apply_visitor(v, exp.op)
            };
        }

        expr::detail::expr_<curry_atom> operator ()(const expr::atom::equality & eq) {
            /*if (boost::get<expr::atom::variable>(&eq.left) != nullptr)
                std::swap(eq.left, eq.right);*/

            term_flatten_visitor v { m_already_flatten, m_new_eqs, m_current_index };
            return const_equality {
                boost::apply_visitor(v, eq.left),
                boost::apply_visitor(v, eq.right)
            };
            /*auto rh = boost::apply_visitor(v, eq.right);

            if (auto f = boost::get<expr::atom::fun>(&eq.left)) {
                if (!is_flat_fun(*f)) {
                    term_flatten_visitor v { m_already_flatten, m_new_eqs, m_current_index };
                    f->args[0] = boost::apply_visitor(v, f->args[0]);
                    f->args[1] = boost::apply_visitor(v, f->args[1]);
                }

                assert(is_flat_fun(*f));

                std::ostringstream ss;
                ss << f;
                auto it = m_already_flatten.find(ss.str());
                if (it == m_already_flatten.end()) {
                    return curry_equality {
                        boost::get<expr::atom::variable>(f->args[0]),
                        boost::get<expr::atom::variable>(f->args[1]),
                        rh
                    };
                } else
                    return const_equality { it->second, rh };
            } else
                return const_equality { boost::get<expr::atom::variable>(eq.left), rh };*/
        }
    };

}
#endif
