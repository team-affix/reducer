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
    std::unique_ptr<model> m_negative_child;
    std::unique_ptr<model> m_positive_child;

    // the function to evaluate the model
    bool eval(std::list<std::any>::const_iterator a_begin,
              std::list<std::any>::const_iterator a_end);
};

#endif // MODEL_HPP
