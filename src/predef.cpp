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
    // Test at binder_depth = 0
    {
        auto l_true = church_true(0);
        assert(l_true->equals(f(f(v(0)))));
    }
    // Test at binder_depth = 1
    {
        auto l_true = wrap_lambdas(church_true(1), 1);
        assert(l_true->equals(f(f(f(v(1))))));
    }
    // Test at binder_depth = 2
    {
        auto l_true = wrap_lambdas(church_true(2), 2);
        assert(l_true->equals(f(f(f(f(v(2)))))));
    }
}

void test_church_false()
{
    using namespace dml::predef;
    // Test at binder_depth = 0
    {
        auto l_false = church_false(0);
        assert(l_false->equals(f(f(v(1)))));
    }
    // Test at binder_depth = 1
    {
        auto l_false = wrap_lambdas(church_false(1), 1);
        assert(l_false->equals(f(f(f(v(2))))));
    }
    // Test at binder_depth = 2
    {
        auto l_false = wrap_lambdas(church_false(2), 2);
        assert(l_false->equals(f(f(f(f(v(3)))))));
    }
}

void test_church_not()
{
    using namespace dml::predef;
    // Test at binder_depth = 0
    {
        auto l_not = church_not(0);
        assert(l_not->equals(f(a(a(v(0), f(f(v(2)))), f(f(v(1)))))));
        auto l_true = church_true(0);
        auto l_not_true = a(l_not->clone(), l_true->clone())->normalize();
        assert(l_not_true.m_expr->equals(church_false(0)));
        auto l_false = church_false(0);
        auto l_not_false = a(l_not->clone(), l_false->clone())->normalize();
        assert(l_not_false.m_expr->equals(church_true(0)));
    }

    // Test at binder_depth = 1
    {
        auto l_not = church_not(1);
        auto l_not_wrapped = wrap_lambdas(l_not->clone(), 1);
        assert(l_not_wrapped->equals(f(f(a(a(v(1), f(f(v(3)))), f(f(v(2))))))));
        auto l_true = church_true(1);
        auto l_not_true =
            wrap_lambdas(a(l_not->clone(), l_true->clone()), 1)->normalize();
        assert(l_not_true.m_expr->equals(wrap_lambdas(church_false(1), 1)));
        auto l_false = church_false(1);
        auto l_not_false =
            wrap_lambdas(a(l_not->clone(), l_false->clone()), 1)->normalize();
        assert(l_not_false.m_expr->equals(wrap_lambdas(church_true(1), 1)));
    }

    // Test at binder_depth = 2
    {
        auto l_not = church_not(2);
        auto l_not_wrapped = wrap_lambdas(l_not->clone(), 2);
        assert(
            l_not_wrapped->equals(f(f(f(a(a(v(2), f(f(v(4)))), f(f(v(3)))))))));
        auto l_true = church_true(2);
        auto l_not_true =
            wrap_lambdas(a(l_not->clone(), l_true->clone()), 2)->normalize();
        assert(l_not_true.m_expr->equals(wrap_lambdas(church_false(2), 2)));
        auto l_false = church_false(2);
        auto l_not_false =
            wrap_lambdas(a(l_not->clone(), l_false->clone()), 2)->normalize();
        assert(l_not_false.m_expr->equals(wrap_lambdas(church_true(2), 2)));
    }
}

