#ifndef TYPE_HPP
#define TYPE_HPP

#include <string>
#include <vector>

// represents a dependent type (root could be a parameterized type, and children
// are the parameters, e.g. List<int>)
struct type
{
    std::string m_root;
    std::vector<type> m_deps;
};

type func_type(const type& a_return_type,
               const std::vector<type>& a_param_types);

type get_return_type(const type& a_type);

std::vector<type> get_param_types(const type& a_type);

bool is_func_type(const type& a_type);

// comparison operators
bool operator<(const type& a_lhs, const type& a_rhs);
bool operator==(const type& a_lhs, const type& a_rhs);

#endif
