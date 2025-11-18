#include "../include/predef.hpp"

// local variable macro
#define L(x) v(a_binder_depth + x)

using namespace lambda;

namespace dml
{
namespace predef
{
// church true
std::unique_ptr<lambda::expr> church_true(size_t a_binder_depth)
{
    return f(f(L(0)));
}

// church false
std::unique_ptr<lambda::expr> church_false(size_t a_binder_depth)
{
    return f(f(L(1)));
}

// church not
std::unique_ptr<lambda::expr> church_not(size_t a_binder_depth)
{
    return f(a(a(L(0), church_false(a_binder_depth + 1)),
               church_true(a_binder_depth + 1)));
}

// church and
std::unique_ptr<lambda::expr> church_and(size_t a_binder_depth)
{
    return f(f(a(a(L(0), L(1)), church_false(a_binder_depth + 2))));
}

// church or
std::unique_ptr<lambda::expr> church_or(size_t a_binder_depth)
{
    return f(f(a(a(L(0), church_true(a_binder_depth + 2)), L(1))));
}

// church zero
std::unique_ptr<lambda::expr> church_zero(size_t a_binder_depth)
{
    return f(f(L(1)));
}

// church succ
std::unique_ptr<lambda::expr> church_succ(size_t a_binder_depth)
{
    return f(f(f(a(L(1), a(a(L(0), L(1)), L(2))))));
}

// church is_zero
std::unique_ptr<lambda::expr> church_is_zero(size_t a_binder_depth)
{
    return f(a(a(L(0), f(church_false(a_binder_depth + 2))),
               church_true(a_binder_depth + 1)));
}

// church pair
std::unique_ptr<lambda::expr> church_pair(size_t a_binder_depth)
{
    return f(f(f(a(a(L(2), L(0)), L(1)))));
}

} // namespace predef
} // namespace dml

#ifdef UNIT_TEST

#include "test_utils.hpp"

// Helper to wrap expression with n lambdas
std::unique_ptr<lambda::expr> wrap_lambdas(std::unique_ptr<lambda::expr>&& expr,
                                           size_t n)
{
    for(size_t i = 0; i < n; ++i)
    {
        expr = f(std::move(expr));
    }
    return std::move(expr);
}

void test_church_true()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_true = church_true(depth);
        auto expected = f(f(v(depth)));
        assert(l_true->equals(expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_false()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_false = church_false(depth);
        auto expected = f(f(v(depth + 1)));
        assert(l_false->equals(expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_not()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_not = church_not(depth);
        auto expected =
            f(a(a(v(depth), f(f(v(depth + 2)))), f(f(v(depth + 1)))));
        assert(l_not->equals(expected));
        auto l_true = church_true(depth);
        auto l_not_true =
            wrap_lambdas(a(l_not->clone(), l_true->clone()), depth)
                ->normalize();
        assert(l_not_true.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
        auto l_false = church_false(depth);
        auto l_not_false =
            wrap_lambdas(a(l_not->clone(), l_false->clone()), depth)
                ->normalize();
        assert(l_not_false.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_and()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_and = church_and(depth);
        auto expected = f(f(a(a(v(depth), v(depth + 1)), f(f(v(depth + 3))))));
        assert(l_and->equals(expected));
        auto l_true = church_true(depth);
        auto l_false = church_false(depth);
        auto l_and_true_true =
            wrap_lambdas(a(a(l_and->clone(), l_true->clone()), l_true->clone()),
                         depth)
                ->normalize();
        assert(l_and_true_true.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_and_false_true =
            wrap_lambdas(
                a(a(l_and->clone(), l_false->clone()), l_true->clone()), depth)
                ->normalize();
        assert(l_and_false_true.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
        auto l_and_true_false =
            wrap_lambdas(
                a(a(l_and->clone(), l_true->clone()), l_false->clone()), depth)
                ->normalize();
        assert(l_and_true_false.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
        auto l_and_false_false =
            wrap_lambdas(
                a(a(l_and->clone(), l_false->clone()), l_false->clone()), depth)
                ->normalize();
        assert(l_and_false_false.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_or()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_or = church_or(depth);
        auto expected = f(f(a(a(v(depth), f(f(v(depth + 2)))), v(depth + 1))));
        assert(l_or->equals(expected));
        auto l_true = church_true(depth);
        auto l_false = church_false(depth);
        auto l_or_true_true =
            wrap_lambdas(a(a(l_or->clone(), l_true->clone()), l_true->clone()),
                         depth)
                ->normalize();
        assert(l_or_true_true.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_or_true_false =
            wrap_lambdas(a(a(l_or->clone(), l_true->clone()), l_false->clone()),
                         depth)
                ->normalize();
        assert(l_or_true_false.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_or_false_true =
            wrap_lambdas(a(a(l_or->clone(), l_false->clone()), l_true->clone()),
                         depth)
                ->normalize();
        assert(l_or_false_true.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_or_false_false =
            wrap_lambdas(
                a(a(l_or->clone(), l_false->clone()), l_false->clone()), depth)
                ->normalize();
        assert(l_or_false_false.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_zero()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_zero = church_zero(depth);
        auto expected = f(f(v(depth + 1)));
        assert(l_zero->equals(expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_succ()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_succ = church_succ(depth);
        auto expected = f(
            f(f(a(v(depth + 1), a(a(v(depth), v(depth + 1)), v(depth + 2))))));
        assert(l_succ->equals(expected));
        auto l_zero = church_zero(depth);
        auto l_one = wrap_lambdas(a(l_succ->clone(), l_zero->clone()), depth)
                         ->normalize();
        auto expected_one = f(f(a(v(depth), v(depth + 1))));
        assert(
            l_one.m_expr->equals(wrap_lambdas(std::move(expected_one), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_is_zero()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_is_zero = church_is_zero(depth);
        auto expected =
            f(a(a(v(depth), f(f(f(v(depth + 3))))), f(f(v(depth + 1)))));
        assert(l_is_zero->equals(expected));
        auto l_zero = church_zero(depth);
        auto l_is_zero_zero =
            wrap_lambdas(a(l_is_zero->clone(), l_zero->clone()), depth)
                ->normalize();
        assert(l_is_zero_zero.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_one = f(f(a(v(depth), v(depth + 1))));
        auto l_is_zero_one =
            wrap_lambdas(a(l_is_zero->clone(), l_one->clone()), depth)
                ->normalize();
        assert(l_is_zero_one.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_pair()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_pair = church_pair(depth);
        auto expected = f(f(f(a(a(v(depth + 2), v(depth)), v(depth + 1)))));
        assert(l_pair->equals(expected));
        auto l_first = wrap_lambdas(a(a(a(l_pair->clone(), church_false(depth)),
                                        church_true(depth)),
                                      church_true(depth)),
                                    depth)
                           ->normalize();
        assert(
            l_first.m_expr->equals(wrap_lambdas(church_false(depth), depth)));
        auto l_second =
            wrap_lambdas(a(a(a(l_pair->clone(), church_false(depth)),
                             church_true(depth)),
                           church_false(depth)),
                         depth)
                ->normalize();
        assert(
            l_second.m_expr->equals(wrap_lambdas(church_true(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void predef_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;
    TEST(test_church_true);
    TEST(test_church_false);
    TEST(test_church_not);
    TEST(test_church_and);
    TEST(test_church_or);
    TEST(test_church_zero);
    TEST(test_church_succ);
    TEST(test_church_is_zero);
    TEST(test_church_pair);
}

#endif // UNIT_TEST
