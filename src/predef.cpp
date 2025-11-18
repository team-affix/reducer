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

void test_church_true()
{
    using namespace dml::predef;
    auto l_true = church_true();
    assert(l_true->equals(f(f(v(0)))));
}

void test_church_false()
{
    using namespace dml::predef;
    auto l_false = church_false();
    assert(l_false->equals(f(f(v(1)))));
}

void test_church_not()
{
    using namespace dml::predef;
    auto l_not = church_not();
    assert(l_not->equals(f(a(a(v(0), f(f(v(2)))), f(f(v(1)))))));
    // make sure that not(true) = false
    auto l_true = church_true();
    auto l_not_true = a(l_not->clone(), l_true->clone())->normalize();
    assert(l_not_true.m_expr->equals(church_false()));
    // make sure that not(false) = true
    auto l_false = church_false();
    auto l_not_false = a(l_not->clone(), l_false->clone())->normalize();
    assert(l_not_false.m_expr->equals(church_true()));
}

void test_church_and()
{
    using namespace dml::predef;
    auto l_and = church_and();
    assert(l_and->equals(f(f(a(a(v(0), v(1)), f(f(v(3))))))));
    // make sure that and(true, true) = true
    auto l_true = church_true();
    auto l_and_true_true =
        a(a(l_and->clone(), l_true->clone()), l_true->clone())->normalize();
    assert(l_and_true_true.m_expr->equals(church_true()));
    // make sure that and(true, false) = false
    auto l_false = church_false();
    auto l_and_false_true =
        a(a(l_and->clone(), l_false->clone()), l_true->clone())->normalize();
    assert(l_and_false_true.m_expr->equals(church_false()));
    // make sure that and(false, true) = false
    auto l_and_true_false =
        a(a(l_and->clone(), l_true->clone()), l_false->clone())->normalize();
    assert(l_and_true_false.m_expr->equals(church_false()));
    // make sure that and(false, false) = false
    auto l_and_false_false =
        a(a(l_and->clone(), l_false->clone()), l_false->clone())->normalize();
    assert(l_and_false_false.m_expr->equals(church_false()));
}

void test_church_or()
{
    using namespace dml::predef;
    auto l_or = church_or();
    assert(l_or->equals(f(f(a(a(v(0), f(f(v(2)))), v(1))))));
    // make sure that or(true, true) = true
    auto l_true = church_true();
    auto l_or_true_true =
        a(a(l_or->clone(), l_true->clone()), l_true->clone())->normalize();
    assert(l_or_true_true.m_expr->equals(church_true()));
    // make sure that or(true, false) = true
    auto l_false = church_false();
    auto l_or_true_false =
        a(a(l_or->clone(), l_true->clone()), l_false->clone())->normalize();
    assert(l_or_true_false.m_expr->equals(church_true()));
    // make sure that or(false, true) = true
    auto l_or_false_true =
        a(a(l_or->clone(), l_false->clone()), l_true->clone())->normalize();
    assert(l_or_false_true.m_expr->equals(church_true()));
    // make sure that or(false, false) = false
    auto l_or_false_false =
        a(a(l_or->clone(), l_false->clone()), l_false->clone())->normalize();
    assert(l_or_false_false.m_expr->equals(church_false()));
}

void test_church_zero()
{
    using namespace dml::predef;
    auto l_zero = church_zero();
    assert(l_zero->equals(f(f(v(1)))));
}

void test_church_succ()
{
    using namespace dml::predef;
    auto l_succ = church_succ();
    assert(l_succ->equals(f(f(f(a(v(1), a(a(v(0), v(1)), v(2))))))));
    // make sure that succ(zero) = one
    auto l_zero = church_zero();
    auto l_one = a(l_succ->clone(), l_zero->clone())->normalize();
    assert(l_one.m_expr->equals(f(f(a(v(0), v(1))))));
}

void test_church_is_zero()
{
    using namespace dml::predef;
    auto l_is_zero = church_is_zero();
    assert(l_is_zero->equals(f(a(a(v(0), f(f(f(v(3))))), f(f(v(1)))))));
    // make sure that is_zero(zero) = true
    auto l_zero = church_zero();
    auto l_is_zero_zero = a(l_is_zero->clone(), l_zero->clone())->normalize();
    assert(l_is_zero_zero.m_expr->equals(church_true()));
    // make sure that is_zero(one) = false
    auto l_one = f(f(a(v(0), v(1))));
    auto l_is_zero_one = a(l_is_zero->clone(), l_one->clone())->normalize();
    assert(l_is_zero_one.m_expr->equals(church_false()));
    // make sure that is_zero(two) = false
    auto l_two = f(f(a(v(0), a(v(0), v(1)))));
    auto l_is_zero_two = a(l_is_zero->clone(), l_two->clone())->normalize();
    assert(l_is_zero_two.m_expr->equals(church_false()));
}

void test_church_pair()
{
    using namespace dml::predef;
    auto l_pair = church_pair();
    assert(l_pair->equals(f(f(f(a(a(v(2), v(0)), v(1)))))));
    // construct pair (false, true)
    auto l_pair_2 =
        a(a(l_pair->clone(), church_false()), church_true())->normalize();
    // get first element using true selector
    auto l_first = a(l_pair_2.m_expr->clone(), church_true())->normalize();
    // get second element using false selector
    auto l_second = a(l_pair_2.m_expr->clone(), church_false())->normalize();
    // assertions
    assert(l_first.m_expr->equals(church_false()));
    assert(l_second.m_expr->equals(church_true()));
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
