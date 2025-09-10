#ifndef FUNC_HPP
#define FUNC_HPP

#include <any>
#include <functional>
#include <list>
#include <ostream>
#include <string>
#include <typeindex>

// represents a function definition
struct func_node_t
{
    std::function<std::any(std::list<std::any>)> m_functor;
    std::list<func_node_t> m_children;
    std::any eval() const;
    size_t node_count() const;
};

// contains total information about a function
struct func_t
{
    // the parameters
    std::list<std::type_index> m_param_types;
    std::list<std::any>::iterator m_params;
    func_node_t m_definition;
    std::string m_repr;
    std::any operator()(const std::list<std::any>&) const;
};

#endif
