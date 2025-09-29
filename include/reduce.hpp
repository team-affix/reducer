#ifndef REDUCE_HPP
#define REDUCE_HPP

#include <any>
#include <cstddef>
#include <functional>
#include <variant>

////////////////////////////////////////////////////
/////////////////// CHOICE TYPES ///////////////////
////////////////////////////////////////////////////
struct return_param
{
    size_t m_index;
};
struct create_and_return_param
{
};
struct compose_func
{
    const std::function<std::any(const std::any*)> m_func;
};
struct return_func
{
    const std::function<std::any(const std::any*)> m_func;
};
struct create_and_return_func
{
};
struct terminate
{
};
struct make_function
{
};

using choice =
    std::variant<return_param, create_and_return_param, compose_func,
                 return_func, create_and_return_func, terminate, make_function>;

// less than comparisons
bool operator<(const return_param&, const return_param&);
bool operator<(const create_and_return_param&, const create_and_return_param&);
bool operator<(const compose_func&, const compose_func&);
bool operator<(const return_func&, const return_func&);
bool operator<(const create_and_return_func&, const create_and_return_func&);
bool operator<(const terminate&, const terminate&);
bool operator<(const make_function&, const make_function&);

#endif
