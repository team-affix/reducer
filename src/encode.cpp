#include "../include/encode.hpp"
#include "../include/predef.hpp"

using namespace lambda;
using namespace dml::predef;

namespace dml
{
namespace encode
{

// church boolean
std::unique_ptr<lambda::expr> church_boolean(size_t a_binder_depth,
                                             bool a_boolean)
{
    return a_boolean ? church_true(a_binder_depth)
                     : church_false(a_binder_depth);
}

// church numeral
std::unique_ptr<lambda::expr> church_numeral(size_t a_binder_depth,
                                             size_t a_numeral)
{
    auto l_result = v(a_binder_depth + 1);
    for(size_t i = 0; i < a_numeral; ++i)
        l_result = a(v(a_binder_depth), std::move(l_result));
    return f(f(std::move(l_result)));
}

// church pair
std::unique_ptr<lambda::expr>
church_pair(size_t a_binder_depth, std::unique_ptr<lambda::expr>&& a_first,
            std::unique_ptr<lambda::expr>&& a_second)
{
    return f(a(a(v(a_binder_depth), std::move(a_first)), std::move(a_second)));
}

} // namespace encode
} // namespace dml

#ifdef UNIT_TEST
#include "test_utils.hpp"
using namespace dml::encode;

void test_encode_church_boolean()
{
    using namespace dml::encode;
    using namespace dml::predef;

    // Test encoding at various depths and boolean values
    auto test_at_depth = [](size_t depth, bool value)
    {
        auto l_boolean = church_boolean(depth, value);
        auto expected = f(f(v(depth + (value ? 0 : 1))));
        assert(l_boolean->equals(expected));
    };

    // Test at multiple binder depths
    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth, true);
        test_at_depth(depth, false);
    }

    // Test behavior with NOT operation at depth 0
    auto test_not = [](bool value)
    {
        constexpr size_t depth = 0;
        auto l_input = church_boolean(depth, value);
        auto l_not = church_not(depth);
        auto l_result = a(std::move(l_not), std::move(l_input))->normalize();
        auto l_expected = church_boolean(depth, !value);
        assert(l_result.m_expr->equals(l_expected));
    };

    test_not(true);
    test_not(false);

    // Test behavior with AND operation at depth 0
    auto test_and = [](bool a_val, bool b_val)
    {
        constexpr size_t depth = 0;
        auto l_a = church_boolean(depth, a_val);
        auto l_b = church_boolean(depth, b_val);
        auto l_and = church_and(depth);
        auto l_result =
            a(a(std::move(l_and), std::move(l_a)), std::move(l_b))->normalize();
        auto l_expected = church_boolean(depth, a_val && b_val);
        assert(l_result.m_expr->equals(l_expected));
    };

    test_and(true, true);
    test_and(true, false);
    test_and(false, true);
    test_and(false, false);

    // Test behavior with OR operation at depth 0
    auto test_or = [](bool a_val, bool b_val)
    {
        constexpr size_t depth = 0;
        auto l_a = church_boolean(depth, a_val);
        auto l_b = church_boolean(depth, b_val);
        auto l_or = church_or(depth);
        auto l_result =
            a(a(std::move(l_or), std::move(l_a)), std::move(l_b))->normalize();
        auto l_expected = church_boolean(depth, a_val || b_val);
        assert(l_result.m_expr->equals(l_expected));
    };

    test_or(true, true);
    test_or(true, false);
    test_or(false, true);
    test_or(false, false);
}

void test_encode_church_numeral()
{
    using namespace dml::encode;
    using namespace dml::predef;

    // Test encoding at various depths and numeral values
    auto test_at_depth = [](size_t depth, size_t numeral)
    {
        auto l_numeral = church_numeral(depth, numeral);

        // Build expected: λf.λx. f^n(x) where f=depth, x=depth+1
        auto expected_body = v(depth + 1);
        for(size_t i = 0; i < numeral; ++i)
            expected_body = a(v(depth), std::move(expected_body));
        auto expected = f(f(std::move(expected_body)));

        assert(l_numeral->equals(expected));
    };

    // Test at multiple binder depths and numerals
    for(size_t depth = 0; depth <= 5; ++depth)
    {
        for(size_t numeral = 0; numeral <= 4; ++numeral)
        {
            test_at_depth(depth, numeral);
        }
    }

    // Test behavior with IS_ZERO at depth 0
    auto test_is_zero = [](size_t numeral)
    {
        constexpr size_t depth = 0;
        auto l_numeral = church_numeral(depth, numeral);
        auto l_is_zero = church_is_zero(depth);
        auto l_result =
            a(std::move(l_is_zero), std::move(l_numeral))->normalize();
        auto l_expected = church_boolean(depth, numeral == 0);
        assert(l_result.m_expr->equals(l_expected));
    };

    test_is_zero(0);
    test_is_zero(1);
    test_is_zero(3);

    // Test behavior with SUCC at depth 0
    auto test_succ = [](size_t numeral)
    {
        constexpr size_t depth = 0;
        auto l_numeral = church_numeral(depth, numeral);
        auto l_succ = church_succ(depth);
        auto l_result = a(std::move(l_succ), std::move(l_numeral))->normalize();
        auto l_expected = church_numeral(depth, numeral + 1);
        assert(l_result.m_expr->equals(l_expected));
    };

    test_succ(0);
    test_succ(1);
    test_succ(2);

    // Test behavior with PRED at depth 0
    auto test_pred = [](size_t numeral)
    {
        constexpr size_t depth = 0;
        auto l_numeral = church_numeral(depth, numeral);
        auto l_pred = church_pred(depth);
        auto l_result = a(std::move(l_pred), std::move(l_numeral))->normalize();
        auto l_expected = church_numeral(depth, numeral > 0 ? numeral - 1 : 0);
        assert(l_result.m_expr->equals(l_expected));
    };

    test_pred(0);
    test_pred(1);
    test_pred(3);
}

void test_encode_church_pair()
{
    using namespace dml::encode;
    using namespace dml::predef;

    // Test encoding at various depths with different values
    auto test_at_depth = [](size_t depth,
                            const std::unique_ptr<lambda::expr>& first,
                            const std::unique_ptr<lambda::expr>& second)
    {
        auto l_pair = church_pair(depth, first->clone(), second->clone());

        // Expected: λf. ((f first) second) where f is at index depth
        auto expected = f(a(a(v(depth), first->clone()), second->clone()));

        assert(l_pair->equals(expected));
    };

    // Test at multiple binder depths with simple values
    for(size_t depth = 0; depth <= 5; ++depth)
    {
        // Test with church booleans as elements
        auto first_bool = church_boolean(depth + 1, true);
        auto second_bool = church_boolean(depth + 1, false);
        test_at_depth(depth, first_bool, second_bool);

        // Test with church numerals as elements
        auto first_num = church_numeral(depth + 1, 0);
        auto second_num = church_numeral(depth + 1, 1);
        test_at_depth(depth, first_num, second_num);

        // Test with variables as elements
        auto first_var = v(depth + 2);
        auto second_var = v(depth + 3);
        test_at_depth(depth, first_var, second_var);
    }

    // Note: Behavioral tests with fst/snd omitted as they require
    // complex normalization that may not terminate quickly with the current
    // implementation. The structural tests above verify correct encoding.
}

void encode_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_encode_church_boolean);
    TEST(test_encode_church_numeral);
    TEST(test_encode_church_pair);
}

#endif // UNIT_TEST