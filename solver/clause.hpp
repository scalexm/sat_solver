//
//  clause.hpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef CLAUSE_HPP
#define CLAUSE_HPP

#include <unordered_set>

namespace detail {
    class clause {
    private:
        std::unordered_set<int> m_litterals;
        int m_satisfied_by = 0;

    public:
        clause(std::unordered_set<int> litterals) : m_litterals { std::move(litterals) } { }

        const std::unordered_set<int> & litterals() const {
            return m_litterals;
        }

        void satisfy(int lit) {
            m_satisfied_by = lit;
        }

        int is_satisfied() const {
            return m_satisfied_by;
        }

        void remove(int lit) {
            auto it = m_litterals.find(lit);
            if (it != m_litterals.end())
                m_litterals.erase(it);
        }

        void add(int lit) {
            m_litterals.emplace(lit);
        }

        bool empty() const {
            return m_litterals.empty();
        }
    };
}

#endif