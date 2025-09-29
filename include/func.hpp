#ifndef FUNC_HPP
#define FUNC_HPP

#include "type.hpp"
#include <any>
#include <functional>
#include <string>
#include <variant>

// contains total information about a function
struct func
{
    // represents a parameter to a func
    struct param
    {
        size_t m_index;
    };

    // represents a primitive function
    struct primitive
    {
        std::function<std::any(const std::any*, size_t)> m_defn;
    };

    // represents a function definition
    struct body
    {
        // either a parameter, primitive, or pointer to a func
        std::variant<param, primitive, const func*> m_functor;

        // children
        std::vector<body> m_children;

        // evaluate the body
        std::any eval(const std::any* a_params, size_t a_param_count) const;

        // count the number of nodes in the body
        size_t node_count() const;
    };

    // the signature and body of the function
    type m_signature;
    body m_body;
    std::string m_repr;

    // normal constructor
    func(const type& a_signature, const body& a_body,
         const std::string& a_repr);

    // prevent copying
    func(const func&) = delete;
    func& operator=(const func&) = delete;
};

bool operator<(const func::param& a_lhs, const func::param& a_rhs);

#endif
