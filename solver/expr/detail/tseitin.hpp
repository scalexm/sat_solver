/*
 *  expr/detail/tseitin.hpp
 *  solver
 *
 *  Created by Nicolas Levy on 12/02/2016.
 *  Copyright © 2016 NicolasPierreLevy. All rights reserved.
 *
 */

#ifndef DETAIL_TSEITIN_HPP
#define DETAIL_TSEITIN_HPP

#include "../logical_expr.hpp"

namespace expr { namespace detail {
    template<class Tag>
    void de_morgan(atom::variable, atom::variable, atom::variable, cnf &) { }

    template<>
    void de_morgan<and_tag>(atom::variable x, atom::variable a, atom::variable b, cnf & t) {
        t.push_back({ -x, a });
        t.push_back({ -x, b });
        t.push_back({ x, -a, -b });
    }

    template<>
    void de_morgan<or_tag>(atom::variable x, atom::variable a, atom::variable b, cnf & t) {
        t.push_back({ -x, a, b });
        t.push_back({ x, -a });
        t.push_back({ x, -b });
    }

    template<>
    void de_morgan<impl_tag>(atom::variable x, atom::variable a, atom::variable b, cnf & t) {
        t.push_back({ -x, -a, b });
        t.push_back({ x, a });
        t.push_back({ x, -b });
    }

    template<>
    void de_morgan<xor_tag>(atom::variable x, atom::variable a, atom::variable b, cnf & t) {
        t.push_back({ -x, a, b });
        t.push_back({ -x, -a, -b });
        t.push_back({ x, -a, b });
        t.push_back({ x, a, -b });
    }

    template<>
    void de_morgan<equiv_tag>(atom::variable x, atom::variable a, atom::variable b, cnf & t) {
        de_morgan<impl_tag>(x, a, b, t);
        de_morgan<impl_tag>(x, b, a, t);
    }

    class tseitin_visitor : public boost::static_visitor<atom::variable> {
    private:
        atom::variable & current_variable; //name of the next fresh variable
        cnf & result;

    public:
        tseitin_visitor(atom::variable & cv, cnf & ret) : current_variable(cv), result(ret) { }

        atom::variable operator ()(const none &) {
            return -1;
        }


        atom::variable operator ()(const atom::variable & v) {
            return v;
        }

        template<class Tag>
        atom::variable operator ()(const logical_binary<Tag> & ex) {
            auto v = tseitin_visitor { current_variable, result };
            auto left = boost::apply_visitor(v, ex.op_left);
            auto right = boost::apply_visitor(v, ex.op_right);

            auto fresh = current_variable;
            de_morgan<Tag>(fresh, left, right, result);
            ++current_variable;
            return fresh;
        }

        atom::variable operator ()(const logical_not & ex){
            auto v = tseitin_visitor { current_variable, result };
            auto arg = boost::apply_visitor(v, ex.op);

            auto fresh = current_variable;
            ++current_variable;

            result.push_back({ -fresh, -arg });
            result.push_back({ fresh, arg });
            return fresh;
        }
    };

   //looking for the first name of the fresh variable
    class find_max_variable : public boost::static_visitor<atom::variable> {
    public:
        find_max_variable() { }

        atom::variable operator ()(const none &) const {
            return 0;
        }

        atom::variable operator ()(const atom::variable & v) const {
            return std::abs(v);
        }

        template<class Tag>
        atom::variable operator ()(const logical_binary<Tag> & ex) const {
            return std::max(
                boost::apply_visitor(find_max_variable {}, ex.op_left),
                boost::apply_visitor(find_max_variable {}, ex.op_right)
            );
        }

        atom::variable operator ()(const logical_not & ex) const {
            return boost::apply_visitor(find_max_variable { }, ex.op);
        }
    };
} }

#endif