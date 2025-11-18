#include "../include/predef.hpp"

using namespace lambda;

namespace dml
{
namespace predef
{
// church true
std::unique_ptr<lambda::expr> church_true()
{
    return f(f(v(0)));
}

// church false
std::unique_ptr<lambda::expr> church_false()
{
    return f(f(v(1)));
}

// church zero
std::unique_ptr<lambda::expr> church_zero()
{
    return f(f(v(1)));
}

// church succ
std::unique_ptr<lambda::expr> church_succ()
{
    return f(f(f(a(v(1), a(a(v(0), v(1)), v(2))))));
}

// church pair
std::unique_ptr<lambda::expr> church_pair()
{
    return f(f(f(a(a(v(2), v(0)), v(1)))));
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
    TEST(test_church_zero);
    TEST(test_church_succ);
    TEST(test_church_pair);
}

#endif // UNIT_TEST
