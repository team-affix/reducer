#include "../include/program.hpp"

#include <cmath>

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

std::list<func>::const_iterator
program::add_parameter(std::list<std::any>::iterator& a_param_it,
                       const int a_param_index)
{
    // create the value
    a_param_it = m_param_heap.insert(m_param_heap.end(), std::any{});

    // create the repr
    std::string l_repr = "p" + std::to_string(a_param_index);

    // create the definition
    func_body l_body{
        .m_functor = [a_param_it](std::list<std::any>::const_iterator,
                                  std::list<std::any>::const_iterator)
        { return *a_param_it; },
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
    return m_funcs.insert(m_funcs.end(), l_func);
}

template <typename Ret, typename... Params>
std::list<func>::const_iterator
program::add_primitive(const std::string& a_repr,
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

    // iterator to end of param heap
    auto l_params = m_param_heap.end();

    // index used for parameter names
    int i = 0;

    // add the parameter nodes to the definition
    for(const auto& l_param_type : l_param_types)
    {
        // temporary iterator to the parameter
        std::list<std::any>::iterator l_param_it;

        // add the parameter to the heap
        l_body.m_children.push_back(func_body{*add_parameter(l_param_it, i++)});

        // save the iterator to the first inserted value
        if(l_params == m_param_heap.end())
            l_params = l_param_it;
    }

    // create the function
    func l_func{
        .m_param_types = l_param_types,
        .m_params = l_params,
        .m_body = l_body,
        .m_repr = a_repr,
    };

    // add the function to the env
    return m_funcs.insert(m_funcs.end(), l_func);
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

void test_program_add_parameter()
{
    // single param
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        // verify the param heap is empty
        assert(l_program.m_param_heap.empty());

        // add the parameter
        std::list<std::any>::iterator l_param_it;
        const auto l_func = l_program.add_parameter(l_param_it, 0);

        // verify the func list contains func
        assert(l_program.m_funcs.begin() == l_func);

        // verify the param heap contains val
        assert(l_program.m_param_heap.begin() == l_param_it);

        // ensure that the func is correct
        assert(l_func->m_param_types.empty());
        assert(l_func->m_params == l_program.m_param_heap.end());
        assert(l_func->m_body.node_count() == 1);
        assert(l_func->m_repr == "p0");

        // create the input
        std::list<std::any> l_input;

        // set the param
        *l_param_it = std::any(10.2);

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   (*l_func)(l_input.begin(), l_input.end())) == 10.2);
    }

    // two params
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        // verify the param heap is empty
        assert(l_program.m_param_heap.empty());

        // add the parameter
        std::list<std::any>::iterator l_param_0_it;
        const auto l_func_0 = l_program.add_parameter(l_param_0_it, 0);

        // verify the func list contains func
        assert(l_program.m_funcs.begin() == l_func_0);

        // verify the param heap contains val
        assert(l_program.m_param_heap.begin() == l_param_0_it);

        // ensure that the func is correct
        assert(l_func_0->m_param_types.empty());
        assert(l_func_0->m_params == l_program.m_param_heap.end());
        assert(l_func_0->m_body.node_count() == 1);
        assert(l_func_0->m_repr == "p0");

        // create the input
        std::list<std::any> l_input;

        // set the param
        *l_param_0_it = std::any(10.2);

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   (*l_func_0)(l_input.begin(), l_input.end())) == 10.2);

        // add the second parameter
        std::list<std::any>::iterator l_param_1_it;
        const auto l_func_1 = l_program.add_parameter(l_param_1_it, 1);

        // verify the func list contains func
        assert(std::next(l_program.m_funcs.begin()) == l_func_1);

        // verify the param heap contains val
        assert(std::next(l_program.m_param_heap.begin()) == l_param_1_it);

        // ensure that the func is correct
        assert(l_func_1->m_param_types.empty());
        assert(l_func_1->m_params == l_program.m_param_heap.end());
        assert(l_func_1->m_body.node_count() == 1);
        assert(l_func_1->m_repr == "p1");

        // set the param
        *l_param_1_it = std::any(20.5);

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   (*l_func_1)(l_input.begin(), l_input.end())) == 20.5);

        // make sure the first function STILL evaluates correctly
        assert(std::any_cast<double>(
                   (*l_func_0)(l_input.begin(), l_input.end())) == 10.2);
    }
}

