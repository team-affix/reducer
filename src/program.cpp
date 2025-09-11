#include "../include/program.hpp"

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

template <typename Ret, typename... Params>
const func*
program_t::add_primitive(const std::string& a_repr,
                         const std::function<Ret(Params...)>& a_func)
{
    // convert the function to a general function
    auto l_general_function = make_general_function(a_func);

    // return type
    std::type_index l_return_type = typeid(Ret);

    // get the parameter types
    std::list<std::type_index> l_param_types = {typeid(Params)...};

    func_body l_body{
        .m_functor = l_general_function,
        .m_children = {},
    };

    // iterator to start of params
    std::list<std::any>::iterator l_params = m_param_heap.end();

    // index used for parameter names
    int i = 0;

    // add the parameter nodes to the definition
    for(const auto& l_param_type : l_param_types)
        l_body.m_children.push_back(
            func_body{*add_parameter(i++, l_param_type)});

    // create the function
    func l_func{
        .m_param_types = l_param_types,
        .m_params = l_params,
        .m_body = l_body,
        .m_repr = a_repr,
    };

    // add the function to the env
    auto l_func_it = m_funcs.insert(m_funcs.end(), l_func);

    // get the pointer to the function
    const func* l_func_ptr = &*l_func_it;

    // return the function
    return l_func_ptr;
}

const func* program_t::add_parameter(const int a_param_index,
                                     const std::type_index& a_type)
{
    // create the value
    auto l_value_it = m_param_heap.insert(m_param_heap.end(), std::any{});

    // create the repr
    std::string l_repr = "p" + std::to_string(a_param_index);

    // create the definition
    func_body l_body{
        .m_functor = [l_value_it](std::list<std::any>::const_iterator,
                                  std::list<std::any>::const_iterator)
        { return *l_value_it; },
        .m_children = {},
    };

    // create the func_t
    func l_func{
        .m_param_types = {},
        .m_params = m_param_heap.end(),
        .m_body = l_body,
        .m_repr = l_repr,
    };

    // add the function to the env
    auto l_func_it = m_funcs.insert(m_funcs.end(), l_func);

    // get the pointer to the function
    const func* l_func_ptr = &*l_func_it;

    // return the function
    return l_func_ptr;
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_make_general_function()
{
    // nullary function
    {
        std::function<int()> l_function = []() { return 10; };
        auto l_general_function = make_general_function(l_function);

        // define the input
        std::list<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<int>(
                   l_general_function(l_input.begin(), l_input.end())) == 10);
    }

    // unary function
    {
        std::function l_function = [](double a_x) { return a_x + 13.5; };
        auto l_general_function = make_general_function(l_function);

        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10.0));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   l_general_function(l_input.begin(), l_input.end())) == 23.5);
    }

    // binary function
    {
        std::function l_function = [](double a_x, double a_y)
        { return a_x * a_y; };
        auto l_general_function = make_general_function(l_function);

        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10.0));
        l_input.push_back(std::any(20.5));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(l_general_function(
                   l_input.begin(), l_input.end())) == 205.0);
    }

    // ternary function
    {
        std::function l_function = [](double a_x, double a_y, double a_z)
        { return a_x * a_y * a_z; };
        auto l_general_function = make_general_function(l_function);

        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10.0));
        l_input.push_back(std::any(20.5));
        l_input.push_back(std::any(30.0));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(l_general_function(
                   l_input.begin(), l_input.end())) == 6150.0);
    }
}

void program_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_make_general_function);
}

#endif
