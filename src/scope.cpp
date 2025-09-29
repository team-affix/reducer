#include "../include/scope.hpp"
#include "../include/symbol.hpp"
#include "../include/type.hpp"

scope::scope(const scope* a_parent) : m_parent(a_parent)
{
}

void scope::add_constant(const symbol& a_symbol, const std::any& a_value)
{
    m_constants.emplace(a_symbol, std::make_shared<std::any>(a_value));
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_scope_construction()
{
    scope l_scope(nullptr);

    assert(l_scope.m_parent == nullptr);
    assert(l_scope.m_children.empty());
    assert(l_scope.m_constants.empty());
}

void test_scope_add_constant()
{
    {
        scope l_scope(nullptr);
        symbol l_symbol("x", type(type::constant("int", {}), {}));

        l_scope.add_constant(l_symbol, 1);

        assert(l_scope.m_constants.size() == 1);
        assert(std::any_cast<int>(*l_scope.m_constants.at(l_symbol)) == 1);
    }

    {
        scope l_scope(nullptr);
        symbol l_symbol("x", type(type::constant("string", {}), {}));

        l_scope.add_constant(l_symbol, std::string("hello"));

        assert(l_scope.m_constants.size() == 1);
        assert(std::any_cast<std::string>(*l_scope.m_constants.at(l_symbol)) ==
               "hello");
    }
}

void scope_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_scope_construction);
    TEST(test_scope_add_constant);
}

#endif
