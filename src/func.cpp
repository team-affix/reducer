#include "../include/func.hpp"
#include <numeric>

std::any func_body::eval() const
{
    // construct the arguments for the functor
    std::list<std::any> l_functor_args;

    // evaluate all children
    std::transform(m_children.begin(), m_children.end(),
                   std::back_inserter(l_functor_args),
                   [](const func_body& a_child) { return a_child.eval(); });

    // evaluate the functor
    return m_functor(l_functor_args.begin(), l_functor_args.end());
}

size_t func_body::node_count() const
{
    // count nodes in children, add one for this node
    return std::accumulate(m_children.begin(), m_children.end(), 1,
                           [](size_t a_sum, const func_body& a_child)
                           { return a_sum + a_child.node_count(); });
}

std::any func::operator()(std::list<std::any>::const_iterator a_begin,
                          std::list<std::any>::const_iterator a_end)
{
    // set the parameters
    std::copy(a_begin, a_end, m_params.begin());
    // evaluate the function
    return m_body.eval();
}

func::func(const std::type_index& a_return_type, const func_body& a_body,
           const std::string& a_repr)
    : m_return_type(a_return_type), m_body(a_body), m_repr(a_repr)
{
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_func_construction()
{
    const std::type_index l_return_type = typeid(int);
    const func_body l_body{.m_functor = [](std::list<std::any>::const_iterator,
                                           std::list<std::any>::const_iterator)
                           { return std::any(10); },
                           .m_children = {}};
    const std::string l_repr = "func123";
    func l_func{l_return_type, l_body, l_repr};
    assert(l_func.m_return_type == l_return_type);
    assert(l_func.m_param_types.empty());
    assert(l_func.m_params.empty());
    assert(l_func.m_body.node_count() == 1);
    assert(std::any_cast<int>(l_func.m_body.eval()) == 10);
    assert(l_func.m_repr == l_repr);
}

void test_func_body_eval()
{
    // nullary
    {
        func_body l_node{.m_functor = [](std::list<std::any>::const_iterator,
                                         std::list<std::any>::const_iterator)
                         { return std::any(10); },
                         .m_children = {}};
        // evaluate the node
        auto l_result = l_node.eval();
        // assert that the type is int
        assert(l_result.type() == typeid(int));
        assert(std::any_cast<int>(l_result) == 10);
    }

    // unary
    {
        func_body l_node{
            .m_functor = [](std::list<std::any>::const_iterator a_begin,
                            std::list<std::any>::const_iterator a_end)
            { return 10 + std::any_cast<int>(*a_begin); },
            .m_children =
                {
                    func_body{
                        .m_functor = [](std::list<std::any>::const_iterator,
                                        std::list<std::any>::const_iterator)
                        { return std::any(11); },
                    },
                },
        };
        // evaluate the node
        auto l_result = l_node.eval();
        // assert that the type is int
        assert(l_result.type() == typeid(int));
        assert(std::any_cast<int>(l_result) == 21);
    }

    // binary
    {
        func_body l_node{
            .m_functor =
                [](std::list<std::any>::const_iterator a_begin,
                   std::list<std::any>::const_iterator a_end)
            {
                return 10 + std::any_cast<int>(*a_begin) +
                       std::any_cast<int>(*std::next(a_begin));
            },
            .m_children =
                {
                    func_body{
                        .m_functor = [](std::list<std::any>::const_iterator,
                                        std::list<std::any>::const_iterator)
                        { return std::any(11); },
                    },
                    func_body{
                        .m_functor = [](std::list<std::any>::const_iterator,
                                        std::list<std::any>::const_iterator)
                        { return std::any(12); },
                    },
                },
        };
        // evaluate the node
        auto l_result = l_node.eval();
        // assert that the type is int
        assert(l_result.type() == typeid(int));
        assert(std::any_cast<int>(l_result) == 33);
    }

    // doubly-nested unary
    {
        func_body l_node{
            .m_functor = [](std::list<std::any>::const_iterator a_begin,
                            std::list<std::any>::const_iterator a_end)
            { return std::any(10 + std::any_cast<int>(*a_begin)); },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [](std::list<std::any>::const_iterator a_begin,
                               std::list<std::any>::const_iterator a_end)
                        { return std::any(11 + std::any_cast<int>(*a_begin)); },
                        .m_children =
                            {
                                func_body{
                                    .m_functor =
                                        [](std::list<std::any>::const_iterator,
                                           std::list<std::any>::const_iterator)
                                    { return std::any(12); },
                                },
                            },
                    },
                },
        };
        // evaluate the node
        auto l_result = l_node.eval();
        // assert that the type is int
        assert(l_result.type() == typeid(int));
        assert(std::any_cast<int>(l_result) == 33);
    }
}

void test_func_body_node_count()
{
    // nullary
    {
        func_body l_node;
        // assert that the node count is 1
        assert(l_node.node_count() == 1);
    }

    // unary
    {
        func_body l_node{
            .m_children =
                {
                    func_body{},
                },
        };
        // assert that the node count is 1
        assert(l_node.node_count() == 2);
    }

    // binary
    {
        func_body l_node{
            .m_children = {func_body{}, func_body{}},
        };
        // assert that the node count is 3
        assert(l_node.node_count() == 3);
    }

    // doubly-nested unary
    {
        func_body l_node{
            .m_children =
                {
                    func_body{
                        .m_children = {func_body{}},
                    },
                },
        };
        // assert that the node count is 3
        assert(l_node.node_count() == 3);
    }

    // ternary
    {
        func_body l_node{
            .m_children = {func_body{}, func_body{}, func_body{}},
        };
        // assert that the node count is 4
        assert(l_node.node_count() == 4);
    }

    // doubly-nested binary
    {
        func_body l_node{
            .m_children =
                {
                    func_body{
                        .m_children = {func_body{}, func_body{}},
                    },
                    func_body{
                        .m_children = {func_body{}, func_body{}},
                    },
                },
        };
        // assert that the node count is 7
        assert(l_node.node_count() == 7);
    }
}

