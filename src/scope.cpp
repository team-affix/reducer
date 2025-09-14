#include "../include/scope.hpp"

// adds a function based on its arity and type
void scope::add_function(func* a_function)
{
    // add to the appropriate list
    if(a_function->m_param_types.empty())
        m_nullaries.emplace(a_function->m_return_type, a_function);
    else
        m_non_nullaries.emplace(a_function->m_return_type, a_function);
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_scope_entry_construction()
{
    scope l_scope;
    assert(l_scope.m_nullaries.empty());
    assert(l_scope.m_non_nullaries.empty());
}

void test_scope_entry_add_function()
{
    // add nullary fxn
    {
        scope l_scope;
        std::list<func> l_funcs;
        l_funcs.emplace_back(typeid(int),
                             std::multimap<std::type_index, size_t>{},
                             func_body{}, "f0");
        l_scope.add_function(&l_funcs.front());
        assert(l_scope.m_nullaries.size() == 1);
        assert(l_scope.m_non_nullaries.empty());
    }

    // add non-nullary fxn
    {
        scope l_scope;
        std::list<func> l_funcs;
        l_funcs.emplace_back(typeid(int),
                             std::multimap<std::type_index, size_t>{},
                             func_body{}, "f0");
        l_funcs.front().m_param_types.emplace(typeid(int), 0);
        l_scope.add_function(&l_funcs.front());
        assert(l_scope.m_nullaries.empty());
        assert(l_scope.m_non_nullaries.size() == 1);
    }
}

void test_scope_construction()
{
    scope l_scope;
    assert(l_scope.m_nullaries.empty());
    assert(l_scope.m_non_nullaries.empty());
}

void test_scope_add_function()
{
    // add nullary of return type int
    {
        scope l_scope;
        std::type_index l_return_type = std::type_index(typeid(int));
        std::list<func> l_funcs;
        l_funcs.emplace_back(l_return_type,
                             std::multimap<std::type_index, size_t>{},
                             func_body{}, "f0");
        l_scope.add_function(&l_funcs.front());
        assert(l_scope.m_nullaries.size() == 1);
        assert(l_scope.m_non_nullaries.empty());
        assert(l_scope.m_nullaries.contains(l_return_type));
        assert(l_scope.m_nullaries.find(l_return_type) !=
               l_scope.m_nullaries.end());
    }

    // add non-nullary of return type int, param string
    {
        scope l_scope;
        std::type_index l_return_type = std::type_index(typeid(int));
        std::list<func> l_funcs;
        l_funcs.emplace_back(l_return_type,
                             std::multimap<std::type_index, size_t>{},
                             func_body{}, "f0");
        l_funcs.front().m_param_types.emplace(typeid(std::string), 0);
        l_scope.add_function(&l_funcs.front());
        assert(l_scope.m_nullaries.empty());
        assert(l_scope.m_non_nullaries.size() == 1);
        assert(l_scope.m_non_nullaries.contains(l_return_type));
        assert(l_scope.m_non_nullaries.find(l_return_type) !=
               l_scope.m_non_nullaries.end());
    }

    // add non-nullary of return type string, param string
    {
        scope l_scope;
        std::type_index l_return_type = std::type_index(typeid(std::string));
        std::list<func> l_funcs;
        l_funcs.emplace_back(l_return_type,
                             std::multimap<std::type_index, size_t>{},
                             func_body{}, "f0");
        l_funcs.front().m_param_types.emplace(typeid(std::string), 0);
        l_scope.add_function(&l_funcs.front());
        assert(l_scope.m_nullaries.empty());
        assert(l_scope.m_non_nullaries.size() == 1);
        assert(l_scope.m_non_nullaries.contains(l_return_type));
        assert(l_scope.m_non_nullaries.find(l_return_type) !=
               l_scope.m_non_nullaries.end());
    }
}

void scope_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_scope_entry_construction);
    TEST(test_scope_entry_add_function);
    TEST(test_scope_construction);
    TEST(test_scope_add_function);
}

#endif
