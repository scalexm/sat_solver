//
//  detail/solver.hpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef DETAIL_SOLVER_HPP
#define DETAIL_SOLVER_HPP

#include <cassert>
#include <queue>

namespace detail {
    enum class polarity {
        VTRUE,
        VFALSE,
        VUNDEF,
    };

    using assignment = std::vector<std::pair<polarity, size_t>>; // (polarity, level)

    inline bool sign(int lit) {
        return lit & 1;
    }

    inline int var(int lit) {
        return lit >> 1;
    }

    inline int neg(int lit) {
        return lit ^ 1;
    }

    inline int lit(int var, bool sign) {
        return var + var + (sign ? 1 : 0);
    }

    inline polarity polarity_lit(const assignment & a, int lit) {
        auto var_polarity = a[var(lit)].first;
        if (var_polarity == polarity::VUNDEF)
            return polarity::VUNDEF;
        else if (sign(lit))
            return var_polarity;
        else
            return var_polarity == polarity::VTRUE ? polarity::VFALSE : polarity::VTRUE;
    }

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

        int first_unassigned(const assignment & a) const {
            for (auto && lit : m_litterals) {
                if (a[detail::var(lit)].first == polarity::VUNDEF)
                    return lit;
            }
            return -1;
        }

        void update_counts(const assignment & a, std::vector<size_t> & counts) const {
            for (auto && lit : m_litterals) {
                if (a[detail::var(lit)].first == polarity::VUNDEF)
                    ++counts[lit];
            }
        }

        void swap_watches() {
            std::swap(m_litterals[0], m_litterals[1]);
        }

        void build_watches(std::vector<std::vector<clause*>> & watches) {
            for (auto && l : m_litterals)
                watches[l].push_back(this);
        }

        int make_new_watch(const assignment & a) {
            int found = -1;
            for (auto i = 2; i < m_litterals.size(); ++i) {
                if (polarity_lit(a, m_litterals[i]) != polarity::VFALSE) {
                    std::swap(m_litterals[1], m_litterals[i]);
                    found = m_litterals[1];
                    break;
                }
            }
            return found;
        }
    };
}

#endif