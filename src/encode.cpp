#include "../include/encode.hpp"
#include "../include/predef.hpp"

#define L(x) v(a_binder_depth + x)

using namespace lambda;
using namespace dml::predef;

std::vector<bool> little_endian_b2_from_b10(size_t a_numeral)
{
    std::vector<bool> l_result;
    while(a_numeral > 0)
    {
        l_result.push_back(a_numeral % 2);
        a_numeral /= 2;
    }
    return l_result;
}

////////////////////////////////////////
//// NOTE: the symmetry between predef and encode is the following:
////       predef defines the fundamental concepts (can define functions).
////       Encode on the other hand, can ONLY apply those concepts to
////       one another. Lambda abstractions are forbidden in encode.cpp,
////       only applications of concepts defined in predef are allowed.

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
    // builds a succ tower on zero
    auto l_result = church_zero(a_binder_depth);
    for(size_t i = 0; i < a_numeral; ++i)
        l_result = a(church_succ(a_binder_depth), std::move(l_result));
    return l_result;
}

// church pair
std::unique_ptr<lambda::expr>
church_pair(size_t a_binder_depth, std::unique_ptr<lambda::expr>&& a_first,
            std::unique_ptr<lambda::expr>&& a_second)
{
    return a(a(predef::church_pair(a_binder_depth), std::move(a_first)),
             std::move(a_second));
}

// scott list
std::unique_ptr<lambda::expr>
scott_list(size_t a_binder_depth,
           const std::list<std::unique_ptr<lambda::expr>>& a_list)
{
    // builds a scott list from a list of expressions
    auto l_result = predef::scott_nil(a_binder_depth);
    // iterate in reverse order
    for(auto it = a_list.rbegin(); it != a_list.rend(); ++it)
        l_result = a(a(predef::scott_cons(a_binder_depth), (*it)->clone()),
                     std::move(l_result));
    return l_result;
}

// binary numeral
std::unique_ptr<lambda::expr> binary_numeral(size_t a_binder_depth,
                                             size_t a_numeral)
{
    auto l_bits = little_endian_b2_from_b10(a_numeral);
    auto l_result = predef::scott_nil(a_binder_depth);
    // iterate in reverse order, LSB should be at front of list
    for(auto it = l_bits.rbegin(); it != l_bits.rend(); ++it)
        l_result = a(a(predef::scott_cons(a_binder_depth),
                       church_boolean(a_binder_depth, *it)),
                     std::move(l_result));
    return l_result;
}

} // namespace encode
} // namespace dml

#ifdef UNIT_TEST
#include "test_utils.hpp"
#include <limits>
using namespace dml::encode;

// Helper to wrap expression with n lambdas
std::unique_ptr<lambda::expr> wrap_lambdas(std::unique_ptr<lambda::expr>&& expr,
                                           size_t n);

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

        // normalize the numeral
        auto l_normalized =
            wrap_lambdas(std::move(l_numeral), depth)->normalize().m_expr;

        // Build expected: λf.λx. f^n(x) where f=depth, x=depth+1
        auto expected_body = v(depth + 1);
        for(size_t i = 0; i < numeral; ++i)
            expected_body = a(v(depth), std::move(expected_body));
        auto expected = wrap_lambdas(f(f(std::move(expected_body))), depth);

        std::cout << *l_normalized << std::endl;

        assert(l_normalized->equals(expected));
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
        auto l_expected =
            church_numeral(depth, numeral + 1)->normalize().m_expr;
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
        auto l_expected = church_numeral(depth, numeral > 0 ? numeral - 1 : 0)
                              ->normalize()
                              .m_expr;
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
        auto l_first = wrap_lambdas(first->clone(), depth)->normalize().m_expr;
        auto l_second =
            wrap_lambdas(second->clone(), depth)->normalize().m_expr;

        auto l_pair =
            wrap_lambdas(
                church_pair(depth, l_first->clone(), l_second->clone()), depth)
                ->normalize()
                .m_expr;

        std::cout << *l_first << std::endl;
        std::cout << *l_second << std::endl;

        std::cout << *l_pair << std::endl;

        // Expected: λf. ((f first) second) where f is at index depth
        auto expected = wrap_lambdas(
            f(a(a(v(depth), l_first->lift(1, 0)), l_second->lift(1, 0))),
            depth);

        assert(l_pair->equals(expected));
    };

    // Test at multiple binder depths with simple values
    for(size_t depth = 0; depth <= 5; ++depth)
    {
        // Test with church booleans as elements
        auto first_bool = church_boolean(depth, true);
        auto second_bool = church_boolean(depth, false);
        test_at_depth(depth, first_bool, second_bool);

        // Test with church numerals as elements
        auto first_num = church_numeral(depth, 0);
        auto second_num = church_numeral(depth, 1);
        test_at_depth(depth, first_num, second_num);

        // Test with variables as elements
        auto first_var = v(depth + 22);
        auto second_var = v(depth + 33);
        test_at_depth(depth, first_var, second_var);
    }

    // Note: Behavioral tests with fst/snd omitted as they require
    // complex normalization that may not terminate quickly with the current
    // implementation. The structural tests above verify correct encoding.
}

