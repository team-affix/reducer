#ifndef MODEL_HPP
#define MODEL_HPP

#include <any>
#include <functional>
#include <memory>

struct model
{
    // describes the value of the bins
    bool m_homogenous_value;

    // the function to produce the bins
    std::string m_func_repr;
    std::function<bool(const std::any*)> m_func;

    // the next functions to evaluate
    std::shared_ptr<model> m_negative_child;
    std::shared_ptr<model> m_positive_child;

    // the function to evaluate the model
    bool eval(const std::any* a_params);

    // get the node count
    size_t node_count() const;

    // representation of the model
    std::string repr() const;
};

#endif // MODEL_HPP
