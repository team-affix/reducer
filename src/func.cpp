#include "../include/func.hpp"
#include <numeric>

// evaluation functions
std::any func_t::eval() const
{
    // construct the arguments for the functor
    std::list<std::any> l_functor_args;

    // evaluate all children
    std::transform(m_children.begin(), m_children.end(),
                   std::back_inserter(l_functor_args),
                   [](const func_t& a_child)
                   { return a_child.eval(); });

    // evaluate the functor
    return m_functor(l_functor_args);
}

std::any
func_t::operator()(const std::list<std::any>& a_args) const
{
    // set the parameters
    std::copy(a_args.begin(), a_args.end(), m_params);
    // evaluate the function
    return eval();
}

size_t func_t::node_count() const
{
    // count nodes in children, add one for this node
    return std::accumulate(
        m_children.begin(), m_children.end(), 1,
        [](size_t a_sum, const func_t& a_child)
        { return a_sum + a_child.node_count(); });
}

// std::ostream& operator<<(std::ostream& a_os,
//                          const func_t& a_func)
// {
//     a_os << a_func.m_base_repr << "(";

//     std::list<func_t>::const_iterator l_it =
//         a_func.m_children.begin();

//     if(l_it != a_func.m_children.end())
//     {
//         a_os << *l_it;
//         ++l_it;
//     }

//     for(; l_it != a_func.m_children.end(); ++l_it)
//     {
//         a_os << ", " << *l_it;
//     }

//     a_os << ")";

//     return a_os;
// }
