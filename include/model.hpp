#ifndef MODEL_HPP
#define MODEL_HPP

#include "lambda.hpp"
#include <memory>

struct model
{
    // describes the value of the bins
    bool m_homogenous_value;

    // the function to produce the bins
    const std::unique_ptr<lambda::expr> m_func;

    // the next functions to evaluate
    std::unique_ptr<model> m_negative_child;
    std::unique_ptr<model> m_positive_child;

    // the function to evaluate the model
    bool eval(const std::unique_ptr<lambda::expr>* a_params,
              size_t a_param_count);

    // get the node count
    size_t node_count() const;

    // representation of the model
    std::string repr() const;
};

#endif // MODEL_HPP
