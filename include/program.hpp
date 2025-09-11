#ifndef ENV_HPP
#define ENV_HPP

#include "../include/func.hpp"
#include <any>
#include <list>

struct program
{
    std::list<func> m_funcs;
    std::list<std::any> m_param_heap;

    // adding functions
    std::list<func>::const_iterator
    add_parameter(std::list<std::any>::iterator& a_param_it,
                  const int a_param_index, const std::type_index& a_type);
    template <typename Ret, typename... Params>
    std::list<func>::const_iterator
    add_primitive(const std::string& a_repr,
                  const std::function<Ret(Params...)>& a_func);
};

#endif
