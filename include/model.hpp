#ifndef MODEL_HPP
#define MODEL_HPP

#include "lambda.hpp"
#include <memory>
#include <optional>
#include <ostream>

struct model
{
    // describes the value of the bins
    bool m_homogenous_value;

    // the function to produce the bins
    const std::unique_ptr<lambda::expr> m_func;

    // the next functions to evaluate
    const std::unique_ptr<model> m_negative_child;
    const std::unique_ptr<model> m_positive_child;

    // the function to evaluate the model
    std::optional<bool> eval(const std::unique_ptr<lambda::expr>* a_params,
                             size_t a_param_count, size_t a_step_limit,
                             size_t a_size_limit);
};

std::ostream& operator<<(std::ostream& a_ostream, const model& a_model);

#endif // MODEL_HPP
