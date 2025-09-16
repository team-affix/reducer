#ifndef ENV_HPP
#define ENV_HPP

#include "../include/func.hpp"
#include <any>
#include <list>
#include <memory>

template <typename Ret>
std::function<std::any(const std::any*, size_t)>
make_general_function(std::function<Ret()> a_function)
{
    return [a_function](const std::any*, size_t) -> std::any
    { return a_function(); };
}

template <typename Ret, typename FirstParam, typename... RestParams>
std::function<std::any(const std::any*, size_t)>
make_general_function(std::function<Ret(FirstParam, RestParams...)> a_function)
{
    return
        [a_function](const std::any* a_params, size_t a_param_count) -> std::any
    {
        // curry the function
        std::function<Ret(RestParams...)> l_function =
            [a_function, a_params](RestParams... a_rest_params) -> Ret
        {
            return a_function(std::any_cast<FirstParam>(*a_params),
                              a_rest_params...);
        };

        // recurse
        return make_general_function(l_function)(a_params + 1,
                                                 a_param_count - 1);
    };
}

struct program
{
    std::list<std::shared_ptr<func>> m_funcs;

    // adding functions
    template <typename Ret, typename... Params>
    func* add_primitive(const std::string& a_repr,
                        const std::function<Ret(Params...)>& a_func)
    {
        // return type
        std::type_index l_return_type = typeid(Ret);

        // convert the function to a general function
        auto l_general_functor = make_general_function(a_func);

        // get the parameter types
        std::vector<std::type_index> l_param_types_list = {typeid(Params)...};

        // convert the parameter types to a multimap
        std::multimap<std::type_index, size_t> l_param_types;
        for(size_t i = 0; i < l_param_types_list.size(); ++i)
            l_param_types.emplace(l_param_types_list[i], i);

        // create the function
        auto l_func = std::make_shared<func>(
            l_return_type, l_param_types,
            func::body{.m_functor = func::primitive{l_general_functor}},
            a_repr);

        // add the function to the program
        m_funcs.push_back(l_func);

        // add the parameter nodes to the definition
        for(int i = 0; i < l_param_types.size(); ++i)
        {
            // add the parameter to the body
            l_func->m_body.m_children.push_back(func::body{
                .m_functor = func::param{(size_t)i},
            });
        }

        // just return the function
        return l_func.get();
    }
};

#endif
