#include "../include/model.hpp"
#include "../include/program.hpp"
#include <cassert>

bool model::eval(std::list<std::any>::const_iterator a_begin,
                 std::list<std::any>::const_iterator a_end)
{
    // if both children are nullptr, then the model is homogenous
    if(m_negative_child == nullptr && m_positive_child == nullptr)
        return m_homogenous_value;

    // evaluate the binning function
    bool l_binning_result = std::any_cast<bool>(m_func->eval(a_begin, a_end));

    // get the appropriate child
    model* l_child =
        l_binning_result ? m_positive_child.get() : m_negative_child.get();

    return l_child->eval(a_begin, a_end);
}

#ifdef UNIT_TEST
#include "test_utils.hpp"

void test_model_eval()
{
    // immediately homogenous (falsy) model
    {
        model l_model{.m_homogenous_value = false};

        // construct input
        std::list<std::any> l_input;

        // evaluate the model
        bool l_result = l_model.eval(l_input.begin(), l_input.end());

        // check the result
        assert(l_result == false);
    }

    // immediately homogenous (truthy) model
    {
        model l_model{.m_homogenous_value = true};

        // construct input
        std::list<std::any> l_input;

        // evaluate the model
        bool l_result = l_model.eval(l_input.begin(), l_input.end());

        // check the result
        assert(l_result == true);
    }

    // model with 1 binning function
    {
        // construct program
        program l_program;

        // add a primitive binning function
        auto l_func_0 = l_program.add_primitive(
            "bin0", std::function([](int a_x) { return a_x > 0; }));

        // construct model
        model l_model{.m_func = l_func_0};

        // construct left child
        l_model.m_negative_child =
            std::make_unique<model>(model{.m_homogenous_value = false});

        // construct right child
        l_model.m_positive_child =
            std::make_unique<model>(model{.m_homogenous_value = true});

        // construct input
        std::list<std::any> l_input;

        // test truthy input
        l_input = {10};
        assert(l_model.eval(l_input.begin(), l_input.end()) == true);

        // test falsy input
        l_input = {-10};
        assert(l_model.eval(l_input.begin(), l_input.end()) == false);
    }

    // model with 1 binning function and a left child
    {
        // construct program
        program l_program;

        // add a primitive binning function
        auto l_func_0 = l_program.add_primitive(
            "bin0", std::function([](int a_x) { return a_x > 0; })); // positive

        // add another primitive binning function
        auto l_func_1 = l_program.add_primitive(
            "bin1",
            std::function([](int a_x) { return a_x % 2 == 0; })); // even

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
        std::list<std::any> l_input;

        // test input 10 (positive and even)
        l_input = {10};
        assert(l_model.eval(l_input.begin(), l_input.end()) == true);

        // test input 7 (positive and odd)
        l_input = {7};
        assert(l_model.eval(l_input.begin(), l_input.end()) == true);

        // test input -10 (negative and even)
        l_input = {-10};
        assert(l_model.eval(l_input.begin(), l_input.end()) == true);

        // test input -7 (negative and odd)
        l_input = {-7};
        assert(l_model.eval(l_input.begin(), l_input.end()) == false);
    }

    // model with 1 binning function and a left, and right child
    {
        // construct program
        program l_program;

        // add a primitive binning function
        auto l_func_0 = l_program.add_primitive(
            "bin0", std::function([](int a_x) { return a_x > 0; })); // positive

        // add another primitive binning function
        auto l_func_1 = l_program.add_primitive(
            "bin1",
            std::function([](int a_x) { return a_x % 2 == 0; })); // even

        // add another primitive binning function
        auto l_func_2 = l_program.add_primitive(
            "bin2", std::function([](int a_x)
                                  { return a_x % 3 == 0; })); // divisible by 3

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
        std::list<std::any> l_input;

        // positive, even, divisible by 3
        l_input = {6};
        assert(l_model.eval(l_input.begin(), l_input.end()) == true);

        // positive, even, not divisible by 3
        l_input = {4};
        assert(l_model.eval(l_input.begin(), l_input.end()) == false);

        // positive, odd, divisible by 3
        l_input = {9};
        assert(l_model.eval(l_input.begin(), l_input.end()) == true);

        // positive, odd, not divisible by 3
        l_input = {7};
        assert(l_model.eval(l_input.begin(), l_input.end()) == false);

        // negative, even, divisible by 3
        l_input = {-6};
        assert(l_model.eval(l_input.begin(), l_input.end()) == true);

        // negative, even, not divisible by 3
        l_input = {-4};
        assert(l_model.eval(l_input.begin(), l_input.end()) == true);

        // negative, odd, divisible by 3
        l_input = {-9};
        assert(l_model.eval(l_input.begin(), l_input.end()) == false);

        // negative, odd, not divisible by 3
        l_input = {-7};
        assert(l_model.eval(l_input.begin(), l_input.end()) == false);
    }
}

void model_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_model_eval);
}

#endif // UNIT_TEST
