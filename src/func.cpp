#include "../include/func.hpp"
#include <numeric>

std::any func_body::eval(const std::any* a_params, size_t a_param_count) const
{
    // if this holds a parameter, return the parameter
    if(const auto* l_param = std::get_if<param>(&m_data))
        return a_params[l_param->m_index];

    // get the functor, and evaluate it
    const auto& l_functor = std::get<functor>(m_data);
    // construct the arguments for the functor
    std::vector<std::any> l_functor_args(m_children.size());

    // evaluate all children
    std::transform(m_children.begin(), m_children.end(), l_functor_args.begin(),
                   [a_params, a_param_count](const func_body& a_child)
                   { return a_child.eval(a_params, a_param_count); });

    // evaluate the functor
    return l_functor(l_functor_args.data(), l_functor_args.size());
}

size_t func_body::node_count() const
{
    // count nodes in children, add one for this node
    return std::accumulate(m_children.begin(), m_children.end(), 1,
                           [](size_t a_sum, const func_body& a_child)
                           { return a_sum + a_child.node_count(); });
}

func::func(const std::type_index& a_return_type,
           const std::multimap<std::type_index, size_t>& a_param_types,
           const func_body& a_body, const std::string& a_repr)
    : m_return_type(a_return_type), m_param_types(a_param_types),
      m_body(a_body), m_repr(a_repr)
{
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_func_construction()
{
    const std::type_index l_return_type = typeid(int);
    const func_body l_body{
        .m_data = [](const std::any* a_params, size_t a_param_count)
        { return std::any(10); },
        .m_children = {},
    };
    const std::string l_repr = "func123";
    func l_func{l_return_type, {}, l_body, l_repr};
    assert(l_func.m_return_type == l_return_type);
    assert(l_func.m_param_types.empty());
    assert(l_func.m_body.node_count() == 1);
    assert(std::any_cast<int>(l_func.m_body.eval(nullptr, 0)) == 10);
    assert(l_func.m_repr == l_repr);
}

void test_func_body_eval()
{
    // nullary
    {
        func_body l_node{
            .m_data = [](const std::any* a_params, size_t a_param_count)
            { return std::any(10); },
            .m_children = {},
        };
        // evaluate the node
        auto l_result = l_node.eval(nullptr, 0);
        // assert that the type is int
        assert(l_result.type() == typeid(int));
        assert(std::any_cast<int>(l_result) == 10);
    }

    // unary
    {
        func_body l_node{
            .m_data = [](const std::any* a_params, size_t a_param_count)
            { return std::any(10 + std::any_cast<int>(a_params[0])); },
            .m_children =
                {
                    func_body{
                        .m_data = param{0},
                        .m_children = {},
                    },
                },
        };
        // create input
        std::vector<std::any> l_input{std::any(11)};
        // evaluate the node
        auto l_result = l_node.eval(l_input.data(), l_input.size());
        // assert that the type is int
        assert(l_result.type() == typeid(int));
        assert(std::any_cast<int>(l_result) == 21);
    }

    // binary
    {
        func_body l_node{
            .m_data =
                [](const std::any* a_params, size_t a_param_count)
            {
                return std::any(10 + std::any_cast<int>(a_params[0]) +
                                std::any_cast<int>(a_params[1]));
            },
            .m_children =
                {
                    func_body{
                        .m_data = param{0},
                        .m_children = {},
                    },
                    func_body{
                        .m_data = param{1},
                        .m_children = {},
                    },
                },
        };
        // create input
        std::vector<std::any> l_input{std::any(11), std::any(12)};
        // evaluate the node
        auto l_result = l_node.eval(l_input.data(), l_input.size());
        // assert that the type is int
        assert(l_result.type() == typeid(int));
        assert(std::any_cast<int>(l_result) == 33);
    }

    // doubly-nested unary
    {
        func_body l_node{
            .m_data = [](const std::any* a_params, size_t a_param_count)
            { return std::any(10 + std::any_cast<int>(a_params[0])); },
            .m_children =
                {
                    func_body{
                        .m_data =
                            [](const std::any* a_params, size_t a_param_count)
                        {
                            return std::any(11 +
                                            std::any_cast<int>(a_params[0]));
                        },
                        .m_children =
                            {
                                func_body{
                                    .m_data = param{0},
                                    .m_children = {},
                                },
                            },
                    },
                },
        };
        // create input
        std::vector<std::any> l_input{std::any(12)};
        // evaluate the node
        auto l_result = l_node.eval(l_input.data(), l_input.size());
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

void func_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_func_construction);
    TEST(test_func_body_eval);
    TEST(test_func_body_node_count);
}

#endif
