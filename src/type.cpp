#include "../include/type.hpp"
#include <variant>

type::constant::constant(const std::string& a_name,
                         const std::vector<type>& a_deps)
    : m_name(a_name), m_deps(a_deps)
{
}

type::variable::variable(size_t a_index) : m_index(a_index)
{
}

type::type(const std::variant<constant, variable>& a_data,
           const std::function<bool(const type*)>& a_validate)
    : m_data(a_data), m_validate(a_validate)
{
}

bool type::unify(const variable& a_var, const type& a_other)
{
    // get the original validator
    auto l_validate = m_validate;

    if(const auto l_var = std::get_if<variable>(&m_data))
    {
        // if the variable is the same, replace this node
        if(*l_var == a_var)
            *this = a_other;
    }
    else
    {
        auto& l_constant = std::get<constant>(m_data);

        // the new dependencies
        std::vector<type> l_new_deps;

        // unify the dependencies
        for(auto& l_dep : l_constant.m_deps)
        {
            if(!l_dep.unify(a_var, a_other))
                return false;
        }

        // replace this node with a new one
        *this = type(constant(l_constant.m_name, l_new_deps), l_validate);
    }

    // if validation fails, unification fails too
    if(!l_validate(this))
        return false;

    return true;
}

bool operator<(const type::variable& a_lhs, const type::variable& a_rhs)
{
    return a_lhs.m_index < a_rhs.m_index;
}

bool operator<(const type::constant& a_lhs, const type::constant& a_rhs)
{
    if(a_lhs.m_name < a_rhs.m_name)
        return true;
    if(a_rhs.m_name < a_lhs.m_name)
        return false;

    return a_lhs.m_deps < a_rhs.m_deps;
}

bool operator<(const type& a_lhs, const type& a_rhs)
{
    return a_lhs.m_data < a_rhs.m_data;
}

bool operator==(const type::variable& a_lhs, const type::variable& a_rhs)
{
    return a_lhs.m_index == a_rhs.m_index;
}

bool operator==(const type::constant& a_lhs, const type::constant& a_rhs)
{
    return a_lhs.m_name == a_rhs.m_name && a_lhs.m_deps == a_rhs.m_deps;
}

bool operator==(const type& a_lhs, const type& a_rhs)
{
    return a_lhs.m_data == a_rhs.m_data;
}

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_type_constant_construction()
{
    // int
    {
        type::constant l_constant("int", {});
        assert(l_constant.m_name == "int");
    }

    // string
    {
        type::constant l_constant("string", {});
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
    type l_type(type::constant("int", {}), {});
    assert(std::get<type::constant>(l_type.m_data).m_name == "int");
    assert(std::get<type::constant>(l_type.m_data).m_deps.empty());

    type l_type2(type::variable(0), {});
    assert(std::get<type::variable>(l_type2.m_data).m_index == 0);

    type l_type3(type::constant("vector", {type(type::variable(0), {})}), {});
    assert(std::get<type::constant>(l_type3.m_data).m_name == "vector");
    assert(std::get<type::constant>(l_type3.m_data).m_deps.size() == 1);
    assert(std::get<type::variable>(
               std::get<type::constant>(l_type3.m_data).m_deps[0].m_data)
               .m_index == 0);
}

void test_type_constant_comparison()
{
    // int < string
    {
        type::constant l_constant("int", {});
        type::constant l_constant2("string", {});
        assert(l_constant < l_constant2);
        assert(!(l_constant2 < l_constant));
    }

    // ensuring deps get compared
    {
        type l_type(type::constant("int", {}));
        type l_type2(
            type::constant("int", {type(type::constant("string", {}))}));
        assert(l_type < l_type2);
        assert(!(l_type2 < l_type));
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
        type l_type(type::constant("int", {}), {});
        type l_type2(type::constant("string", {}), {});
        assert(l_type < l_type2);
        assert(!(l_type2 < l_type));
    }

    // vector<int> < vector<string>
    {
        type l_type(
            type::constant("vector", {type(type::constant("int", {}), {})}),
            {});
        type l_type2(
            type::constant("vector", {type(type::constant("string", {}), {})}),
            {});
        assert(l_type < l_type2);
        assert(!(l_type2 < l_type));
    }

    // map<int, string> < map<string, int>
    {
        type l_type(
            type::constant("map", {type(type::constant("int", {}), {}),
                                   type(type::constant("string", {}), {})}),
            {});
        type l_type2(
            type::constant("map", {type(type::constant("string", {}), {}),
                                   type(type::constant("int", {}), {})}),
            {});
        assert(l_type < l_type2);
        assert(!(l_type2 < l_type));
    }
}

void test_type_constant_equality()
{
    // int == int
    {
        type::constant l_constant("int", {});
        type::constant l_constant2("int", {});
        assert(l_constant == l_constant2);
        assert(!(l_constant == type::constant("string", {})));
    }

    // ensuring deps get compared
    {
        type l_type(type::constant("int", {}));
        type l_type2(type::constant("int", {}));
        assert(l_type == l_type2);
        assert(!(l_type == type(type::constant("string", {}))));
        assert(!(l_type == type(type::constant(
                               "int", {type(type::constant("string", {}))}))));
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
        type l_type(type::constant("int", {}), {});
        type l_type2(type::constant("int", {}), {});
        assert(l_type == l_type2);
        assert(!(l_type == type(type::constant("string", {}), {})));
    }

    // vector<int> == vector<int>
    {
        type l_type(
            type::constant("vector", {type(type::constant("int", {}), {})}),
            {});
        type l_type2(
            type::constant("vector", {type(type::constant("int", {}), {})}),
            {});
        assert(l_type == l_type2);
        assert(!(l_type ==
                 type(type::constant("vector",
                                     {type(type::constant("string", {}), {})}),
                      {})));
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
