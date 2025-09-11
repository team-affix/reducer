#ifndef REDUCE_HPP
#define REDUCE_HPP

#include "func.hpp"
#include <variant>

////////////////////////////////////////////////////
/////////////////// CHOICE TYPES ///////////////////
////////////////////////////////////////////////////
struct place_node
{
    std::list<func>::const_iterator m_func_it;
};
struct place_new_param
{
};
struct terminate
{
};
struct make_function
{
};

using choice =
    std::variant<place_node, place_new_param, terminate, make_function>;

// less than comparisons
bool operator<(const place_node&, const place_node&);
bool operator<(const place_new_param&, const place_new_param&);
bool operator<(const terminate&, const terminate&);
bool operator<(const make_function&, const make_function&);

#endif
