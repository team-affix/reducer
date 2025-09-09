#ifndef ENV_HPP
#define ENV_HPP

#include "func.hpp"
#include <any>
#include <list>

struct env_t
{
    std::list<func_t> m_funcs;
    std::list<std::any> m_values;
};

#endif
