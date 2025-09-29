#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include "type.hpp"
#include <string>

struct symbol
{
    // the representation of the symbol
    std::string m_repr;

    // the type of the symbol
    type m_type;

    // only constructor
    symbol(const std::string& a_repr, const type& a_type);
};

bool operator<(const symbol& a_lhs, const symbol& a_rhs);
bool operator==(const symbol& a_lhs, const symbol& a_rhs);

#endif
