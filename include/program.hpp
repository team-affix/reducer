#ifndef ENV_HPP
#define ENV_HPP

#include "../include/func.hpp"
#include <any>
#include <list>

struct program_t
{
    std::list<func> m_funcs;
    std::list<std::any> m_param_heap;

    // adding functions
    template <typename Ret, typename... Params>
    const func* add_primitive(
        const std::string& a_repr,
        const std::function<Ret(Params...)>& a_func);
    const func*
    add_parameter(const int a_param_index,
                  const std::type_index& a_type);
};

#endif
