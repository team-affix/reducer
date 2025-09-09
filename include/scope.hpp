#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "func.hpp"
#include <map>
#include <typeindex>
#include <vector>

// contains all functions of a given type
struct scope_entry_t
{
    std::vector<const func_t*> m_nullaries;
    std::vector<const func_t*> m_non_nullaries;
};

// contains all functions of all types
struct scope_t
{
    std::map<std::type_index, scope_entry_t> m_entries;
    // checks that all types have >=1 nullary functions
    void validate() const;
};

#endif
