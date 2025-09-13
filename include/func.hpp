#ifndef FUNC_HPP
#define FUNC_HPP

#include <any>
#include <functional>
#include <list>
#include <string>
#include <typeindex>

// represents a parameter to a func
struct param
{
    size_t m_index;
};

// represents a function definition
struct func_body
{
    using functor =
        std::function<std::any(std::list<std::any>::const_iterator,
                               std::list<std::any>::const_iterator)>;

    std::variant<functor, param> m_body;

    std::list<func_body> m_children;
    std::any eval() const;
    size_t node_count() const;
};

// contains total information about a function
struct func
{
    // the parameters
    std::type_index m_return_type;
    std::list<std::type_index> m_param_types;
    std::list<std::any> m_params;
    func_body m_body;
    std::string m_repr;
    std::any eval(std::list<std::any>::const_iterator a_begin,
                  std::list<std::any>::const_iterator a_end);
    // normal constructor
    func(const std::type_index& a_return_type, const func_body& a_body,
         const std::string& a_repr);
    // prevent copying
    func(const func&) = delete;
    func& operator=(const func&) = delete;
};

#endif
