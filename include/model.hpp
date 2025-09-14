#ifndef MODEL_HPP
#define MODEL_HPP

#include "func.hpp"
#include <memory>

struct model
{
    // describes the value of the bins
    bool m_homogenous_value;

    // the function to produce the bins
    func* m_func;

    // the next functions to evaluate
    std::shared_ptr<model> m_negative_child;
    std::shared_ptr<model> m_positive_child;

    // the function to evaluate the model
    bool eval(const std::any* a_params, size_t a_param_count);

    // get the node count
    size_t node_count() const;

    // representation of the model
    std::string repr() const;
};

#endif // MODEL_HPP
