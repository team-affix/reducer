#include "../include/program.hpp"

#include <cmath>
#include <sstream>

func* program::add_parameter(func* a_func, std::type_index a_type)
{
    // create the repr
    std::string l_repr = "p" + std::to_string(a_func->m_params->size());

    // add the parameter type
    a_func->m_param_types.push_back(a_type);

    // create the value
    auto l_param_it =
        a_func->m_params->insert(a_func->m_params->end(), std::any{});

    // create the definition
    func_body l_body{
        .m_functor = [l_param_it](std::list<std::any>::const_iterator,
                                  std::list<std::any>::const_iterator)
        { return *l_param_it; },
        .m_children = {},
    };

    // create the func_t in place using emplace
    func* l_result = new func(a_type, l_body, l_repr);

    // add the function to the program
    m_funcs.push_back(std::shared_ptr<func>(l_result));

    // just return the function
    return l_result;
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

        // create function
        func l_func{typeid(int), func_body{}, "myfunc"};

        // add the parameter
        const auto l_param_func =
            l_program.add_parameter(&l_func, typeid(double));

        // verify the func list contains param func
        assert(l_program.m_funcs.begin()->get() == l_param_func);

        // ensure that the func is correct
        assert(l_func.m_param_types.size() == 1);
        assert(l_func.m_param_types.front() == typeid(double));
        assert(l_func.m_params->size() == 1);
        assert(l_func.m_body.node_count() ==
               1); // THE NODE COUNT IS STILL 1 BECAUSE WE DID NOT ADD PARAM
                   // NODE TO THE BODY
        assert(l_func.m_repr == "myfunc");

        // make sure the param func is correct
        assert(l_param_func->m_param_types.empty());
        assert(l_param_func->m_params->empty());
        assert(l_param_func->m_body.node_count() == 1);
        assert(l_param_func->m_repr == "p0");

        // create the input
        std::list<std::any> l_input;

        // set the param
        l_func.m_params->front() = std::any(10.2);

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   l_param_func->eval(l_input.begin(), l_input.end())) == 10.2);
    }

    // two params
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        // create the function
        func l_func{typeid(std::string), func_body{}, "myfunc"};
        l_func.m_body.m_functor = std::function(
            [](std::list<std::any>::const_iterator a_begin,
               std::list<std::any>::const_iterator a_end)
            {
                std::ostringstream l_oss;
                l_oss << std::any_cast<double>(*a_begin);
                return l_oss.str() + std::string(" ") +
                       std::any_cast<std::string>(*std::next(a_begin));
            });

        // add the parameter
        auto l_param_0_func = l_program.add_parameter(&l_func, typeid(double));

        // add the parameter
        auto l_param_1_func =
            l_program.add_parameter(&l_func, typeid(std::string));

        // verify the func list contains param func
        assert(l_program.m_funcs.begin()->get() == l_param_0_func);
        assert(std::next(l_program.m_funcs.begin())->get() == l_param_1_func);

        // ensure that the func is correct
        assert(l_func.m_return_type == typeid(std::string));
        assert(l_func.m_param_types.size() == 2);
        assert(l_func.m_param_types.front() == typeid(double));
        assert(l_func.m_param_types.back() == typeid(std::string));
        assert(l_func.m_params->size() == 2);
        assert(l_func.m_body.node_count() ==
               1); // we have not yet added the parameter nodes to the body
        assert(l_func.m_repr == "myfunc");

        // add the parameter nodes to the body
        l_func.m_body.m_children.push_back(func_body{
            [l_param_0_func](std::list<std::any>::const_iterator a_begin,
                             std::list<std::any>::const_iterator a_end)
            { return l_param_0_func->eval(a_begin, a_end); }});
        l_func.m_body.m_children.push_back(func_body{
            [l_param_1_func](std::list<std::any>::const_iterator a_begin,
                             std::list<std::any>::const_iterator a_end)
            { return l_param_1_func->eval(a_begin, a_end); }});

        // create the input
        std::list<std::any> l_input;

        // set the param
        l_func.m_params->front() = std::any(10.2);
        l_func.m_params->back() = std::any(std::string("hello"));

        // make sure the function evaluates correctly
        assert(std::any_cast<std::string>(l_func.eval(
                   l_input.begin(), l_input.end())) == "10.2 hello");
    }
}

