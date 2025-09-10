#ifndef REDUCE_HPP
#define REDUCE_HPP

#include "func.hpp"
#include <variant>

////////////////////////////////////////////////////
/////////////////// CHOICE TYPES ///////////////////
////////////////////////////////////////////////////
struct place_node_t
{
    const func* m_node;
};
struct place_new_param_t
{
};
struct terminate_t
{
};
struct make_function_t
{
};

using choice_t =
    std::variant<place_node_t, place_new_param_t,
                 terminate_t, make_function_t>;

// less than comparisons
bool operator<(const place_node_t&, const place_node_t&);
bool operator<(const place_new_param_t&,
               const place_new_param_t&);
bool operator<(const terminate_t&, const terminate_t&);
bool operator<(const make_function_t&,
               const make_function_t&);

#endif