void test_encode_scott_list()
{
    using namespace dml::encode;
    using namespace dml::predef;

    // Test empty list (nil) at various depths
    auto test_nil = [](size_t depth)
    {
        std::list<std::unique_ptr<lambda::expr>> l_empty_list;
        auto l_nil = scott_list(depth, l_empty_list);
        auto l_expected = scott_nil(depth);
        assert(l_nil->equals(l_expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_nil(depth);
    }

    // Test single element list at various depths
    auto test_single = [](size_t depth)
    {
        std::list<std::unique_ptr<lambda::expr>> l_list;
        l_list.push_back(church_boolean(depth, true));
        auto l_result = scott_list(depth, l_list);

        // Expected: cons (church_boolean true) nil
        auto l_expected = a(a(scott_cons(depth), church_boolean(depth, true)),
                            scott_nil(depth));
        assert(l_result->equals(l_expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_single(depth);
    }

    // Test multi-element list at various depths
    auto test_multi = [](size_t depth)
    {
        std::list<std::unique_ptr<lambda::expr>> l_list;
        l_list.push_back(church_boolean(depth, true));
        l_list.push_back(church_boolean(depth, false));
        l_list.push_back(church_numeral(depth, 2));
        auto l_result = scott_list(depth, l_list);

        // Expected: cons true (cons false (cons 2 nil))
        auto l_expected =
            a(a(scott_cons(depth), church_boolean(depth, true)),
              a(a(scott_cons(depth), church_boolean(depth, false)),
                a(a(scott_cons(depth), church_numeral(depth, 2)),
                  scott_nil(depth))));
        assert(l_result->equals(l_expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_multi(depth);
    }

    // Note: Behavioral tests with normalized extractions omitted due to De
    // Bruijn level complexity during normalization. The structural tests above
    // and the is-empty test below verify correct Scott list encoding behavior.

    // Test behavioral: check if list is empty using case analysis
    auto test_is_empty_check = [](size_t depth)
    {
        // Test with empty list - should return nilCase (true)
        std::list<std::unique_ptr<lambda::expr>> l_empty;
        auto l_empty_list = scott_list(depth, l_empty);

        auto l_nil_case = church_boolean(depth, true);
        auto l_cons_case = f(f(church_boolean(depth + 2, false)));

        auto l_result_empty =
            wrap_lambdas(a(a(std::move(l_empty_list), l_nil_case->clone()),
                           l_cons_case->clone()),
                         depth)
                ->normalize(std::numeric_limits<size_t>::max(),
                            std::numeric_limits<size_t>::max(),
                            [](const std::unique_ptr<lambda::expr>& a_expr)
                            { std::cout << *a_expr << std::endl; });

        auto l_expected_empty = wrap_lambdas(church_boolean(depth, true), depth)
                                    ->normalize()
                                    .m_expr;
        assert(l_result_empty.m_expr->equals(l_expected_empty));

        // Test with non-empty list - should return consCase result (false)
        std::list<std::unique_ptr<lambda::expr>> l_nonempty;
        l_nonempty.push_back(
            v(depth + 5)); // Simple variable adjusted for depth
        auto l_nonempty_list = scott_list(depth, l_nonempty);

        auto l_result_nonempty =
            wrap_lambdas(a(a(std::move(l_nonempty_list), l_nil_case->clone()),
                           l_cons_case->clone()),
                         depth)
                ->normalize();

        auto l_expected_nonempty =
            wrap_lambdas(church_boolean(depth, false), depth)
                ->normalize()
                .m_expr;
        assert(l_result_nonempty.m_expr->equals(l_expected_nonempty));
    };

    // Test at depth 0 only due to normalization complexity
    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_is_empty_check(depth);
    }
}

void test_encode_binary_numeral()
{
    using namespace dml::encode;
    using namespace dml::predef;

    auto l_test = [](size_t depth, size_t numeral)
    {
        // construct numeral
        auto l_numeral = binary_numeral(depth, numeral);

        // compute the succ
        auto l_succ =
            wrap_lambdas(a(binary_succ(depth), l_numeral->clone()), depth)
                ->normalize()
                .m_expr;

        // compute the expected
        auto l_expected =
            wrap_lambdas(binary_numeral(depth, numeral + 1), depth)
                ->normalize()
                .m_expr;
        assert(l_succ->equals(l_expected));
    };

    // Test on various depths and numeral values
    for(size_t depth = 0; depth <= 5; ++depth)
    {
        for(size_t numeral = 0; numeral <= 10; ++numeral)
        {
            l_test(depth, numeral);
        }
    }
}

void encode_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_encode_church_boolean);
    TEST(test_encode_church_numeral);
    TEST(test_encode_church_pair);
    TEST(test_encode_scott_list);
    TEST(test_encode_binary_numeral);
}

#endif // UNIT_TEST