void test_program_add_primitive()
{
    // add a nullary int primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        auto l_func =
            l_program.add_primitive("f0", std::function([]() { return 10; }));

        // verify the func list is not empty
        assert(l_program.m_funcs.begin()->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(int));
        assert(l_func->m_param_types.empty());
        assert(l_func->m_params->empty());
        assert(l_func->m_body.node_count() == 1);

        // create the input
        std::list<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<int>(
                   l_func->eval(l_input.begin(), l_input.end())) == 10);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a nullary double primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        auto l_func =
            l_program.add_primitive("f0", std::function([]() { return 10.4; }));

        // verify the func list is not empty
        assert(l_program.m_funcs.begin()->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(double));
        assert(l_func->m_param_types.empty());
        assert(l_func->m_params->empty());
        assert(l_func->m_body.node_count() == 1);

        // create the input
        std::list<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   l_func->eval(l_input.begin(), l_input.end())) == 10.4);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a nullary string primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        auto l_func = l_program.add_primitive(
            "f0", std::function([]() { return std::string("hello"); }));

        // verify the func list is not empty
        assert(l_program.m_funcs.begin()->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(std::string));
        assert(l_func->m_param_types.empty());
        assert(l_func->m_params->empty());
        assert(l_func->m_body.node_count() == 1);

        // create the input
        std::list<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<std::string>(
                   l_func->eval(l_input.begin(), l_input.end())) == "hello");

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a unary int primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        auto l_func = l_program.add_primitive(
            "f0", std::function([](double a_x) { return int(a_x + 10); }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin(), 1)->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(int));
        assert(l_func->m_param_types.size() == 1);
        assert(l_func->m_param_types.front() == typeid(double));
        assert(l_func->m_params->size() == 1);
        assert(l_func->m_body.node_count() == 2);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10.2));

        // make sure the function evaluates correctly
        assert(std::any_cast<int>(
                   l_func->eval(l_input.begin(), l_input.end())) == 20);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a unary double primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        auto l_func = l_program.add_primitive(
            "f0", std::function([](double a_x) { return a_x + 10.3; }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin(), 1)->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(double));
        assert(l_func->m_param_types.size() == 1);
        assert(l_func->m_param_types.front() == typeid(double));
        assert(l_func->m_params->size() == 1);
        assert(l_func->m_body.node_count() == 2);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10.2));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   l_func->eval(l_input.begin(), l_input.end())) == 20.5);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a unary string primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        auto l_func = l_program.add_primitive(
            "f0",
            std::function([](std::string a_x) { return a_x + " world"; }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin(), 1)->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(std::string));
        assert(l_func->m_param_types.size() == 1);
        assert(l_func->m_param_types.front() == typeid(std::string));
        assert(l_func->m_params->size() == 1);
        assert(l_func->m_body.node_count() == 2);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(std::string("hello")));

        // make sure the function evaluates correctly
        assert(std::any_cast<std::string>(l_func->eval(
                   l_input.begin(), l_input.end())) == "hello world");

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a binary int primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        auto l_func = l_program.add_primitive(
            "f0",
            std::function([](int a_x, int a_y) { return double(a_x + a_y); }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin(), 2)->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(double));
        assert(l_func->m_param_types.size() == 2);
        assert(std::equal(l_func->m_param_types.begin(),
                          l_func->m_param_types.end(),
                          std::list({std::type_index(typeid(int)),
                                     std::type_index(typeid(int))})
                              .begin()));
        assert(l_func->m_params->size() == 2);
        assert(l_func->m_body.node_count() == 3);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10));
        l_input.push_back(std::any(20));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   l_func->eval(l_input.begin(), l_input.end())) == 30.0);

        // make sure the function has the correct repr
        assert(l_func->m_repr == "f0");
    }

    // add a binary double primitive
    {
        program l_program;

        // verify the func list is empty
        assert(l_program.m_funcs.empty());

        auto l_func = l_program.add_primitive(
            "pow", std::function([](double a_x, double a_y)
                                 { return pow(a_x, a_y); }));

        // verify the func list is not empty
        assert(std::next(l_program.m_funcs.begin(), 2)->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(double));
        assert(l_func->m_param_types.size() == 2);
        assert(std::equal(l_func->m_param_types.begin(),
                          l_func->m_param_types.end(),
                          std::list({std::type_index(typeid(double)),
                                     std::type_index(typeid(double))})
                              .begin()));
        assert(l_func->m_params->size() == 2);
        assert(l_func->m_body.node_count() == 3);

        // create the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(10.2));
        l_input.push_back(std::any(2.5));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(l_func->eval(
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
