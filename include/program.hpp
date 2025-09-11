#ifndef ENV_HPP
#define ENV_HPP

#include "../include/func.hpp"
#include <any>
#include <list>

template <typename Ret>
std::function<std::any(std::list<std::any>::const_iterator,
                       std::list<std::any>::const_iterator)>
make_general_function(std::function<Ret()> a_function)
{
    return [a_function](std::list<std::any>::const_iterator,
                        std::list<std::any>::const_iterator) -> std::any
    { return a_function(); };
}

template <typename Ret, typename FirstParam, typename... RestParams>
std::function<std::any(std::list<std::any>::const_iterator,
                       std::list<std::any>::const_iterator)>
make_general_function(std::function<Ret(FirstParam, RestParams...)> a_function)
{
    return [a_function](std::list<std::any>::const_iterator a_begin,
                        std::list<std::any>::const_iterator a_end) -> std::any
    {
        // curry the function
        std::function<Ret(RestParams...)> l_function =
            [a_function, a_begin](RestParams... a_rest_params) -> Ret
        {
            return a_function(std::any_cast<FirstParam>(*a_begin),
                              a_rest_params...);
        };

        // recurse
        return make_general_function(l_function)(std::next(a_begin), a_end);
    };
}

struct program
{
    std::list<func> m_funcs;

    // adding functions
    std::list<func>::iterator add_parameter(std::list<func>::iterator a_func_it,
                                            std::type_index a_type);
    template <typename Ret, typename... Params>
    std::list<func>::iterator
    add_primitive(const std::string& a_repr,
                  const std::function<Ret(Params...)>& a_func)
    {
        // return type
        std::type_index l_return_type = typeid(Ret);

        // convert the function to a general function
        auto l_general_functor = make_general_function(a_func);

        // insert the function in-place
        auto l_func_it =
            m_funcs.emplace(m_funcs.end(), l_return_type,
                            func_body{.m_functor = l_general_functor}, a_repr);

        // get the parameter types
        std::list<std::type_index> l_param_types = {typeid(Params)...};

        // add the parameter nodes to the definition
        for(const auto& l_param_type : l_param_types)
        {
            // create the parameter
            auto l_param_func_it = add_parameter(l_func_it, l_param_type);

            // add the parameter to the body
            l_func_it->m_body.m_children.push_back(func_body{
                [l_param_func_it](std::list<std::any>::const_iterator a_begin,
                                  std::list<std::any>::const_iterator a_end)
                { return l_param_func_it->eval(a_begin, a_end); }});
        }

        // just return the iterator
        return l_func_it;
    }
};

#endif
