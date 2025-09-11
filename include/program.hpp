#ifndef ENV_HPP
#define ENV_HPP

#include "../include/func.hpp"
#include <any>
#include <list>

struct program
{
    std::list<func> m_funcs;

    // adding functions
    std::list<func>::iterator add_parameter(std::list<func>::iterator a_func_it,
                                            std::type_index a_type);
    template <typename Ret, typename... Params>
    std::list<func>::iterator
    add_primitive(const std::string& a_repr,
                  const std::function<Ret(Params...)>& a_func);
};

#endif
