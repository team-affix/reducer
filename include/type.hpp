#ifndef TYPE_HPP
#define TYPE_HPP

#include <functional>
#include <string>
#include <variant>
#include <vector>

// represents a dependent type (root could be a parameterized type, and children
// are the parameters, e.g. List<int>)
struct type
{
    // represents a constant in a given type (like List in List<T>)
    struct constant
    {
        // the name of the constant
        std::string m_name;

        // the dependencies of the type
        std::vector<type> m_deps;

        // only constructor
        constant(const std::string& a_name, const std::vector<type>& a_deps);
    };

    // represents a variable in a given type (like T in List<T>)
    struct variable
    {
        // the index of the variable in the type
        size_t m_index;

        // only constructor
        variable(size_t a_index);
    };

    // the root of the type
    std::variant<constant, variable> m_data;

    // a function which validates the type
    std::function<bool(const type*)> m_validate;

    // only constructor
    type(
        const std::variant<constant, variable>& a_data,
        const std::function<bool(const type*)>& a_validate = [](const type*)
        { return true; });

    // unification of variable with something else
    bool unify(const variable& a_var, const type& a_other);
};

// type func_type(const type& a_return_type,
//                const std::vector<type>& a_param_types);

// type get_return_type(const type& a_type);

// std::vector<type> get_param_types(const type& a_type);

// bool is_func_type(const type& a_type);

// comparison operators
bool operator<(const type::variable& a_lhs, const type::variable& a_rhs);
bool operator<(const type::constant& a_lhs, const type::constant& a_rhs);
bool operator<(const type& a_lhs, const type& a_rhs);

// equality operators
bool operator==(const type::variable& a_lhs, const type::variable& a_rhs);
bool operator==(const type::constant& a_lhs, const type::constant& a_rhs);
bool operator==(const type& a_lhs, const type& a_rhs);

#endif
