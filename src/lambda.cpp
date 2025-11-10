#include "../include/lambda.hpp"

namespace lambda
{

// LIFT METHODS
std::unique_ptr<expr> hole::lift(size_t a_new_depth) const
{
    return std::make_unique<hole>(m_captures);
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
    return clone();
}

// REDUCE METHODS
std::unique_ptr<expr> hole::reduce(const global_map& a_globals) const
{
    throw std::runtime_error("Error: cannot reduce a hole.");
}

std::unique_ptr<expr> func::reduce(const global_map& a_globals) const
{
    return clone();
}

std::unique_ptr<expr> app::reduce(const global_map& a_globals) const
{
    // reduce the function to WHNF
    auto l_reduced_func = m_func->reduce(a_globals);

    // check if the lhs is a beta-redex
    const func* l_beta_redex = dynamic_cast<func*>(l_reduced_func.get());

    if(!l_beta_redex || dynamic_cast<local*>(m_arg.get()))
        // leave the func in WHNF and the argument alone
        return std::make_unique<app>(l_reduced_func, m_arg);

    // beta-reduce the app
    std::unique_ptr<expr> l_substituted_body =
        l_beta_redex->m_body->substitute(0, m_arg);

    // reduce the result
    return l_substituted_body->reduce(a_globals);
}

std::unique_ptr<expr> local::reduce(const global_map& a_globals) const
{
    return clone();
}

std::unique_ptr<expr> global::reduce(const global_map& a_globals) const
{
    // look up the definition of the global variable
    const auto l_definition_it = a_globals.find(m_index);

    // get the definition
    const auto l_definition = l_definition_it->second->clone();

    // reduce the result
    return l_definition->reduce(a_globals);
}

// EXPR CLONE METHOD
std::unique_ptr<expr> expr::clone() const
{
    return lift(0);
}

// CONSTRUCTORS
expr::expr()
{
}

hole::hole(const std::set<size_t>& a_captures) : expr(), m_captures(a_captures)
{
}

func::func(const std::unique_ptr<expr>& a_body)
    : expr(), m_body(a_body->clone())
{
}

app::app(const std::unique_ptr<expr>& a_func,
         const std::unique_ptr<expr>& a_arg)
    : expr(), m_func(a_func->clone()), m_arg(a_arg->clone())
{
}

local::local(size_t a_index) : expr(), m_index(a_index)
{
}

global::global(size_t a_index) : expr(), m_index(a_index)
{
}

} // namespace lambda

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_hole_constructor()
{
    // empty captures
    {
        lambda::hole l_hole{{}};
        assert(l_hole.m_captures.empty());
    }

    // non-empty captures
    {
        std::set<size_t> l_captures{1, 2, 3};
        lambda::hole l_hole{l_captures};
        assert(l_hole.m_captures == l_captures);
    }
}

void test_func_constructor()
{
    // hole body
    {
        std::set<size_t> l_captures{1, 2, 3};
        lambda::func l_func{std::make_unique<lambda::hole>(l_captures)};
        // get body
        const auto& l_body = l_func.m_body;
        // check if the body is a hole
        const lambda::hole* l_hole = dynamic_cast<lambda::hole*>(l_body.get());
        assert(l_hole != nullptr);
        // check if the captures are correct
        assert(l_hole->m_captures == l_captures);
    }
}

void test_app_constructor()
{
    // hole application
    {
        std::set<size_t> l_lhs_captures{1, 2, 3};
        std::set<size_t> l_rhs_captures{4, 5, 6};
        lambda::app l_app{std::make_unique<lambda::hole>(l_lhs_captures),
                          std::make_unique<lambda::hole>(l_rhs_captures)};
        // get the lhs
        const auto& l_lhs = l_app.m_func;
        // get the rhs
        const auto& l_rhs = l_app.m_arg;

        // make sure they both are holes
        const lambda::hole* l_lhs_hole =
            dynamic_cast<lambda::hole*>(l_lhs.get());
        const lambda::hole* l_rhs_hole =
            dynamic_cast<lambda::hole*>(l_rhs.get());
        assert(l_lhs_hole != nullptr);
        assert(l_rhs_hole != nullptr);

        // make sure the captures are correct
        assert(l_lhs_hole->m_captures == l_lhs_captures);
        assert(l_rhs_hole->m_captures == l_rhs_captures);
    }
}

void lambda_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_hole_constructor);
    TEST(test_func_constructor);
    TEST(test_app_constructor);
}

#endif
