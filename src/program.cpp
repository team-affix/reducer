#include "../include/program.hpp"

#include <cmath>
#include <sstream>

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_make_general_function()
{
    // nullary function
    {
        std::function<int()> l_function = []() { return 10; };
        auto l_general_function = make_general_function(l_function);

        // define the input
        std::vector<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<int>(
                   l_general_function(l_input.data(), l_input.size())) == 10);
    }

    // unary function
    {
        std::function l_function = [](double a_x) { return a_x + 13.5; };
        auto l_general_function = make_general_function(l_function);

        // define the input
        std::vector<std::any> l_input;
        l_input.push_back(std::any(10.0));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(
                   l_general_function(l_input.data(), l_input.size())) == 23.5);
    }

    // binary function
    {
        std::function l_function = [](double a_x, double a_y)
        { return a_x * a_y; };
        auto l_general_function = make_general_function(l_function);

        // define the input
        std::vector<std::any> l_input;
        l_input.push_back(std::any(10.0));
        l_input.push_back(std::any(20.5));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(l_general_function(
                   l_input.data(), l_input.size())) == 205.0);
    }

    // ternary function
    {
        std::function l_function = [](double a_x, double a_y, double a_z)
        { return a_x * a_y * a_z; };
        auto l_general_function = make_general_function(l_function);

        // define the input
        std::vector<std::any> l_input;
        l_input.push_back(std::any(10.0));
        l_input.push_back(std::any(20.5));
        l_input.push_back(std::any(30.0));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(l_general_function(
                   l_input.data(), l_input.size())) == 6150.0);
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
        assert(l_func->m_body.node_count() == 1);

        // create the input
        std::vector<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<int>(
                   l_func->m_body.eval(l_input.data(), l_input.size())) == 10);

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
        assert(l_func->m_body.node_count() == 1);

        // create the input
        std::vector<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(l_func->m_body.eval(
                   l_input.data(), l_input.size())) == 10.4);

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
        assert(l_func->m_body.node_count() == 1);

        // create the input
        std::vector<std::any> l_input;

        // make sure the function evaluates correctly
        assert(std::any_cast<std::string>(l_func->m_body.eval(
                   l_input.data(), l_input.size())) == "hello");

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
        assert(l_program.m_funcs.begin()->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(int));
        assert(l_func->m_param_types ==
               (std::multimap<std::type_index, size_t>({{typeid(double), 0}})));
        assert(l_func->m_body.node_count() == 2);

        // create the input
        std::vector<std::any> l_input;
        l_input.push_back(std::any(10.2));

        // make sure the function evaluates correctly
        assert(std::any_cast<int>(
                   l_func->m_body.eval(l_input.data(), l_input.size())) == 20);

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
        assert(l_program.m_funcs.begin()->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(double));
        assert(l_func->m_param_types ==
               (std::multimap<std::type_index, size_t>({{typeid(double), 0}})));
        assert(l_func->m_body.node_count() == 2);
        assert(l_func->m_repr == "f0");

        // create the input
        std::vector<std::any> l_input;
        l_input.push_back(std::any(10.2));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(l_func->m_body.eval(
                   l_input.data(), l_input.size())) == 20.5);
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
        assert(l_program.m_funcs.begin()->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(std::string));
        assert(l_func->m_param_types == (std::multimap<std::type_index, size_t>(
                                            {{typeid(std::string), 0}})));
        assert(l_func->m_body.node_count() == 2);
        assert(l_func->m_repr == "f0");

        // create the input
        std::vector<std::any> l_input;
        l_input.push_back(std::any(std::string("hello")));

        // make sure the function evaluates correctly
        assert(std::any_cast<std::string>(l_func->m_body.eval(
                   l_input.data(), l_input.size())) == "hello world");
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
        assert(l_program.m_funcs.begin()->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(double));
        assert(l_func->m_param_types ==
               (std::multimap<std::type_index, size_t>(
                   {{typeid(int), 0}, {typeid(int), 1}})));
        assert(l_func->m_body.node_count() == 3);

        // create the input
        std::vector<std::any> l_input;
        l_input.push_back(std::any(10));
        l_input.push_back(std::any(20));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(l_func->m_body.eval(
                   l_input.data(), l_input.size())) == 30.0);

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
        assert(l_program.m_funcs.begin()->get() == l_func);

        // verify the function has the correct param types
        assert(l_func->m_return_type == typeid(double));
        assert(l_func->m_param_types ==
               (std::multimap<std::type_index, size_t>(
                   {{typeid(double), 0}, {typeid(double), 1}})));
        assert(l_func->m_body.node_count() == 3);
        assert(l_func->m_repr == "pow");

        // create the input
        std::vector<std::any> l_input;
        l_input.push_back(std::any(10.2));
        l_input.push_back(std::any(2.5));

        // make sure the function evaluates correctly
        assert(std::any_cast<double>(l_func->m_body.eval(
                   l_input.data(), l_input.size())) == pow(10.2, 2.5));
    }
}

void program_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_make_general_function);
    TEST(test_program_add_primitive);
}

#endif
