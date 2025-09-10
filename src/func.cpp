#include "../include/func.hpp"
#include <numeric>

std::any func_body::eval() const
{
    // construct the arguments for the functor
    std::list<std::any> l_functor_args;

    // evaluate all children
    std::transform(m_children.begin(), m_children.end(),
                   std::back_inserter(l_functor_args),
                   [](const func_body& a_child)
                   { return a_child.eval(); });

    // evaluate the functor
    return m_functor(l_functor_args);
}

size_t func_body::node_count() const
{
    // count nodes in children, add one for this node
    return std::accumulate(
        m_children.begin(), m_children.end(), 1,
        [](size_t a_sum, const func_body& a_child)
        { return a_sum + a_child.node_count(); });
}

std::any
func::operator()(const std::list<std::any>& a_args) const
{
    // set the parameters
    std::copy(a_args.begin(), a_args.end(), m_params);
    // evaluate the function
    return m_body.eval();
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_func_construction()
{
    func l_func;
}

void test_func_body_eval()
{
    // nullary
    {
        func_body l_node{
            .m_functor =
                [](const std::list<std::any>& a_args)
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
            .m_functor =
                [](const std::list<std::any>& a_args)
            {
                return 10 +
                       std::any_cast<int>(a_args.front());
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [](const std::list<std::any>&
                                   a_args)
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
                [](const std::list<std::any>& a_args)
            {
                return std::any(
                    10 +
                    std::any_cast<int>(a_args.front()) +
                    std::any_cast<int>(
                        *std::next(a_args.begin())));
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [](const std::list<std::any>&
                                   a_args)
                        { return std::any(11); },
                    },
                    func_body{
                        .m_functor =
                            [](const std::list<std::any>&
                                   a_args)
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
            .m_functor =
                [](const std::list<std::any>& a_args)
            {
                return std::any(10 + std::any_cast<int>(
                                         a_args.front()));
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [](const std::list<std::any>&
                                   a_args)
                        {
                            return std::any(
                                11 + std::any_cast<int>(
                                         a_args.front()));
                        },
                        .m_children =
                            {
                                func_body{
                                    .m_functor =
                                        [](const std::list<
                                            std::any>&
                                               a_args)
                                    {
                                        return std::any(12);
                                    },
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
            .m_children = {func_body{}, func_body{},
                           func_body{}},
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
                        .m_children = {func_body{},
                                       func_body{}},
                    },
                    func_body{
                        .m_children = {func_body{},
                                       func_body{}},
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
        // define the global param vector
        std::list<std::any> l_params;
        // define the param types
        std::list<std::type_index> l_param_types;
        // define the body
        func_body l_body{
            .m_functor = [](const std::list<std::any>&)
            { return std::any(10); },
            .m_children = {},
        };
        // define the function
        func l_func{
            .m_param_types = l_param_types,
            .m_params = l_params.begin(),
            .m_body = l_body,
        };
        // define the input
        std::list<std::any> l_input;
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input)) == 10);
    }

    // just a parameter
    {
        // define the global param vector
        std::list<std::any> l_params;
        // define the param types
        std::list<std::type_index> l_param_types;
        // add a parameter
        l_param_types.push_back(typeid(int));
        l_params.push_back(int());
        // define the body
        func_body l_body{
            .m_functor =
                [&l_params](const std::list<std::any>&)
            { return l_params.front(); },
            .m_children = {},
        };
        // define the function
        func l_func{
            .m_param_types = l_param_types,
            .m_params = l_params.begin(),
            .m_body = l_body,
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(11));
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input)) == 11);
    }

    // unary on a parameter
    {
        // define the global param vector
        std::list<std::any> l_params;
        // define the param types
        std::list<std::type_index> l_param_types;
        // add a parameter
        l_param_types.push_back(typeid(int));
        l_params.push_back(int());
        // define the body
        func_body l_body{
            .m_functor =
                [](const std::list<std::any>& a_args)
            {
                return std::any(10 + std::any_cast<int>(
                                         a_args.front()));
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [&l_params](
                                const std::list<std::any>&)
                        { return l_params.front(); },
                    },
                },
        };
        // define the function
        func l_func{
            .m_param_types = l_param_types,
            .m_params = l_params.begin(),
            .m_body = l_body,
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(11));
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input)) == 21);
    }

    // binary of a parameter and a constant
    {
        // define the global param vector
        std::list<std::any> l_params;
        // define the param types
        std::list<std::type_index> l_param_types;
        // add a parameter
        l_param_types.push_back(typeid(int));
        l_params.push_back(int());
        // define the body
        func_body l_body{
            .m_functor =
                [](const std::list<std::any>& a_args)
            {
                int l_arg_0 =
                    std::any_cast<int>(*a_args.begin());
                int l_arg_1 = std::any_cast<int>(
                    *std::next(a_args.begin()));
                return 10 + l_arg_0 + l_arg_1;
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [&l_params](
                                const std::list<std::any>&)
                        { return l_params.front(); },
                    },
                    func_body{
                        .m_functor =
                            [](const std::list<std::any>&)
                        { return std::any(12); },
                    },
                },
        };
        // define the function
        func l_func{
            .m_param_types = l_param_types,
            .m_params = l_params.begin(),
            .m_body = l_body,
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(13));
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input)) == 35);
    }

    // binary of two parameters
    {
        // define the global param vector
        std::list<std::any> l_params;
        // define the param types
        std::list<std::type_index> l_param_types;
        // add a parameter
        l_param_types.push_back(typeid(int));
        l_params.push_back(int());
        // add a parameter
        l_param_types.push_back(typeid(int));
        l_params.push_back(int());
        // define the body
        func_body l_body{
            .m_functor =
                [](const std::list<std::any>& a_args)
            {
                std::vector<int> l_args;
                std::transform(
                    a_args.begin(), a_args.end(),
                    std::back_inserter(l_args),
                    [](const std::any& a_arg)
                    { return std::any_cast<int>(a_arg); });
                return l_args[0] + l_args[1];
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [&l_params](
                                const std::list<std::any>&)
                        { return *l_params.begin(); },
                    },
                    func_body{
                        .m_functor =
                            [&l_params](
                                const std::list<std::any>&)
                        {
                            return *std::next(
                                l_params.begin());
                        },
                    },
                },
        };
        // define the function
        func l_func{
            .m_param_types = l_param_types,
            .m_params = l_params.begin(),
            .m_body = l_body,
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(13));
        l_input.push_back(std::any(14));
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input)) == 27);
    }

    // doubly-nested unary of a parameter
    {
        // define the global param vector
        std::list<std::any> l_params;
        // define the param types
        std::list<std::type_index> l_param_types;
        // add a parameter
        l_param_types.push_back(typeid(int));
        l_params.push_back(int());
        // define the body
        func_body l_body{
            .m_functor =
                [](const std::list<std::any>& a_args)
            {
                return 10 +
                       std::any_cast<int>(a_args.front());
            },
            .m_children =
                {
                    func_body{
                        .m_functor =
                            [](const std::list<std::any>&
                                   a_args)
                        {
                            return 15 + std::any_cast<int>(
                                            a_args.front());
                        },
                        .m_children =
                            {
                                func_body{
                                    .m_functor =
                                        [&l_params](
                                            const std::list<
                                                std::any>&)
                                    {
                                        return *l_params
                                                    .begin();
                                    },
                                },
                            },
                    },
                },
        };
        // define the function
        func l_func{
            .m_param_types = l_param_types,
            .m_params = l_params.begin(),
            .m_body = l_body,
        };
        // define the input
        std::list<std::any> l_input;
        l_input.push_back(std::any(7));
        // make sure we get the right result
        assert(std::any_cast<int>(l_func(l_input)) == 32);
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
