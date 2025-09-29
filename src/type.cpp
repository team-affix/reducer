#include "../include/type.hpp"

type func_type(const type& a_return_type,
               const std::vector<type>& a_param_types)
{
    // the first dependency is the return type
    // the second dependency is a tuple of the param types
    return type{.m_root = "func",
                .m_deps = {a_return_type, type{
                                              .m_root = "tuple",
                                              .m_deps = a_param_types,
                                          }}};
}

type get_return_type(const type& a_type)
{
    // the first dependency is the return type
    return a_type.m_deps[0];
}

std::vector<type> get_param_types(const type& a_type)
{
    // the second dependency is a tuple of the param types
    return a_type.m_deps[1].m_deps;
}

bool is_func_type(const type& a_type)
{
    return a_type.m_root == "func";
}

bool operator<(const type& a_lhs, const type& a_rhs)
{
    if(a_lhs.m_root < a_rhs.m_root)
        return true;
    if(a_lhs.m_root > a_rhs.m_root)
        return false;
    if(a_lhs.m_deps.size() < a_rhs.m_deps.size())
        return true;
    if(a_lhs.m_deps.size() > a_rhs.m_deps.size())
        return false;
    for(size_t i = 0; i < a_lhs.m_deps.size(); ++i)
    {
        if(a_lhs.m_deps[i] < a_rhs.m_deps[i])
            return true;
        if(a_rhs.m_deps[i] < a_lhs.m_deps[i])
            return false;
    }
    return false;
}

bool operator==(const type& a_lhs, const type& a_rhs)
{
    return !(a_lhs < a_rhs) && !(a_rhs < a_lhs);
}
