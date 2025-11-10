#include "../include/lambda.hpp"

namespace lambda
{

// LIFT METHODS
std::unique_ptr<expr> hole::lift(size_t a_new_depth) const
{
    throw std::runtime_error("Error: cannot lift a hole.");
}

std::unique_ptr<expr> func::lift(size_t a_new_depth) const
{
    return std::make_unique<func>(m_body->lift(a_new_depth));
}

std::unique_ptr<expr> app::lift(size_t a_new_depth) const
{
    return std::make_unique<app>(m_func->lift(a_new_depth),
                                 m_arg->lift(a_new_depth));
}

std::unique_ptr<expr> local::lift(size_t a_new_depth) const
{
    return std::make_unique<local>(m_index + a_new_depth);
}

std::unique_ptr<expr> global::lift(size_t a_new_depth) const
{
    return std::make_unique<global>(m_index);
}

// SUBSTITUTE METHODS
std::unique_ptr<expr> hole::substitute(size_t a_new_depth,
                                       const std::unique_ptr<expr>& a_arg) const
{
    // throw an error if we are beta-reducing a hole
    throw std::runtime_error("Error: cannot substitute into a hole.");
}

std::unique_ptr<expr> func::substitute(size_t a_new_depth,
                                       const std::unique_ptr<expr>& a_arg) const
{
    // increment the binder depth
    return std::make_unique<func>(m_body->substitute(a_new_depth + 1, a_arg));
}

std::unique_ptr<expr> app::substitute(size_t a_new_depth,
                                      const std::unique_ptr<expr>& a_arg) const
{
    // just substitute the function and argument
    return std::make_unique<app>(m_func->substitute(a_new_depth, a_arg),
                                 m_arg->substitute(a_new_depth, a_arg));
}

std::unique_ptr<expr>
local::substitute(size_t a_new_depth, const std::unique_ptr<expr>& a_arg) const
{
    if(m_index > 0)
        // this var is defined inside the redex, so it is
        //     now 1 level shallower.
        return std::make_unique<local>(m_index - 1);

    // this var is the one we are substituting, so we must substitute it
    return a_arg->lift(a_new_depth);
}

std::unique_ptr<expr>
global::substitute(size_t a_new_depth, const std::unique_ptr<expr>& a_arg) const
{
    // doing a local substitution on a global variable is a no-op
    return lift(0);
}

// REDUCE METHODS
std::unique_ptr<expr> hole::reduce() const
{
    throw std::runtime_error("Error: cannot reduce a hole.");
}

std::unique_ptr<expr> func::reduce() const
{
    return lift(0);
}

std::unique_ptr<expr> app::reduce() const
{
    // reduce the function to WHNF
    auto l_reduced_func = m_func->reduce();

    // check if the lhs is a beta-redex
    const func* l_beta_redex = dynamic_cast<func*>(l_reduced_func.get());

    if(!l_beta_redex || dynamic_cast<local*>(m_arg.get()))
        // leave the func in WHNF and the argument alone
        return std::make_unique<app>(l_reduced_func, m_arg);

    // beta-reduce the app
    std::unique_ptr<expr> l_substituted_body =
        l_beta_redex->m_body->substitute(0, m_arg);

    return l_substituted_body->reduce();
}

std::unique_ptr<expr> local::reduce() const
{
    return lift(0);
}

std::unique_ptr<expr> global::reduce() const
{
    // return the reduction of a delta reduction
}

} // namespace lambda
