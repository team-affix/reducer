#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "func.hpp"
#include <map>
#include <typeindex>

// contains all functions of a given type
struct scope_entry_t
{
    std::list<const func_t*> m_nullaries;
    std::list<const func_t*> m_non_nullaries;
    // adds a function based on its arity
    void add_function(const func_t* a_function);
    // checks that the nullaries are not empty
    void validate() const;
};

// contains all functions of all types
struct scope_t
{
    std::map<std::type_index, scope_entry_t> m_entries;
    // adds a function based on its return type
    void add_function(std::type_index a_return_type,
                      const func_t* a_function);
    // checks that all types have >=1 nullary functions
    void validate() const;
};

#endif
