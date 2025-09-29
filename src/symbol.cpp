#include "../include/symbol.hpp"

symbol::symbol(const std::string& a_repr, const type& a_type)
    : m_repr(a_repr), m_type(a_type)
{
}

bool operator<(const symbol& a_lhs, const symbol& a_rhs)
{
    if(a_lhs.m_type < a_rhs.m_type)
        return true;
    if(a_rhs.m_type < a_lhs.m_type)
        return false;
    return a_lhs.m_repr < a_rhs.m_repr;
}

bool operator==(const symbol& a_lhs, const symbol& a_rhs)
{
    return a_lhs.m_repr == a_rhs.m_repr && a_lhs.m_type == a_rhs.m_type;
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_symbol_construction_and_equality()
{
    symbol l_symbol("x", type(type::constant("int"), {}));
    assert(l_symbol.m_repr == "x");
    assert(l_symbol.m_type == type(type::constant("int"), {}));
    assert(!(l_symbol == symbol("y", type(type::constant("int"), {}))));
    assert(!(l_symbol == symbol("x", type(type::constant("string"), {}))));
    assert(
        !(l_symbol == symbol("x", type(type::constant("int"),
                                       {type{type::constant("string"), {}}}))));
}

void symbol_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_symbol_construction_and_equality);
}

#endif
