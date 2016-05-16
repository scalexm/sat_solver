//
//  detail/clause.hpp
//  solver
//
//  Created by Alexandre Martin on 03/04/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef DETAIL_CLAUSE_HPP
#define DETAIL_CLAUSE_HPP

#include <vector>
#include <cstddef>
#include <list>
#include <cassert>

namespace detail {
    enum class polarity;
    class clause;

    struct var_data {
        polarity pol;
        int level;
        clause * reason;
        std::list<int>::iterator it;
    };

    using assignment = std::vector<var_data>;

    class clause {
    private:
        std::vector<int> m_litterals;
        size_t m_id = 0;
        int m_satisfied_by = -1;
        int m_count = 0;
        bool m_learnt = false;
        int m_score = 0;
        int m_reason_count = 0;

    public:
        clause() = default;
        clause(clause &&) = default;
        clause & operator =(clause &&) = default;

        // set m_count to 1: use only for clause learning
        clause(std::vector<int> litterals) {
            m_litterals = std::move(litterals);
            m_count = 1;
            m_learnt = true;
        }

        void inc_score() {
            m_score += 1;
        }

        int score() const {
            return m_score;
        }

        bool is_reason() const {
            return m_reason_count > 0;
        }

        void inc_reason() {
            ++m_reason_count;
            assert(m_reason_count == 1);
        }

        void dec_reason() {
            --m_reason_count;
        }

        bool learnt() const {
            return m_learnt;
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