void test_church_and()
{
    using namespace dml::predef;
    // Test at binder_depth = 0
    {
        auto l_and = church_and(0);
        assert(l_and->equals(f(f(a(a(v(0), v(1)), f(f(v(3))))))));
        auto l_true = church_true(0);
        auto l_false = church_false(0);
        auto l_and_true_true =
            a(a(l_and->clone(), l_true->clone()), l_true->clone())->normalize();
        assert(l_and_true_true.m_expr->equals(church_true(0)));
        auto l_and_false_true =
            a(a(l_and->clone(), l_false->clone()), l_true->clone())
                ->normalize();
        assert(l_and_false_true.m_expr->equals(church_false(0)));
        auto l_and_true_false =
            a(a(l_and->clone(), l_true->clone()), l_false->clone())
                ->normalize();
        assert(l_and_true_false.m_expr->equals(church_false(0)));
        auto l_and_false_false =
            a(a(l_and->clone(), l_false->clone()), l_false->clone())
                ->normalize();
        assert(l_and_false_false.m_expr->equals(church_false(0)));
    }

    // Test at binder_depth = 1
    {
        auto l_and = church_and(1);
        auto l_and_wrapped = wrap_lambdas(l_and->clone(), 1);
        assert(l_and_wrapped->equals(f(f(f(a(a(v(1), v(2)), f(f(v(4)))))))));
        auto l_true = church_true(1);
        auto l_false = church_false(1);
        auto l_and_true_true =
            wrap_lambdas(a(a(l_and->clone(), l_true->clone()), l_true->clone()),
                         1)
                ->normalize();
        assert(l_and_true_true.m_expr->equals(wrap_lambdas(church_true(1), 1)));
        auto l_and_false_false =
            wrap_lambdas(
                a(a(l_and->clone(), l_false->clone()), l_false->clone()), 1)
                ->normalize();
        assert(
            l_and_false_false.m_expr->equals(wrap_lambdas(church_false(1), 1)));
    }
}

void test_church_or()
{
    using namespace dml::predef;
    // Test at binder_depth = 0
    {
        auto l_or = church_or(0);
        assert(l_or->equals(f(f(a(a(v(0), f(f(v(2)))), v(1))))));
        auto l_true = church_true(0);
        auto l_false = church_false(0);
        auto l_or_true_true =
            a(a(l_or->clone(), l_true->clone()), l_true->clone())->normalize();
        assert(l_or_true_true.m_expr->equals(church_true(0)));
        auto l_or_true_false =
            a(a(l_or->clone(), l_true->clone()), l_false->clone())->normalize();
        assert(l_or_true_false.m_expr->equals(church_true(0)));
        auto l_or_false_true =
            a(a(l_or->clone(), l_false->clone()), l_true->clone())->normalize();
        assert(l_or_false_true.m_expr->equals(church_true(0)));
        auto l_or_false_false =
            a(a(l_or->clone(), l_false->clone()), l_false->clone())
                ->normalize();
        assert(l_or_false_false.m_expr->equals(church_false(0)));
    }

    // Test at binder_depth = 1
    {
        auto l_or = church_or(1);
        auto l_or_wrapped = wrap_lambdas(l_or->clone(), 1);
        assert(l_or_wrapped->equals(f(f(f(a(a(v(1), f(f(v(3)))), v(2)))))));
        auto l_true = church_true(1);
        auto l_false = church_false(1);
        auto l_or_true_true =
            wrap_lambdas(a(a(l_or->clone(), l_true->clone()), l_true->clone()),
                         1)
                ->normalize();
        assert(l_or_true_true.m_expr->equals(wrap_lambdas(church_true(1), 1)));
        auto l_or_true_false =
            wrap_lambdas(a(a(l_or->clone(), l_true->clone()), l_false->clone()),
                         1)
                ->normalize();
        assert(l_or_true_false.m_expr->equals(wrap_lambdas(church_true(1), 1)));
        auto l_or_false_true =
            wrap_lambdas(a(a(l_or->clone(), l_false->clone()), l_true->clone()),
                         1)
                ->normalize();
        assert(l_or_false_true.m_expr->equals(wrap_lambdas(church_true(1), 1)));
        auto l_or_false_false =
            wrap_lambdas(
                a(a(l_or->clone(), l_false->clone()), l_false->clone()), 1)
                ->normalize();
        assert(
            l_or_false_false.m_expr->equals(wrap_lambdas(church_false(1), 1)));
    }

    // Test at binder_depth = 2
    {
        auto l_or = church_or(2);
        auto l_or_wrapped = wrap_lambdas(l_or->clone(), 2);
        assert(l_or_wrapped->equals(f(f(f(f(a(a(v(2), f(f(v(4)))), v(3))))))));
        auto l_true = church_true(2);
        auto l_false = church_false(2);
        auto l_or_true_true =
            wrap_lambdas(a(a(l_or->clone(), l_true->clone()), l_true->clone()),
                         2)
                ->normalize();
        assert(l_or_true_true.m_expr->equals(wrap_lambdas(church_true(2), 2)));
        auto l_or_true_false =
            wrap_lambdas(a(a(l_or->clone(), l_true->clone()), l_false->clone()),
                         2)
                ->normalize();
        assert(l_or_true_false.m_expr->equals(wrap_lambdas(church_true(2), 2)));
        auto l_or_false_true =
            wrap_lambdas(a(a(l_or->clone(), l_false->clone()), l_true->clone()),
                         2)
                ->normalize();
        assert(l_or_false_true.m_expr->equals(wrap_lambdas(church_true(2), 2)));
        auto l_or_false_false =
            wrap_lambdas(
                a(a(l_or->clone(), l_false->clone()), l_false->clone()), 2)
                ->normalize();
        assert(
            l_or_false_false.m_expr->equals(wrap_lambdas(church_false(2), 2)));
    }
}

