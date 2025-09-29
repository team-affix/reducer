#include "../include/model.hpp"
#include <cassert>

bool model::eval(const std::any* a_params)
{
    // if the model is homogenous, then return the homogenous value
    if(m_func == nullptr)
        return m_homogenous_value;

    // evaluate the binning function (these are always nullary)
    bool l_binning_result = m_func(a_params);

    // get the appropriate child
    model* l_child =
        l_binning_result ? m_positive_child.get() : m_negative_child.get();

    return l_child->eval(a_params);
}

size_t model::node_count() const
{
    size_t l_result = 1;

    if(m_func != nullptr)
    {
        l_result += m_negative_child->node_count();
        l_result += m_positive_child->node_count();
    }

    return l_result;
}

std::string model::repr() const
{
    if(m_func == nullptr)
        return std::to_string(m_homogenous_value);

    return "[" + m_func_repr + "] ? {" + m_positive_child->repr() + "} : {" +
           m_negative_child->repr() + "}";
}

#ifdef UNIT_TEST
#include "test_utils.hpp"

void test_model_eval()
{
    // immediately homogenous (falsy) model
    {
        model l_model{.m_homogenous_value = false};

        // evaluate the model
        bool l_result = l_model.eval(nullptr);

        // check the result
        assert(l_result == false);
    }

    // immediately homogenous (truthy) model
    {
        model l_model{.m_homogenous_value = true};

        // evaluate the model
        bool l_result = l_model.eval(nullptr);

        // check the result
        assert(l_result == true);
    }

    // model with 1 binning function
    {
        // add a primitive binning function
        auto l_func_0 =
            std::function([](const std::any* a_params)
                          { return std::any_cast<int>(*a_params) > 0; });

        // construct model
        model l_model{.m_func = l_func_0};

        // construct left child
        l_model.m_negative_child =
            std::make_unique<model>(model{.m_homogenous_value = false});

        // construct right child
        l_model.m_positive_child =
            std::make_unique<model>(model{.m_homogenous_value = true});

        // construct input
        std::vector<std::any> l_input;

        // test truthy input
        l_input = {10};
        assert(l_model.eval(l_input.data()) == true);

        // test falsy inputs
        l_input = {-10};
        assert(l_model.eval(l_input.data()) == false);
    }

    // model with 1 binning function and a left child
    {
        // add a primitive binning function
        auto l_func_0 =
            std::function([](const std::any* a_params)
                          { return std::any_cast<int>(*a_params) > 0; });

        // add another primitive binning function
        auto l_func_1 =
            std::function([](const std::any* a_params)
                          { return std::any_cast<int>(*a_params) % 2 == 0; });

        // construct model
        model l_model{.m_func = l_func_0};

        // construct left child
        l_model.m_negative_child =
            std::make_unique<model>(model{.m_func = l_func_1});

        // construct left-left child and left-right child
        l_model.m_negative_child->m_negative_child =
            std::make_unique<model>(model{.m_homogenous_value = false});
        l_model.m_negative_child->m_positive_child =
            std::make_unique<model>(model{.m_homogenous_value = true});

        // construct right child
        l_model.m_positive_child =
            std::make_unique<model>(model{.m_homogenous_value = true});

        // construct input
        std::vector<std::any> l_input;

        // test input 10 (positive and even)
        l_input = {10};
        assert(l_model.eval(l_input.data()) == true);

        // test input 7 (positive and odd)
        l_input = {7};
        assert(l_model.eval(l_input.data()) == true);

        // test input -10 (negative and even)
        l_input = {-10};
        assert(l_model.eval(l_input.data()) == true);

        // test input -7 (negative and odd)
        l_input = {-7};
        assert(l_model.eval(l_input.data()) == false);
    }

    // model with 1 binning function and a left, and right child
    {
        // add a primitive binning function
        auto l_func_0 =
            std::function([](const std::any* a_params)
                          { return std::any_cast<int>(*a_params) > 0; });

        // add another primitive binning function
        auto l_func_1 =
            std::function([](const std::any* a_params)
                          { return std::any_cast<int>(*a_params) % 2 == 0; });

        // add another primitive binning function
        auto l_func_2 =
            std::function([](const std::any* a_params)
                          { return std::any_cast<int>(*a_params) % 3 == 0; });

        // construct model
        model l_model{.m_func = l_func_0};

        // construct left child
        l_model.m_negative_child =
            std::make_unique<model>(model{.m_func = l_func_1});

        // construct left-left child and left-right child
        l_model.m_negative_child->m_negative_child =
            std::make_unique<model>(model{.m_homogenous_value = false});
        l_model.m_negative_child->m_positive_child =
            std::make_unique<model>(model{.m_homogenous_value = true});

        // construct right child
        l_model.m_positive_child =
            std::make_unique<model>(model{.m_func = l_func_2});

        // construct right-left child and right-right child
        l_model.m_positive_child->m_negative_child =
            std::make_unique<model>(model{.m_homogenous_value = false});
        l_model.m_positive_child->m_positive_child =
            std::make_unique<model>(model{.m_homogenous_value = true});

        // construct input
        std::vector<std::any> l_input;

        // positive, even, divisible by 3
        l_input = {6};
        assert(l_model.eval(l_input.data()) == true);

        // positive, even, not divisible by 3
        l_input = {4};
        assert(l_model.eval(l_input.data()) == false);

        // positive, odd, divisible by 3
        l_input = {9};
        assert(l_model.eval(l_input.data()) == true);

        // positive, odd, not divisible by 3
        l_input = {7};
        assert(l_model.eval(l_input.data()) == false);

        // negative, even, divisible by 3
        l_input = {-6};
        assert(l_model.eval(l_input.data()) == true);

        // negative, even, not divisible by 3
        l_input = {-4};
        assert(l_model.eval(l_input.data()) == true);

        // negative, odd, divisible by 3
        l_input = {-9};
        assert(l_model.eval(l_input.data()) == false);

        // negative, odd, not divisible by 3
        l_input = {-7};
        assert(l_model.eval(l_input.data()) == false);
    }
}

void model_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_model_eval);
}

#endif // UNIT_TEST
