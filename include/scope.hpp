#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "func.hpp"
#include <map>
#include <typeindex>

// contains all functions of all types
struct scope
{
    std::multimap<std::type_index, func*> m_nullaries;
    std::multimap<std::type_index, func*> m_non_nullaries;
    // adds a function based on its return type and its arity
    void add_function(func* a_function);
};

#endif
