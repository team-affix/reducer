#ifndef MODEL_HPP
#define MODEL_HPP

#include "../mcts/include/mcts.hpp"
#include "lambda.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <ostream>
#include <variant>
#include <vector>

struct model
{
    // the function to evaluate the model
    std::optional<bool>
    eval(const std::list<std::unique_ptr<lambda::expr>>& a_helpers,
         const std::unique_ptr<lambda::expr>* a_args, size_t a_arity,
         size_t a_step_limit, size_t a_size_limit);

  private:
    // constructor for homogenous models
    model(bool a_homogenous_value);

    // constructor for non-homogenous models
    model(std::unique_ptr<lambda::expr>&& a_func,
          std::unique_ptr<model>&& a_positive_child,
          std::unique_ptr<model>&& a_negative_child);

    // describes the value of the bins
    const bool m_homogenous_value;

    // the function to produce the bins
    const std::unique_ptr<lambda::expr> m_func;

    // the next functions to evaluate
    const std::unique_ptr<model> m_positive_child;
    const std::unique_ptr<model> m_negative_child;

    // friend operators
    friend std::unique_ptr<model> m(bool a_value);
    friend std::unique_ptr<model> m(std::unique_ptr<lambda::expr>&& a_func,
                                    std::unique_ptr<model>&& a_positive_child,
                                    std::unique_ptr<model>&& a_negative_child);
    friend std::ostream& operator<<(std::ostream& a_ostream,
                                    const model& a_model);
};

// factory functions for model
std::unique_ptr<model> m(bool a_value);
std::unique_ptr<model> m(std::unique_ptr<lambda::expr>&& a_func,
                         std::unique_ptr<model>&& a_positive_child,
                         std::unique_ptr<model>&& a_negative_child);

// ostream inserter for model
std::ostream& operator<<(std::ostream& a_ostream, const model& a_model);

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
    data_point(std::vector<std::unique_ptr<lambda::expr>>&& a_inputs,
               bool a_output);
    std::vector<std::unique_ptr<lambda::expr>> m_inputs;
    bool m_output;
};

// build a model from data
std::unique_ptr<model>
build_model(const std::vector<const data_point*>& a_data,
            const std::list<std::unique_ptr<lambda::expr>>& a_helpers,
            const size_t& a_step_limit, const size_t& a_size_limit,
            const size_t& a_arity,
            monte_carlo::simulation<choice, std::mt19937>& a_simulation,
            const size_t& a_recursion_limit);

#endif // MODEL_HPP
