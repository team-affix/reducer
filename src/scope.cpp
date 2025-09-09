#include "../include/scope.hpp"
#include <stdexcept>

// checks that all types have at least one nullary
void scope_t::validate() const
{
    for(const auto& [type_index, entry] : m_entries)
    {
        if(entry.m_nullaries.empty())
        {
            throw std::runtime_error(
                "Type " + std::string(type_index.name()) +
                " has no nullaries");
        }
    }
}