void test_func_operator_parens()
{
    // nullary, no params
    {
        // define the return type
        const std::type_index l_return_type = typeid(int);
        // define the body
        func_body l_body{
            .m_functor = [](std::list<std::any>::const_iterator,
                            std::list<std::any>::const_iterator)
            { return std::any(10); },
            .m_children = {},
        };
        // define the function
        func l_func{l_return_type, l_body, "f0"};
        // define the input
        std::list<std::any> l_input;
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input.begin(), l_input.end())) ==
               10);
    }

    // just a parameter
    {
        // define the function
        func l_func{typeid(int), func_body{}, "f0"};
        // define the param types
        l_func.m_param_types.push_back(typeid(int));
        // define the params
        l_func.m_params.push_back(std::any());
        // define the body
        l_func.m_body = {
            .m_functor = [&l_func](std::list<std::any>::const_iterator,
                                   std::list<std::any>::const_iterator)
            { return l_func.m_params.front(); },
            .m_children = {},
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(14));
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input.begin(), l_input.end())) ==
               14);
    }

    // unary on a parameter
    {
        // define the function
        func l_func{typeid(int), func_body{}, "f0"};
        // define the param types
        l_func.m_param_types.push_back(typeid(int));
        // define the params
        l_func.m_params.push_back(std::any());
        // define the body
        l_func.m_body = {
            .m_functor =
                [&l_func](std::list<std::any>::const_iterator,
                          std::list<std::any>::const_iterator)
            {
                return std::any(10 +
                                std::any_cast<int>(l_func.m_params.front()));
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [&l_func](std::list<std::any>::const_iterator,
                                      std::list<std::any>::const_iterator)
                        { return l_func.m_params.front(); },
                    },
                },
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(15));
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input.begin(), l_input.end())) ==
               25);
    }

    // binary of a parameter and a constant
    {
        // define the function
        func l_func{typeid(int), func_body{}, "f0"};
        // define the param types
        l_func.m_param_types.push_back(typeid(int));
        // define the params
        l_func.m_params.push_back(std::any());
        // define the body
        l_func.m_body = {
            .m_functor =
                [](std::list<std::any>::const_iterator a_begin,
                   std::list<std::any>::const_iterator a_end)
            {
                int l_arg_0 = std::any_cast<int>(*a_begin);
                int l_arg_1 = std::any_cast<int>(*std::next(a_begin));
                return 10 + l_arg_0 + l_arg_1;
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [&l_func](std::list<std::any>::const_iterator,
                                      std::list<std::any>::const_iterator)
                        { return l_func.m_params.front(); },
                    },
                    func_body{
                        .m_functor = [](std::list<std::any>::const_iterator,
                                        std::list<std::any>::const_iterator)
                        { return std::any(12); },
                    },
                },
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(16));
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input.begin(), l_input.end())) ==
               38);
    }

    // binary of two parameters
    {
        // define the function
        func l_func{typeid(double), func_body{}, "f0"};
        // define the param types
        l_func.m_param_types.push_back(typeid(int));
        l_func.m_param_types.push_back(typeid(double));
        // define the params
        l_func.m_params.push_back(std::any());
        l_func.m_params.push_back(std::any());
        // define the body
        l_func.m_body = {
            .m_functor =
                [](std::list<std::any>::const_iterator a_begin,
                   std::list<std::any>::const_iterator a_end)
            {
                return std::any_cast<int>(*a_begin) +
                       std::any_cast<double>(*std::next(a_begin));
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [&l_func](std::list<std::any>::const_iterator,
                                      std::list<std::any>::const_iterator)
                        { return l_func.m_params.front(); },
                    },
                    func_body{
                        .m_functor =
                            [&l_func](std::list<std::any>::const_iterator,
                                      std::list<std::any>::const_iterator)
                        { return *std::next(l_func.m_params.begin()); },
                    },
                },
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(17));
        l_input.push_back(std::any(18.5));
        // make sure we get the right result
        assert(std::any_cast<double>(l_func(l_input.begin(), l_input.end())) ==
               35.5);
    }

    // doubly-nested unary of a parameter
    {
        // define the function
        func l_func{typeid(int), func_body{}, "f0"};
        // define the param types
        l_func.m_param_types.push_back(typeid(int));
        // define the params
        l_func.m_params.push_back(std::any());
        // define the body
        l_func.m_body = {
            .m_functor = [](std::list<std::any>::const_iterator a_begin,
                            std::list<std::any>::const_iterator a_end)
            { return 10 + std::any_cast<int>(*a_begin); },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [](std::list<std::any>::const_iterator a_begin,
                               std::list<std::any>::const_iterator a_end)
                        { return 15 + std::any_cast<int>(*a_begin); },
                        .m_children =
                            {
                                func_body{
                                    .m_functor =
                                        [&l_func](
                                            std::list<std::any>::const_iterator,
                                            std::list<std::any>::const_iterator)
                                    { return l_func.m_params.front(); },
                                },
                            },
                    },
                },
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(7));
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input.begin(), l_input.end())) ==
               32);
    }
}

void func_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_func_construction);
    TEST(test_func_body_eval);
    TEST(test_func_body_node_count);
    TEST(test_func_operator_parens);
}

#endif