void test_church_zero()
{
    using namespace dml::predef;
    // Test at binder_depth = 0
    {
        auto l_zero = church_zero(0);
        assert(l_zero->equals(f(f(v(1)))));
    }
    // Test at binder_depth = 1
    {
        auto l_zero = wrap_lambdas(church_zero(1), 1);
        assert(l_zero->equals(f(f(f(v(2))))));
    }
    // Test at binder_depth = 2
    {
        auto l_zero = wrap_lambdas(church_zero(2), 2);
        assert(l_zero->equals(f(f(f(f(v(3)))))));
    }
}

void test_church_succ()
{
    using namespace dml::predef;
    // Test at binder_depth = 0
    {
        auto l_succ = church_succ(0);
        assert(l_succ->equals(f(f(f(a(v(1), a(a(v(0), v(1)), v(2))))))));
        auto l_zero = church_zero(0);
        auto l_one = a(l_succ->clone(), l_zero->clone())->normalize();
        assert(l_one.m_expr->equals(f(f(a(v(0), v(1))))));
    }

    // Test at binder_depth = 1
    {
        auto l_succ = church_succ(1);
        auto l_succ_wrapped = wrap_lambdas(l_succ->clone(), 1);
        assert(l_succ_wrapped->equals(
            f(f(f(f(a(v(2), a(a(v(1), v(2)), v(3)))))))));
        auto l_zero = church_zero(1);
        auto l_one =
            wrap_lambdas(a(l_succ->clone(), l_zero->clone()), 1)->normalize();
        assert(l_one.m_expr->equals(wrap_lambdas(f(f(a(v(1), v(2)))), 1)));
    }

    // Test at binder_depth = 2
    {
        auto l_succ = church_succ(2);
        auto l_succ_wrapped = wrap_lambdas(l_succ->clone(), 2);
        assert(l_succ_wrapped->equals(
            f(f(f(f(f(a(v(3), a(a(v(2), v(3)), v(4))))))))));
        auto l_zero = church_zero(2);
        auto l_one =
            wrap_lambdas(a(l_succ->clone(), l_zero->clone()), 2)->normalize();
        assert(l_one.m_expr->equals(wrap_lambdas(f(f(a(v(2), v(3)))), 2)));
    }
}

