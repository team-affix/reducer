#include "../include/type.hpp"

// type func_type(const type& a_return_type,
//                const std::vector<type>& a_param_types)
// {
//     // the first dependency is the return type
//     // the second dependency is a tuple of the param types
//     return type{.m_root = "func",
//                 .m_deps = {a_return_type, type{
//                                               .m_root = "tuple",
//                                               .m_deps = a_param_types,
//                                           }}};
// }

// type get_return_type(const type& a_type)
// {
//     // the first dependency is the return type
//     return a_type.m_deps[0];
// }

// std::vector<type> get_param_types(const type& a_type)
// {
//     // the second dependency is a tuple of the param types
//     return a_type.m_deps[1].m_deps;
// }

// bool is_func_type(const type& a_type)
// {
//     return a_type.m_root == "func";
// }

type::variable::variable(size_t a_index) : m_index(a_index)
{
}

type::constant::constant(const std::string& a_name) : m_name(a_name)
{
}

type::type(const std::variant<constant, variable>& a_root,
           const std::vector<type>& a_deps)
    : m_root(a_root), m_deps(a_deps)
{
}

bool operator<(const type::variable& a_lhs, const type::variable& a_rhs)
{
    return a_lhs.m_index < a_rhs.m_index;
}

bool operator<(const type::constant& a_lhs, const type::constant& a_rhs)
{
    return a_lhs.m_name < a_rhs.m_name;
}

bool operator<(const type& a_lhs, const type& a_rhs)
{
    if(a_lhs.m_root < a_rhs.m_root)
        return true;
    if(a_rhs.m_root < a_lhs.m_root)
        return false;
    return a_lhs.m_deps < a_rhs.m_deps;
}

bool operator==(const type::variable& a_lhs, const type::variable& a_rhs)
{
    return a_lhs.m_index == a_rhs.m_index;
}

bool operator==(const type::constant& a_lhs, const type::constant& a_rhs)
{
    return a_lhs.m_name == a_rhs.m_name;
}

bool operator==(const type& a_lhs, const type& a_rhs)
{
    return a_lhs.m_root == a_rhs.m_root && a_lhs.m_deps == a_rhs.m_deps;
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_type_constant_construction()
{
    // int
    {
        type::constant l_constant("int");
        assert(l_constant.m_name == "int");
    }

    // string
    {
        type::constant l_constant("string");
        assert(l_constant.m_name == "string");
    }
}

void test_type_variable_construction()
{
    type::variable l_variable(0);
    assert(l_variable.m_index == 0);
}

void test_type_construction()
{
    type l_type(type::constant("int"), {});
    assert(std::get<type::constant>(l_type.m_root).m_name == "int");
    assert(l_type.m_deps.empty());

    type l_type2(type::variable(0), {});
    assert(std::get<type::variable>(l_type2.m_root).m_index == 0);
    assert(l_type2.m_deps.empty());

    type l_type3(type::constant("vector"), {type(type::variable(0), {})});
    assert(std::get<type::constant>(l_type3.m_root).m_name == "vector");
    assert(l_type3.m_deps.size() == 1);
    assert(std::get<type::variable>(l_type3.m_deps[0].m_root).m_index == 0);
}

void test_type_constant_comparison()
{
    // int < string
    {
        type::constant l_constant("int");
        type::constant l_constant2("string");
        assert(l_constant < l_constant2);
        assert(!(l_constant2 < l_constant));
    }
}

void test_type_variable_comparison()
{
    // int < string
    {
        type::variable l_variable(0);
        type::variable l_variable2(1);
        assert(l_variable < l_variable2);
        assert(!(l_variable2 < l_variable));
    }
}

void test_type_comparison()
{
    // int < string
    {
        type l_type(type::constant("int"), {});
        type l_type2(type::constant("string"), {});
        assert(l_type < l_type2);
        assert(!(l_type2 < l_type));
    }

    // vector<int> < vector<string>
    {
        type l_type(type::constant("vector"),
                    {type(type::constant("int"), {})});
        type l_type2(type::constant("vector"),
                     {type(type::constant("string"), {})});
        assert(l_type < l_type2);
        assert(!(l_type2 < l_type));
    }

    // map<int, string> < map<string, int>
    {
        type l_type(type::constant("map"),
                    {type(type::constant("int"), {}),
                     type(type::constant("string"), {})});
        type l_type2(type::constant("map"), {type(type::constant("string"), {}),
                                             type(type::constant("int"), {})});
        assert(l_type < l_type2);
        assert(!(l_type2 < l_type));
    }
}

void test_type_constant_equality()
{
    // int == int
    {
        type::constant l_constant("int");
        type::constant l_constant2("int");
        assert(l_constant == l_constant2);
        assert(!(l_constant == type::constant("string")));
    }
}

void test_type_variable_equality()
{
    // int == int
    {
        type::variable l_variable(0);
        type::variable l_variable2(0);
        assert(l_variable == l_variable2);
        assert(!(l_variable == type::variable(1)));
    }
}

void test_type_equality()
{
    // int == int
    {
        type l_type(type::constant("int"), {});
        type l_type2(type::constant("int"), {});
        assert(l_type == l_type2);
        assert(!(l_type == type(type::constant("string"), {})));
    }

    // vector<int> == vector<int>
    {
        type l_type(type::constant("vector"),
                    {type(type::constant("int"), {})});
        type l_type2(type::constant("vector"),
                     {type(type::constant("int"), {})});
        assert(l_type == l_type2);
        assert(!(l_type == type(type::constant("vector"),
                                {type(type::constant("string"), {})})));
    }
}

void type_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_type_constant_construction);
    TEST(test_type_variable_construction);
    TEST(test_type_constant_comparison);
    TEST(test_type_variable_comparison);
    TEST(test_type_comparison);
    TEST(test_type_constant_equality);
    TEST(test_type_variable_equality);
    TEST(test_type_equality);
}

#endif
