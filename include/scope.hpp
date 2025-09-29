#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "symbol.hpp"
#include <any>
#include <map>
#include <memory>

struct scope
{
    // the adjacent scopes
    const scope* m_parent;
    std::vector<scope> m_children;

    // the objects in the scope
    std::map<symbol, std::shared_ptr<std::any>> m_constants;

    // lone constructor
    scope(const scope* a_parent);

    // query functions

    // mutation functions
    void add_constant(const symbol& a_symbol, const std::any& a_value);
};

#endif
