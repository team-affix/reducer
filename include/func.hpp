#ifndef FUNC_HPP
#define FUNC_HPP

#include <any>
#include <functional>
#include <list>
#include <typeindex>

// contains total information about a function
struct func_t
{
    // the parameters
    std::list<std::type_index> m_param_types;
    std::list<std::any>::iterator m_params;

    // the definition of the function
    std::function<std::any(std::list<std::any>)> m_functor;
    std::list<func_t> m_children;

    // evaluation functions
    std::any eval() const;
    std::any operator()(const std::list<std::any>&) const;

    // node count
    size_t node_count() const;
};

// // output operator
// std::ostream& operator<<(std::ostream& a_os,
//                          const func_t& a_func);

#endif
