#ifndef REDUCE_HPP
#define REDUCE_HPP

#include "../mcts/include/mcts.hpp"
#include "model.hpp"
#include <cstddef>
#include <list>
#include <variant>
#include <vector>

////////////////////////////////////////////////////
/////////////////// CHOICE TYPES ///////////////////
////////////////////////////////////////////////////
struct place_var_node
{
    size_t m_index;
};
struct place_func_node
{
};
struct place_app_node
{
};
struct add_helper
{
};
struct terminate
{
};

using choice = std::variant<place_var_node, place_func_node, place_app_node,
                            add_helper, terminate>;

// less than comparisons
bool operator<(const place_var_node&, const place_var_node&);
bool operator<(const place_func_node&, const place_func_node&);
bool operator<(const place_app_node&, const place_app_node&);
bool operator<(const add_helper&, const add_helper&);
bool operator<(const terminate&, const terminate&);

// data_point type
struct data_point
{
    const std::vector<std::unique_ptr<lambda::expr>> m_inputs;
    const bool m_output;
};

// build a model from data
model build_model(const size_t a_binder_depth, const size_t a_arity,
                  const std::vector<const data_point*>& a_data,
                  monte_carlo::simulation<choice, std::mt19937>& a_simulation,
                  const size_t& a_recursion_limit);

#endif
