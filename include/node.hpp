#ifndef NODE_HPP
#define NODE_HPP

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <tuple>

template<typename DATA>
struct node
{
    DATA m_data;
    std::vector<node> m_children;
    // node(const DATA& a_data, const std::vector<node>& a_children) :
    //     m_data(a_data), m_children(a_children) {}
    bool operator==(const node& a_other) const
    {
        return
            std::tie(m_data, m_children) ==
            std::tie(a_other.m_data, a_other.m_children);
    }
};

#endif
