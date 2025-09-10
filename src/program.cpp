#include "../include/program.hpp"

template <typename Ret>
std::function<std::any(const std::list<std::any>&)>
make_general_function(std::function<Ret()> a_function)
{
    return [a_function](const std::list<std::any>& a_params)
               -> std::any { return a_function(); };
}

template <typename Ret, typename FirstParam,
          typename... RestParams>
std::function<std::any(const std::list<std::any>&)>
make_general_function(
    std::function<Ret(FirstParam, RestParams...)>
        a_function)
{
    return
        [a_function](
            const std::list<std::any>& a_params) -> std::any
    {
        // curry the function
        std::function<Ret(RestParams...)> l_function =
            [a_function,
             a_params](RestParams... a_rest_params) -> Ret
        {
            return a_function(
                std::any_cast<FirstParam>(a_params.front()),
                a_rest_params...);
        };

        // recurse
        return make_general_function(l_function)(
            std::list<std::any>(std::next(a_params.begin()),
                                a_params.end()));
    };
}

template <typename Ret, typename... Params>
const func_t* program_t::add_primitive(
    const std::string& a_repr,
    const std::function<Ret(Params...)>& a_func)
{
    // convert the function to a general function
    auto l_general_function = make_general_function(a_func);

    // return type
    std::type_index l_return_type = typeid(Ret);

    // get the parameter types
    std::list<std::type_index> l_param_types = {
        typeid(Params)...};

    func_body_t l_body{
        .m_functor = l_general_function,
        .m_children = {},
    };

    // iterator to start of params
    std::list<std::any>::iterator l_params =
        m_param_heap.end();

    // index used for parameter names
    int i = 0;

    // add the parameter nodes to the definition
    for(const auto& l_param_type : l_param_types)
        l_body.m_children.push_back(
            func_body_t{*add_parameter(i++, l_param_type)});

    // create the function
    func_t l_func{
        .m_param_types = l_param_types,
        .m_params = l_params,
        .m_body = l_body,
        .m_repr = a_repr,
    };

    // add the function to the env
    auto l_func_it = m_funcs.insert(m_funcs.end(), l_func);

    // get the pointer to the function
    const func_t* l_func_ptr = &*l_func_it;

    // return the function
    return l_func_ptr;
}

const func_t*
program_t::add_parameter(const int a_param_index,
                         const std::type_index& a_type)
{
    // create the value
    auto l_value_it =
        m_param_heap.insert(m_param_heap.end(), std::any{});

    // create the repr
    std::string l_repr =
        "p" + std::to_string(a_param_index);

    // create the definition
    func_body_t l_body{
        .m_functor =
            [l_value_it](const std::list<std::any>&)
        { return *l_value_it; },
        .m_children = {},
    };

    // create the func_t
    func_t l_func{
        .m_param_types = {},
        .m_params = m_param_heap.end(),
        .m_body = l_body,
        .m_repr = l_repr,
    };

    // add the function to the env
    auto l_func_it = m_funcs.insert(m_funcs.end(), l_func);

    // get the pointer to the function
    const func_t* l_func_ptr = &*l_func_it;

    // return the function
    return l_func_ptr;
}
