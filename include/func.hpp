#ifndef FUNC_HPP
#define FUNC_HPP

#include <any>
#include <functional>
#include <map>
#include <string>
#include <typeindex>
#include <variant>

// represents a parameter to a func
struct param
{
    size_t m_index;
};

// represents a function definition
struct func_body
{
    // functor type
    using functor = std::function<std::any(const std::any*, size_t)>;

    // either a functor or a parameter
    std::variant<functor, param> m_data;

    // children
    std::vector<func_body> m_children;

    // evaluate the body
    std::any eval(const std::any* a_params, size_t a_param_count) const;

    // count the number of nodes in the body
    size_t node_count() const;
};

// contains total information about a function
struct func
{
    // the parameters
    std::type_index m_return_type;
    std::multimap<std::type_index, size_t> m_param_types;
    func_body m_body;
    std::string m_repr;
    // normal constructor
    func(const std::type_index& a_return_type,
         const std::multimap<std::type_index, size_t>& a_param_types,
         const func_body& a_body, const std::string& a_repr);
    // prevent copying
    func(const func&) = delete;
    func& operator=(const func&) = delete;
};

#endif
