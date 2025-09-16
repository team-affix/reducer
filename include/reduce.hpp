#ifndef REDUCE_HPP
#define REDUCE_HPP

#include "func.hpp"
#include <variant>

////////////////////////////////////////////////////
/////////////////// CHOICE TYPES ///////////////////
////////////////////////////////////////////////////
struct place_param_node
{
    size_t m_index;
};
struct place_func_node
{
    const func* m_func;
};
struct terminate
{
};
struct make_function
{
};

using choice =
    std::variant<place_func_node, place_param_node, terminate, make_function>;

// less than comparisons
bool operator<(const place_func_node&, const place_func_node&);
bool operator<(const place_param_node&, const place_param_node&);
bool operator<(const terminate&, const terminate&);
bool operator<(const make_function&, const make_function&);

#endif
