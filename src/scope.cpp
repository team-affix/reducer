#include "../include/scope.hpp"

// adds a function based on its arity
void scope::entry::add_function(std::list<func>::const_iterator a_function)
{
    // add to the appropriate list
    if(a_function->m_param_types.empty())
        m_nullaries.push_back(a_function);
    else
        m_non_nullaries.push_back(a_function);
}

// adds a function based on its return type
void scope::add_function(std::type_index a_return_type,
                         std::list<func>::const_iterator a_function)
{
    m_entries[a_return_type].add_function(a_function);
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_scope_entry_construction()
{
    scope::entry l_entry;
    assert(l_entry.m_nullaries.empty());
    assert(l_entry.m_non_nullaries.empty());
}

void test_scope_entry_add_function()
{
    // add nullary fxn
    {
        scope::entry l_entry;
        func l_func;
        std::list<func> l_funcs{l_func};
        l_entry.add_function(l_funcs.begin());
        assert(l_entry.m_nullaries.size() == 1);
        assert(l_entry.m_non_nullaries.empty());
    }

    // add non-nullary fxn
    {
        scope::entry l_entry;
        func l_func{
            .m_param_types = {std::type_index(typeid(int))},
        };
        std::list<func> l_funcs{l_func};
        l_entry.add_function(l_funcs.begin());
        assert(l_entry.m_nullaries.empty());
        assert(l_entry.m_non_nullaries.size() == 1);
    }
}

void test_scope_construction()
{
    scope l_scope;
    assert(l_scope.m_entries.empty());
}

void test_scope_add_function()
{
    // add nullary of return type int
    {
        scope l_scope;
        std::type_index l_return_type = std::type_index(typeid(int));
        func l_func;
        std::list<func> l_funcs{l_func};
        l_scope.add_function(l_return_type, l_funcs.begin());
        assert(l_scope.m_entries.size() == 1);
        assert(l_scope.m_entries.contains(l_return_type));
        // get the scope entry
        auto l_entry = l_scope.m_entries.at(l_return_type);
        assert(l_entry.m_nullaries.size() == 1);
        assert(l_entry.m_non_nullaries.empty());
    }

    // add non-nullary of return type int, param string
    {
        scope l_scope;
        std::type_index l_return_type = std::type_index(typeid(int));
        func l_func{
            .m_param_types = {std::type_index(typeid(std::string))},
        };
        std::list<func> l_funcs{l_func};
        l_scope.add_function(l_return_type, l_funcs.begin());
        assert(l_scope.m_entries.size() == 1);
        assert(l_scope.m_entries.contains(l_return_type));
        // get the scope entry
        auto l_entry = l_scope.m_entries.at(l_return_type);
        assert(l_entry.m_nullaries.empty());
        assert(l_entry.m_non_nullaries.size() == 1);
    }

    // add non-nullary of return type string, param string
    {
        scope l_scope;
        std::type_index l_return_type = std::type_index(typeid(std::string));
        func l_func{
            .m_param_types = {std::type_index(typeid(std::string))},
        };
        std::list<func> l_funcs{l_func};
        l_scope.add_function(l_return_type, l_funcs.begin());
        assert(l_scope.m_entries.size() == 1);
        assert(l_scope.m_entries.contains(l_return_type));
        // get the scope entry
        auto l_entry = l_scope.m_entries.at(l_return_type);
        assert(l_entry.m_nullaries.empty());
        assert(l_entry.m_non_nullaries.size() == 1);
    }
}

// void test_scope_validate()
// {
//     // scope with no entries validates
//     {
//         scope l_scope;
//         l_scope.validate();
//     }

//     // scope with one empty entry fails to validate
//     {
//         scope l_scope;
//         // default construct the entry
//         l_scope.m_entries[std::type_index(typeid(int))];
//         // validate the scope
//         assert_throws(l_scope.validate(), const std::runtime_error&);
//     }

//     // scope with one entry with one nullary validates
//     {
//         scope l_scope;
//         scope_entry l_entry{
//             .m_nullaries = {nullptr},
//         };
//         l_scope.validate();
//     }

//     // scope with one non-nullary entry fails
//     {
//         scope l_scope;
//         scope_entry l_entry{
//             .m_non_nullaries = {nullptr},
//         };
//         l_scope.m_entries[std::type_index(typeid(int))] = l_entry;
//         assert_throws(l_scope.validate(), const std::runtime_error&);
//     }

//     // scope with one nullary entry and one non-nullary
//     // entry fails
//     {
//         scope l_scope;
//         scope_entry l_nullary_entry{
//             .m_nullaries = {nullptr},
//         };
//         scope_entry l_non_nullary_entry{
//             .m_non_nullaries = {nullptr},
//         };
//         l_scope.m_entries[std::type_index(typeid(int))] = l_nullary_entry;
//         l_scope.m_entries[std::type_index(typeid(std::string))] =
//             l_non_nullary_entry;
//         assert_throws(l_scope.validate(), const std::runtime_error&);
//     }
// }

void scope_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_scope_entry_construction);
    TEST(test_scope_entry_add_function);
    TEST(test_scope_construction);
    TEST(test_scope_add_function);
}

#endif