void test_program_add_primitive()
{
    // add a nullary int primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        const auto l_func =
            l_program.add_primitive("f0", std::function([]() { return 10; }));

        // verify the func list is not empty
        assert(l_program.m_funcs.begin() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_param_types.empty());
        assert(l_func->m_params == l_program.m_param_heap.begin());
        assert(l_func->m_body.node_count() == 1);

        // create the input
        std::list<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<int>((*l_func)(l_input.begin(), l_input.end())) ==
               10);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a nullary double primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        const auto l_func =
            l_program.add_primitive("f0", std::function([]() { return 10.4; }));

        // verify the func list is not empty
        assert(l_program.m_funcs.begin() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_param_types.empty());
        assert(l_func->m_params == l_program.m_param_heap.begin());
        assert(l_func->m_body.node_count() == 1);

        // create the input
        std::list<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   (*l_func)(l_input.begin(), l_input.end())) == 10.4);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a nullary string primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        const auto l_func = l_program.add_primitive(
            "f0", std::function([]() { return std::string("hello"); }));

        // verify the func list is not empty
        assert(l_program.m_funcs.begin() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_param_types.empty());
        assert(l_func->m_params == l_program.m_param_heap.begin());
        assert(l_func->m_body.node_count() == 1);

        // create the input
        std::list<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<std::string>(
                   (*l_func)(l_input.begin(), l_input.end())) == "hello");

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a unary int primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        const auto l_func = l_program.add_primitive(
            "f0", std::function([](int a_x) { return a_x + 10; }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin()) == l_func);

        // verify the function has the correct param types
        assert(l_func->m_param_types.size() == 1);
        assert(l_func->m_param_types.front() == typeid(int));
        assert(l_func->m_params == l_program.m_param_heap.begin());
        assert(l_func->m_body.node_count() == 2);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10));

        // make sure the function evaluates correctly
        assert(std::any_cast<int>((*l_func)(l_input.begin(), l_input.end())) ==
               20);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a unary double primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        const auto l_func = l_program.add_primitive(
            "f0", std::function([](double a_x) { return a_x + 10.3; }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin()) == l_func);

        // verify the function has the correct param types
        assert(l_func->m_param_types.size() == 1);
        assert(l_func->m_param_types.front() == typeid(double));
        assert(l_func->m_params == l_program.m_param_heap.begin());
        assert(l_func->m_body.node_count() == 2);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10.2));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   (*l_func)(l_input.begin(), l_input.end())) == 20.5);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a unary string primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        const auto l_func = l_program.add_primitive(
            "f0",
            std::function([](std::string a_x) { return a_x + " world"; }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin()) == l_func);

        // verify the function has the correct param types
        assert(l_func->m_param_types.size() == 1);
        assert(l_func->m_param_types.front() == typeid(std::string));
        assert(l_func->m_params == l_program.m_param_heap.begin());
        assert(l_func->m_body.node_count() == 2);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(std::string("hello")));

        // make sure the function evaluates correctly
        assert(std::any_cast<std::string>(
                   (*l_func)(l_input.begin(), l_input.end())) == "hello world");

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a binary int primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        const auto l_func = l_program.add_primitive(
            "f0", std::function([](int a_x, int a_y) { return a_x + a_y; }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin(), 2) == l_func);

        // verify the function has the correct param types
        assert(l_func->m_param_types.size() == 2);
        assert(std::equal(l_func->m_param_types.begin(),
                          l_func->m_param_types.end(),
                          std::list({std::type_index(typeid(int)),
                                     std::type_index(typeid(int))})
                              .begin()));
        assert(l_func->m_params == l_program.m_param_heap.begin());
        assert(l_func->m_body.node_count() == 3);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10));
        l_input.push_back(std::any(20));

        // make sure the function evaluates correctly
        assert(std::any_cast<int>((*l_func)(l_input.begin(), l_input.end())) ==
               30);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a binary double primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        const auto l_func = l_program.add_primitive(
            "pow", std::function([](double a_x, double a_y)
                                 { return pow(a_x, a_y); }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin(), 2) == l_func);

        // verify the function has the correct param types
        assert(l_func->m_param_types.size() == 2);
        assert(std::equal(l_func->m_param_types.begin(),
                          l_func->m_param_types.end(),
                          std::list({std::type_index(typeid(double)),
                                     std::type_index(typeid(double))})
                              .begin()));
        assert(l_func->m_params == l_program.m_param_heap.begin());
        assert(l_func->m_body.node_count() == 3);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10.2));
        l_input.push_back(std::any(2.5));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>((*l_func)(
                   l_input.begin(), l_input.end())) == pow(10.2, 2.5));

        // make sure the function has the correct repr
        assert(l_func->m_repr == "pow");
    }
}

void program_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_make_general_function);
    TEST(test_program_add_parameter);
    TEST(test_program_add_primitive);
}

#endif
