#include "../include/scope.hpp"
#include <stdexcept>

// adds a function based on its arity
void scope_entry_t::add_function(const func* a_function)
{
    if(a_function->m_param_types.empty())
        m_nullaries.push_back(a_function);
    else
        m_non_nullaries.push_back(a_function);
}

// checks that the nullaries are not empty
void scope_entry_t::validate() const
{
    if(m_nullaries.empty())
        throw std::runtime_error("Type has no nullaries");
}

// adds a function based on its return type
void scope_t::add_function(std::type_index a_return_type,
                           const func* a_function)
{
    m_entries[a_return_type].add_function(a_function);
}

// checks that all types have at least one nullary
void scope_t::validate() const
{
    for(const auto& [_, entry] : m_entries)
        entry.validate();
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_scope_entry_construction()
{
    scope_entry_t l_entry;
    assert(l_entry.m_nullaries.empty());
    assert(l_entry.m_non_nullaries.empty());
}

void test_scope_entry_add_function()
{
    // add nullary fxn
    {
        scope_entry_t l_entry;
        func l_func;
        l_entry.add_function(&l_func);
        assert(l_entry.m_nullaries.size() == 1);
        assert(l_entry.m_non_nullaries.empty());
    }

    // add non-nullary fxn
    {
        scope_entry_t l_entry;
        func l_func{
            .m_param_types = {std::type_index(typeid(int))},
        };
        l_entry.add_function(&l_func);
        assert(l_entry.m_nullaries.empty());
        assert(l_entry.m_non_nullaries.size() == 1);
    }
}

void test_scope_entry_validate()
{
    // empty entry fails to validate
    {
        scope_entry_t l_entry;
        assert_throws(l_entry.validate(),
                      const std::runtime_error&);
    }
    // entry with nullary AND non-nullary validates
    {
        scope_entry_t l_entry;
        func l_nullary{
            .m_param_types = {},
        };
        func l_non_nullary{
            .m_param_types = {std::type_index(typeid(int))},
        };
        l_entry.add_function(&l_nullary);
        l_entry.add_function(&l_non_nullary);
        l_entry.validate();
    }
    // entry with only non-nullary does not validate
    {
        scope_entry_t l_entry;
        func l_func{
            .m_param_types = {std::type_index(typeid(int))},
        };
        l_entry.add_function(&l_func);
        assert_throws(l_entry.validate(),
                      const std::runtime_error&);
    }
}

void test_scope_construction()
{
    scope_t l_scope;
    assert(l_scope.m_entries.empty());
}

void test_scope_add_function()
{
    // add nullary of return type int
    {
        scope_t l_scope;
        std::type_index l_return_type =
            std::type_index(typeid(int));
        func l_func;
        l_scope.add_function(l_return_type, &l_func);
        assert(l_scope.m_entries.size() == 1);
        assert(l_scope.m_entries.contains(l_return_type));
        // get the scope entry
        auto l_entry = l_scope.m_entries.at(l_return_type);
        assert(l_entry.m_nullaries.size() == 1);
        assert(l_entry.m_non_nullaries.empty());
    }

    // add non-nullary of return type int, param string
    {
        scope_t l_scope;
        std::type_index l_return_type =
            std::type_index(typeid(int));
        func l_func{
            .m_param_types = {std::type_index(
                typeid(std::string))},
        };
        l_scope.add_function(l_return_type, &l_func);
        assert(l_scope.m_entries.size() == 1);
        assert(l_scope.m_entries.contains(l_return_type));
        // get the scope entry
        auto l_entry = l_scope.m_entries.at(l_return_type);
        assert(l_entry.m_nullaries.empty());
        assert(l_entry.m_non_nullaries.size() == 1);
    }

    // add non-nullary of return type string, param string
    {
        scope_t l_scope;
        std::type_index l_return_type =
            std::type_index(typeid(std::string));
        func l_func{
            .m_param_types = {std::type_index(
                typeid(std::string))},
        };
        l_scope.add_function(l_return_type, &l_func);
        assert(l_scope.m_entries.size() == 1);
        assert(l_scope.m_entries.contains(l_return_type));
        // get the scope entry
        auto l_entry = l_scope.m_entries.at(l_return_type);
        assert(l_entry.m_nullaries.empty());
        assert(l_entry.m_non_nullaries.size() == 1);
    }
}

void test_scope_validate()
{
    // scope with no entries validates
    {
        scope_t l_scope;
        l_scope.validate();
    }

    // scope with one empty entry fails to validate
    {
        scope_t l_scope;
        // default construct the entry
        l_scope.m_entries[std::type_index(typeid(int))];
        // validate the scope
        assert_throws(l_scope.validate(),
                      const std::runtime_error&);
    }

    // scope with one entry with one nullary validates
    {
        scope_t l_scope;
        scope_entry_t l_entry{
            .m_nullaries = {nullptr},
        };
        l_scope.validate();
    }

    // scope with one non-nullary entry fails
    {
        scope_t l_scope;
        scope_entry_t l_entry{
            .m_non_nullaries = {nullptr},
        };
        l_scope.m_entries[std::type_index(typeid(int))] =
            l_entry;
        assert_throws(l_scope.validate(),
                      const std::runtime_error&);
    }

    // scope with one nullary entry and one non-nullary
    // entry fails
    {
        scope_t l_scope;
        scope_entry_t l_nullary_entry{
            .m_nullaries = {nullptr},
        };
        scope_entry_t l_non_nullary_entry{
            .m_non_nullaries = {nullptr},
        };
        l_scope.m_entries[std::type_index(typeid(int))] =
            l_nullary_entry;
        l_scope.m_entries[std::type_index(
            typeid(std::string))] = l_non_nullary_entry;
        assert_throws(l_scope.validate(),
                      const std::runtime_error&);
    }
}

void scope_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_scope_entry_construction);
    TEST(test_scope_entry_add_function);
    TEST(test_scope_entry_validate);
    TEST(test_scope_construction);
    TEST(test_scope_add_function);
    TEST(test_scope_validate);
}

#endif
