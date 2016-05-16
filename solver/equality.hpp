//
//  equality.hpp
//  solver
//
//  Created by Alexandre Martin on 13/05/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef EQUALITY_HPP
#define EQUALITY_HPP

#include "curry.hpp"
#include <boost/functional/hash/hash.hpp>
#include <vector>
#include <unordered_map>

class equality_solver {
private:
    struct dummy { };
    using edge = boost::variant<const_equality, std::pair<curry_equality, curry_equality>, dummy>;

    struct record {
        std::vector<std::vector<curry_equality>> use_lists;

        std::unordered_map<
            std::pair<int, int>,
            curry_equality,
            boost::hash<std::pair<int, int>>
        > lookup_table;

        std::vector<int> representative;
        std::vector<std::vector<int>> class_lists;
        std::vector<std::unordered_set<int>> disequalities;
        std::vector<std::pair<int, edge>> proof_forest;

        record(int max_const) : representative(max_const),
                                class_lists(max_const),
                                use_lists(max_const),
                                disequalities(max_const),
                                proof_forest(max_const, std::make_pair(-1, dummy { })) { }
    };

    std::vector<edge> m_pending;
    std::vector<curry_atom> m_atoms;
    std::vector<record> m_db;

    bool propagate(std::vector<int> &);
public:
    equality_solver(int, std::vector<curry_atom>);

    std::pair<std::vector<int>, bool> set_true(int);
    void backtrack_one();
};

#endif
