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
    template<class T>
    void de_morgan(variable, variable, variable, cnf &) { }

    template<>
    void de_morgan<and_tag>(variable x, variable a, variable b, cnf & t) {
        t.push_back({ -x, a });
        t.push_back({ -x, b });
        t.push_back({ x, -a, -b });
    }

    template<>
    void de_morgan<or_tag>(variable x, variable a, variable b, cnf & t) {
        t.push_back({ -x, a, b });
        t.push_back({ x, -a });
        t.push_back({ x, -b });
    }

    template<>
    void de_morgan<impl_tag>(variable x, variable a, variable b, cnf & t) {
        t.push_back({ -x, -a, b });
        t.push_back({ x, a });
        t.push_back({ x, -b });
    }

    template<>
    void de_morgan<xor_tag>(variable x, variable a, variable b, cnf & t) {
        t.push_back({ -x, a, b });
        t.push_back({ -x, -a, -b });
        t.push_back({ x, -a, b });
        t.push_back({ x, a, -b });
    }

    template<>
    void de_morgan<eq_tag>(variable x, variable a, variable b, cnf & t) {
        de_morgan<impl_tag>(x, a, b, t);
        de_morgan<impl_tag>(x, b, a, t);
    }

    class tseitin_visitor : public boost::static_visitor<variable> {
    private:
        variable & current_variable; //name of the next fresh variable
        cnf & result;

    public:
        tseitin_visitor(variable & cv, cnf & ret) : current_variable(cv), result(ret) { }

        variable operator ()(const none &) {
            return -1;
        }


        variable operator ()(const variable & v) {
            return v;
        }

        template<class T>
        variable operator ()(const logical_binary<T> & ex) {
            auto v_left = tseitin_visitor { current_variable, result };
            auto left = boost::apply_visitor(v_left, ex.op_left);

            auto v_right = tseitin_visitor { current_variable, result };
            auto right = boost::apply_visitor(v_right, ex.op_right);

            auto fresh = current_variable;
            de_morgan<T>(fresh, left, right, result);
            ++current_variable;
            return fresh;
        }

        variable operator ()(const logical_not & ex){
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
    class find_max_variable : public boost::static_visitor<variable> {
    public:
        find_max_variable() { }

        variable operator ()(const none &) const {
            return 0;
        }

        variable operator ()(const variable & v) const {
            return std::abs(v);
        }

        template<class T>
        variable operator ()(const logical_binary<T> & ex) const {
            return std::max(
                boost::apply_visitor(find_max_variable {}, ex.op_left),
                boost::apply_visitor(find_max_variable {}, ex.op_right)
            );
        }

        variable operator ()(const logical_not & ex) const {
            return boost::apply_visitor(find_max_variable { }, ex.op);
        }
    };
} }

#endif