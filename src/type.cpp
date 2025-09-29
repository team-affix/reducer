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

type::type(const std::variant<variable, constant>& a_root,
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

void type_test_main()
{
}

#endif
