#include "../include/func.hpp"
#include <numeric>

std::any func_node_t::eval() const
{
    // construct the arguments for the functor
    std::list<std::any> l_functor_args;

    // evaluate all children
    std::transform(m_children.begin(), m_children.end(),
                   std::back_inserter(l_functor_args),
                   [](const func_node_t& a_child)
                   { return a_child.eval(); });

    // evaluate the functor
    return m_functor(l_functor_args);
}

size_t func_node_t::node_count() const
{
    // count nodes in children, add one for this node
    return std::accumulate(
        m_children.begin(), m_children.end(), 1,
        [](size_t a_sum, const func_node_t& a_child)
        { return a_sum + a_child.node_count(); });
}

std::any
func_t::operator()(const std::list<std::any>& a_args) const
{
    // set the parameters
    std::copy(a_args.begin(), a_args.end(), m_params);
    // evaluate the function
    return m_definition.eval();
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_func_construction()
{
    func_t l_func;
}

void test_func_node_eval()
{
    // nullary
    {
        func_node_t l_node{
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
        func_node_t l_node{
            .m_functor =
                [](const std::list<std::any>& a_args)
            {
                return 10 +
                       std::any_cast<int>(a_args.front());
            },
            .m_children =
                {
                    func_node_t{
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
        func_node_t l_node{
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
                    func_node_t{
                        .m_functor =
                            [](const std::list<std::any>&
                                   a_args)
                        { return std::any(11); },
                    },
                    func_node_t{
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
        func_node_t l_node{
            .m_functor =
                [](const std::list<std::any>& a_args)
            {
                return std::any(10 + std::any_cast<int>(
                                         a_args.front()));
            },
            .m_children =
                {
                    func_node_t{
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
                                func_node_t{
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

void func_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_func_construction);
    TEST(test_func_node_eval);
}

#endif
