//
//  equality.cpp
//  solver
//
//  Created by Alexandre Martin on 13/05/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#include "equality.hpp"

equality_solver::equality_solver(int max_const,
                                 std::vector<curry_atom> atoms) : m_atoms(std::move(atoms)) {
    m_db.emplace_back(max_const);
    auto && db = m_db.back();

    for (auto i = 0; i < max_const; ++i) {
        db.representative[i] = i;
        db.class_lists[i].emplace_back(i);
    }
}

bool equality_solver::propagate(std::vector<int> & consequences) {
    bool conflict = false;
    auto && db = m_db.back();

    while (!m_pending.empty()) {
        auto E = m_pending.back();
        m_pending.pop_back();
        int a, b;
        if (auto eq = boost::get<const_equality>(&E)) {
            a = eq->left;
            b = eq->right;
        } else if (auto eq = boost::get<std::pair<curry_equality, curry_equality>>(&E)) {
            a = eq->first.right;
            b = eq->second.right;
        }

        auto repr_a = db.representative[a], repr_b = db.representative[b];

        if (db.disequalities[repr_a].find(repr_b) != db.disequalities[repr_a].end())
            conflict = true;

        if (repr_a != repr_b) {
            if (db.class_lists[repr_a].size() > db.class_lists[repr_b].size()) {
                std::swap(a, b);
                std::swap(repr_a, repr_b);
            }

            auto old_repr_a = repr_a;
            db.proof_forest[a] = std::make_pair(b, E);

            for (auto && c : db.class_lists[old_repr_a]) {
                db.representative[c] = repr_b;
                db.class_lists[repr_b].emplace_back(c);
            }
            db.class_lists[old_repr_a].clear();

            for (auto && eq : db.use_lists[old_repr_a]) {
                auto p = std::make_pair(db.representative[eq.arg1], db.representative[eq.arg2]);
                auto it = db.lookup_table.find(p);
                if (it != db.lookup_table.end())
                    m_pending.emplace_back(std::make_pair(eq, it->second));
                else {
                    db.lookup_table.emplace(p, eq);
                    db.use_lists[repr_b].emplace_back(eq);
                }
            }
            db.use_lists[old_repr_a].clear();

            for (auto && c : db.disequalities[old_repr_a]) {
                db.disequalities[repr_b].insert(c);
                db.disequalities[c].erase(db.disequalities[c].find(old_repr_a));
                db.disequalities[c].insert(repr_b);
            }
            db.disequalities[old_repr_a].clear();
        }
    }
    return conflict;
}

std::pair<std::vector<int>, bool> equality_solver::set_true(int lit) {
    if (std::abs(lit) > m_atoms.size()) // lit from Tseitin
        return std::make_pair(std::vector<int> { }, false);

    std::vector<int> consequences;
    bool conflict = false;
    m_db.emplace_back(m_db.back());
    auto && db = m_db.back();

    if (lit > 0) {
        auto && at = m_atoms[lit - 1];
        if (auto eq = boost::get<const_equality>(&at)) {
            m_pending.emplace_back(*eq);
            conflict = propagate(consequences);
        } else if (auto eq = boost::get<curry_equality>(&at)) {
            auto repr1 = db.representative[eq->arg1], repr2 = db.representative[eq->arg2];
            auto p = std::make_pair(repr1, repr2);
            auto it = db.lookup_table.find(p);
            if (it != db.lookup_table.end()) {
                m_pending.emplace_back(std::make_pair(*eq, it->second));
                conflict = propagate(consequences);
            } else { // conflict cannot take place here
                db.lookup_table.emplace(p, *eq);
                db.use_lists[repr1].emplace_back(*eq);
                db.use_lists[repr2].emplace_back(*eq);
            }
        }
    } else {
        auto eq = boost::get<const_equality>(&m_atoms[-lit - 1]);
        assert(eq != nullptr);
        auto repr_a = db.representative[eq->left], repr_b = db.representative[eq->right];
        if (repr_a == repr_b)
            conflict = true;
        db.disequalities[repr_a].insert(repr_b);
        db.disequalities[repr_b].insert(repr_a);
    }

    return std::make_pair(std::move(consequences), conflict);
}

void equality_solver::backtrack_one(int lit) {
    if (std::abs(lit) > m_atoms.size())
        return;

    m_db.pop_back();
}