void test_church_is_zero()
{
    using namespace dml::predef;
    // Test at binder_depth = 0
    {
        auto l_is_zero = church_is_zero(0);
        assert(l_is_zero->equals(f(a(a(v(0), f(f(f(v(3))))), f(f(v(1)))))));
        auto l_zero = church_zero(0);
        auto l_is_zero_zero =
            a(l_is_zero->clone(), l_zero->clone())->normalize();
        assert(l_is_zero_zero.m_expr->equals(church_true(0)));
        auto l_one = f(f(a(v(0), v(1))));
        auto l_is_zero_one = a(l_is_zero->clone(), l_one->clone())->normalize();
        assert(l_is_zero_one.m_expr->equals(church_false(0)));
        auto l_two = f(f(a(v(0), a(v(0), v(1)))));
        auto l_is_zero_two = a(l_is_zero->clone(), l_two->clone())->normalize();
        assert(l_is_zero_two.m_expr->equals(church_false(0)));
    }

    // Test at binder_depth = 1
    {
        auto l_is_zero = church_is_zero(1);
        auto l_is_zero_wrapped = wrap_lambdas(l_is_zero->clone(), 1);
        assert(l_is_zero_wrapped->equals(
            f(f(a(a(v(1), f(f(f(v(4))))), f(f(v(2))))))));
        auto l_zero = church_zero(1);
        auto l_is_zero_zero =
            wrap_lambdas(a(l_is_zero->clone(), l_zero->clone()), 1)
                ->normalize();
        assert(l_is_zero_zero.m_expr->equals(wrap_lambdas(church_true(1), 1)));
        auto l_one = f(f(a(v(1), v(2))));
        auto l_is_zero_one =
            wrap_lambdas(a(l_is_zero->clone(), l_one->clone()), 1)->normalize();
        assert(l_is_zero_one.m_expr->equals(wrap_lambdas(church_false(1), 1)));
    }

    // Test at binder_depth = 2
    {
        auto l_is_zero = church_is_zero(2);
        auto l_is_zero_wrapped = wrap_lambdas(l_is_zero->clone(), 2);
        assert(l_is_zero_wrapped->equals(
            f(f(f(a(a(v(2), f(f(f(v(5))))), f(f(v(3)))))))));
        auto l_zero = church_zero(2);
        auto l_is_zero_zero =
            wrap_lambdas(a(l_is_zero->clone(), l_zero->clone()), 2)
                ->normalize();
        assert(l_is_zero_zero.m_expr->equals(wrap_lambdas(church_true(2), 2)));
        auto l_one = f(f(a(v(2), v(3))));
        auto l_is_zero_one =
            wrap_lambdas(a(l_is_zero->clone(), l_one->clone()), 2)->normalize();
        assert(l_is_zero_one.m_expr->equals(wrap_lambdas(church_false(2), 2)));
    }
}

void test_church_pair()
{
    using namespace dml::predef;
    // Test at binder_depth = 0
    {
        auto l_pair = church_pair(0);
        assert(l_pair->equals(f(f(f(a(a(v(2), v(0)), v(1)))))));
        auto l_pair_ft =
            a(a(l_pair->clone(), church_false(0)), church_true(0))->normalize();
        auto l_first =
            a(l_pair_ft.m_expr->clone(), church_true(0))->normalize();
        assert(l_first.m_expr->equals(church_false(0)));
        auto l_second =
            a(l_pair_ft.m_expr->clone(), church_false(0))->normalize();
        assert(l_second.m_expr->equals(church_true(0)));
    }

    // Test at binder_depth = 1
    {
        auto l_pair = church_pair(1);
        auto l_pair_wrapped = wrap_lambdas(l_pair->clone(), 1);
        assert(l_pair_wrapped->equals(f(f(f(f(a(a(v(3), v(1)), v(2))))))));
        auto l_first = wrap_lambdas(a(a(a(l_pair->clone(), church_false(1)),
                                        church_true(1)),
                                      church_true(1)),
                                    1)
                           ->normalize();
        assert(l_first.m_expr->equals(wrap_lambdas(church_false(1), 1)));
        auto l_second = wrap_lambdas(a(a(a(l_pair->clone(), church_false(1)),
                                         church_true(1)),
                                       church_false(1)),
                                     1)
                            ->normalize();
        assert(l_second.m_expr->equals(wrap_lambdas(church_true(1), 1)));
    }

    // Test at binder_depth = 2
    {
        auto l_pair = church_pair(2);
        auto l_pair_wrapped = wrap_lambdas(l_pair->clone(), 2);
        assert(l_pair_wrapped->equals(f(f(f(f(f(a(a(v(4), v(2)), v(3)))))))));
        auto l_first = wrap_lambdas(a(a(a(l_pair->clone(), church_false(2)),
                                        church_true(2)),
                                      church_true(2)),
                                    2)
                           ->normalize();
        assert(l_first.m_expr->equals(wrap_lambdas(church_false(2), 2)));
        auto l_second = wrap_lambdas(a(a(a(l_pair->clone(), church_false(2)),
                                         church_true(2)),
                                       church_false(2)),
                                     2)
                            ->normalize();
        assert(l_second.m_expr->equals(wrap_lambdas(church_true(2), 2)));
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
