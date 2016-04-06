//
//  clause.hpp
//  solver
//
//  Created by Alexandre Martin on 03/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef CLAUSE_HPP
#define CLAUSE_HPP

#include <vector>

namespace detail {
    enum class polarity;
    using assignment = std::vector<std::pair<polarity, size_t>>; // (polarity, level)

    class clause {
    private:
        std::vector<int> m_litterals;
        size_t m_id = 0;
        int m_satisfied_by = -1;
        int m_count = 0;

    public:
        clause() = default;
        clause(clause &&) = default;
        clause & operator =(clause &&) = default;

        clause(size_t capacity) {
            m_litterals.reserve(capacity);
        }

        const std::vector<int> & litterals() const {
            return m_litterals;
        }

        void set_id(size_t id) {
            m_id = id;
        }

        void satisfy(int lit) {
            m_satisfied_by = lit;
        }

        int satisfied_by() const {
            return m_satisfied_by;
        }

        size_t id() const {
            return m_id;
        }

        void emplace(int lit) {
            m_litterals.emplace_back(lit);
            ++m_count;
        }

        void increase_count() {
            ++m_count;
        }

        void decrease_count() {
            --m_count;
        }

        int count() const {
            return m_count;
        }

        int watch_0() const {
            return m_litterals[0];
        }

        int watch_1() const {
            return m_litterals[1];
        }

        void swap_watches() {
            std::swap(m_litterals[0], m_litterals[1]);
        }

        void build_watches(std::vector<std::vector<clause*>> & watches) {
            for (auto && l : m_litterals)
                watches[l].push_back(this);
        }

        int first_unassigned(const assignment &) const;
        void update_counts(const assignment &, std::vector<size_t> &) const;
        int make_new_watch(const assignment & a);
    };
}

#endif
