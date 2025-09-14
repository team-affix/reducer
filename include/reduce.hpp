#ifndef REDUCE_HPP
#define REDUCE_HPP

#include "func.hpp"
#include <variant>

////////////////////////////////////////////////////
/////////////////// CHOICE TYPES ///////////////////
////////////////////////////////////////////////////
struct place_functor_node
{
    func* m_func;
};
struct place_param_node
{
    size_t m_index;
};
struct terminate
{
};
struct make_function
{
};

using choice = std::variant<place_functor_node, place_param_node, terminate,
                            make_function>;

// less than comparisons
bool operator<(const place_functor_node&, const place_functor_node&);
bool operator<(const place_param_node&, const place_param_node&);
bool operator<(const terminate&, const terminate&);
bool operator<(const make_function&, const make_function&);

#endif
