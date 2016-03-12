//
//  detail/clause.hpp
//  solver
//
//  Created by Alexandre Martin on 07/02/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef CLAUSE_HPP
#define CLAUSE_HPP

#include <cassert>
#include <queue>

namespace detail {
    inline int lit_to_ind(int lit) {
        return lit > 0 ? (2 * (lit - 1)) : (2 * (-lit - 1) + 1);
    }

    inline int ind_to_lit(int ind) {
        return ind % 2 == 0 ? (ind / 2 + 1) : -((ind - 1) / 2 + 1);
    }

    class litterals_container {
    private:
        std::vector<bool> m_litterals;
        size_t m_2N;
        size_t m_count = 0;

    public:
        litterals_container(size_t N) : m_2N { 2 * N }, m_litterals(2 * N, false) { }
        litterals_container(litterals_container &&) = default;
        litterals_container(const litterals_container &) = default;
        litterals_container & operator =(const litterals_container &) = default;
        litterals_container & operator =(litterals_container &&) = default;

        void emplace(int lit) {
            assert(lit != 0);

            auto ind = lit_to_ind(lit);
            if (!m_litterals[ind])
                ++m_count;
            m_litterals[ind] = true;
        }

        void remove(int lit) {
            assert(lit != 0);

            auto ind = lit_to_ind(lit);
            if (m_litterals[ind])
                --m_count;
            m_litterals[ind] = false;
        }

        void update_counts(std::vector<size_t> & counts) const {
            for (auto i = 0; i < m_2N; ++i) {
                if (m_litterals[i])
                    ++counts[i];
            }
        }

        size_t size() const {
            return m_count;
        }

        bool empty() const {
            return size() == 0;
        }

        int first() const {
            assert(!empty());
            for (auto i = 0; i < m_2N; ++i) {
                if (m_litterals[i])
                    return ind_to_lit(i);
            }

            return 0; // not reached
        }
    };

    class clause {
    private:
        litterals_container m_litterals;
        int m_satisfied_by = 0;

    public:
        clause(litterals_container litterals) : m_litterals { std::move(litterals) } { }

        void satisfy(int lit) {
            m_satisfied_by = lit;
        }

        int is_satisfied() const {
            return m_satisfied_by;
        }

        void remove(int lit) {
            m_litterals.remove(lit);
        }

        void add(int lit) {
            m_litterals.emplace(lit);
        }

        bool empty() const {
            return m_litterals.empty();
        }

        void update_counts(std::vector<size_t> & counts) const {
            m_litterals.update_counts(counts);
        }

        size_t size() const {
            return m_litterals.size();
        }

        int first() const {
            return m_litterals.first();
        }
    };
}

#endif