#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "func.hpp"
#include <map>
#include <typeindex>

// contains all functions of all types
struct scope
{
    // contains all functions of a given type
    struct entry
    {
        std::list<std::list<func>::iterator> m_nullaries;
        std::list<std::list<func>::iterator> m_non_nullaries;
        // adds a function based on its arity
        void add_function(std::list<func>::iterator a_function);
    };
    // contains all in-scope functions
    std::map<std::type_index, entry> m_entries;
    // adds a function based on its return type and its arity
    void add_function(std::list<func>::iterator a_function);
};

#endif
