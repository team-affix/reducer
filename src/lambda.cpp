#include "../include/lambda.hpp"

namespace lambda
{

// LIFT METHODS

std::unique_ptr<expr> local::lift(size_t a_new_depth) const
{
    return std::make_unique<local>(m_index + a_new_depth);
}

std::unique_ptr<expr> global::lift(size_t a_new_depth) const
{
    return std::make_unique<global>(m_index);
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

std::unique_ptr<expr> hole::lift(size_t a_new_depth) const
{
    return std::make_unique<hole>(m_captures);
}

// SUBSTITUTE METHODS

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

std::unique_ptr<expr> hole::substitute(size_t a_new_depth,
                                       const std::unique_ptr<expr>& a_arg) const
{
    // throw an error if we are beta-reducing a hole
    throw std::runtime_error("Error: cannot substitute into a hole.");
}

// REDUCE METHODS

std::unique_ptr<expr> local::reduce(const global_map& a_globals) const
{
    return clone();
}

std::unique_ptr<expr> global::reduce(const global_map& a_globals) const
{
    // look up the definition of the global variable
    const auto& l_definition = a_globals[m_index];

    // reduce the result
    return l_definition->reduce(a_globals);
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

std::unique_ptr<expr> hole::reduce(const global_map& a_globals) const
{
    throw std::runtime_error("Error: cannot reduce a hole.");
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

local::local(size_t a_index) : expr(), m_index(a_index)
{
}

global::global(size_t a_index) : expr(), m_index(a_index)
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

hole::hole(const std::set<size_t>& a_captures) : expr(), m_captures(a_captures)
{
}

} // namespace lambda

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_local_constructor()
{
    // index 0
    {
        lambda::local l_local{0};
        assert(l_local.m_index == 0);
    }

    // index 1
    {
        lambda::local l_local{1};
        assert(l_local.m_index == 1);
    }
}

void test_global_constructor()
{
    // index 0
    {
        lambda::global l_global{0};
        assert(l_global.m_index == 0);
    }

    // index 1
    {
        lambda::global l_global{1};
        assert(l_global.m_index == 1);
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

void test_local_lift()
{
    // index 0, lift 1 level
    {
        lambda::local l_local{0};
        auto l_lifted = l_local.lift(1);
        const lambda::local* l_lifted_local =
            dynamic_cast<lambda::local*>(l_lifted.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 1);
    }

    // index 1, lift 1 level
    {
        lambda::local l_local{1};
        auto l_lifted = l_local.lift(1);
        const lambda::local* l_lifted_local =
            dynamic_cast<lambda::local*>(l_lifted.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 2);
    }

    // index 1, lift 0 levels
    {
        lambda::local l_local{1};
        auto l_lifted = l_local.lift(0);
        const lambda::local* l_lifted_local =
            dynamic_cast<lambda::local*>(l_lifted.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 1);
    }
}

void test_global_lift()
{
    // index 0, lift 1 level
    {
        lambda::global l_global{0};
        auto l_lifted = l_global.lift(1);
        const lambda::global* l_lifted_global =
            dynamic_cast<lambda::global*>(l_lifted.get());
        assert(l_lifted_global != nullptr);

        // globals are left unchanged by lifting
        assert(l_lifted_global->m_index == 0);
    }

    // index 3, lift 2 levels
    {
        lambda::global l_global{3};
        auto l_lifted = l_global.lift(2);
        const lambda::global* l_lifted_global =
            dynamic_cast<lambda::global*>(l_lifted.get());
        assert(l_lifted_global != nullptr);

        // globals are left unchanged by lifting
        assert(l_lifted_global->m_index == 3);
    }
}

void test_func_lift()
{
    // local body, lift 1 level
    {
        lambda::local l_local{0};
        lambda::func l_func{l_local.clone()};
        auto l_lifted = l_func.lift(1);
        const lambda::func* l_lifted_func =
            dynamic_cast<lambda::func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const lambda::local* l_lifted_local =
            dynamic_cast<lambda::local*>(l_lifted_func->m_body.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 1);
    }

    // local body, lift 2 levels
    {
        lambda::local l_local{1};
        lambda::func l_func{l_local.clone()};
        auto l_lifted = l_func.lift(2);
        const lambda::func* l_lifted_func =
            dynamic_cast<lambda::func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const lambda::local* l_lifted_local =
            dynamic_cast<lambda::local*>(l_lifted_func->m_body.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 3);
    }
}

void test_app_lift()
{
    // local lhs, local rhs, lift 1 level
    {
        lambda::local l_lhs_local{1};
        lambda::local l_rhs_local{2};
        lambda::app l_app{l_lhs_local.clone(), l_rhs_local.clone()};
        auto l_lifted = l_app.lift(1);

        // get the lifted app (still an app)
        const lambda::app* l_lifted_app =
            dynamic_cast<lambda::app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_func;
        const lambda::local* l_lifted_lhs_local =
            dynamic_cast<lambda::local*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 2);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_arg;
        const lambda::local* l_lifted_rhs_local =
            dynamic_cast<lambda::local*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 3);
    }

    // local lhs, local rhs, lift 2 levels
    {
        lambda::local l_lhs_local{1};
        lambda::local l_rhs_local{2};
        lambda::app l_app{l_lhs_local.clone(), l_rhs_local.clone()};
        auto l_lifted = l_app.lift(2);

        // get the lifted app (still an app)
        const lambda::app* l_lifted_app =
            dynamic_cast<lambda::app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_func;
        const lambda::local* l_lifted_lhs_local =
            dynamic_cast<lambda::local*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 3);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_arg;
        const lambda::local* l_lifted_rhs_local =
            dynamic_cast<lambda::local*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 4);
    }
}

void test_hole_lift()
{
    // empty captures
    {
        lambda::hole l_hole{{}};
        auto l_lifted = l_hole.lift(1);

        const lambda::hole* l_lifted_hole =
            dynamic_cast<lambda::hole*>(l_lifted.get());

        assert(l_lifted_hole != nullptr);
        assert(l_lifted_hole->m_captures.empty());
    }

    // non-empty captures
    {
        std::set<size_t> l_captures{1, 2, 3};
        lambda::hole l_hole{l_captures};
        auto l_lifted = l_hole.lift(1);

        const lambda::hole* l_lifted_hole =
            dynamic_cast<lambda::hole*>(l_lifted.get());

        assert(l_lifted_hole != nullptr);
        assert(l_lifted_hole->m_captures == l_captures);
    }
}

void test_local_substitute()
{
    // index 0, occurrance depth 0, substitute with a local
    {
        lambda::local l_local{0};
        lambda::local l_substitute{1};
        auto l_substituted = l_local.substitute(0, l_substitute.clone());

        const lambda::local* l_substituted_local =
            dynamic_cast<lambda::local*>(l_substituted.get());
        assert(l_substituted_local != nullptr);
        assert(l_substituted_local->m_index == 1);
    }
    // index 0, occurrance depth 10, substitute with a local
    {
        lambda::local l_local{0};
        lambda::local l_substitute{1};
        auto l_substituted = l_local.substitute(10, l_substitute.clone());

        const lambda::local* l_substituted_local =
            dynamic_cast<lambda::local*>(l_substituted.get());
        assert(l_substituted_local != nullptr);
        // it would be 1 if occurrance depth == 0, but since 10,
        //     l_substitute had to be lifted.
        assert(l_substituted_local->m_index == 11);
    }

    // index 2, occurrance depth 0, substitute with a local
    {
        lambda::local l_local{2};
        lambda::local l_substitute{3};
        auto l_substituted = l_local.substitute(0, l_substitute.clone());

        const lambda::local* l_substituted_local =
            dynamic_cast<lambda::local*>(l_substituted.get());
        assert(l_substituted_local != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_local->m_index == 1);
    }

    // index 1, occurrance depth 0, substitute with a local
    {
        lambda::local l_local{1};
        lambda::local l_substitute{3};
        auto l_substituted = l_local.substitute(0, l_substitute.clone());

        const lambda::local* l_substituted_local =
            dynamic_cast<lambda::local*>(l_substituted.get());
        assert(l_substituted_local != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_local->m_index == 0);
    }

    // index 2, occurrance depth 10, substitute with a local
    {
        lambda::local l_local{2};
        lambda::local l_substitute{3};
        auto l_substituted = l_local.substitute(10, l_substitute.clone());

        const lambda::local* l_substituted_local =
            dynamic_cast<lambda::local*>(l_substituted.get());
        assert(l_substituted_local != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_local->m_index == 1);
    }

    // index 1, occurrance depth 10, substitute with a local
    {
        lambda::local l_local{1};
        lambda::local l_substitute{3};
        auto l_substituted = l_local.substitute(10, l_substitute.clone());

        const lambda::local* l_substituted_local =
            dynamic_cast<lambda::local*>(l_substituted.get());
        assert(l_substituted_local != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_local->m_index == 0);
    }
}

void test_global_substitute()
{
    // index 0, depth 0, substitute with a local
    {
        lambda::global l_global{0};
        lambda::local l_local{1};
        const auto l_substituted = l_global.substitute(0, l_local.clone());

        // should be global still
        const lambda::global* l_subbed_global =
            dynamic_cast<lambda::global*>(l_substituted.get());
        assert(l_subbed_global != nullptr);
        // globals are unaffected by substitution.
        assert(l_subbed_global->m_index == 0);
    }

    // index 0, depth 10, substitute with a local
    {
        lambda::global l_global{0};
        lambda::local l_local{1};
        const auto l_substituted = l_global.substitute(10, l_local.clone());

        // should be global still
        const lambda::global* l_subbed_global =
            dynamic_cast<lambda::global*>(l_substituted.get());
        assert(l_subbed_global != nullptr);
        // globals are unaffected by substitution.
        assert(l_subbed_global->m_index == 0);
    }

    // index 10, depth 10, substitute with a local
    {
        lambda::global l_global{10};
        lambda::local l_local{1};
        const auto l_substituted = l_global.substitute(10, l_local.clone());

        // should be global still
        const lambda::global* l_subbed_global =
            dynamic_cast<lambda::global*>(l_substituted.get());
        assert(l_subbed_global != nullptr);
        // globals are unaffected by substitution.
        assert(l_subbed_global->m_index == 10);
    }
}

void test_func_substitute()
{
    // single composition lambda, outer depth 0, occurrance found
    {
        // it should be noted:
        // when saying here that l_func has a body local of index 0,
        // that the local does NOT reference the binder introduced by
        // l_func. This is because, when subbing, it is implied that
        // there USED to be an even more outer binder (which 0 would
        // have been bound to, hence the substitution DOES take place)

        lambda::func l_func{lambda::local{0}.clone()};
        lambda::local l_local{11};

        const auto l_subbed = l_func.substitute(0, l_local.clone());

        const lambda::func* l_subbed_func =
            dynamic_cast<lambda::func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const lambda::local* l_subbed_local =
            dynamic_cast<lambda::local*>(l_subbed_func->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_local != nullptr);

        // the index of the substitute is lifted by 1 (one binder)
        assert(l_subbed_local->m_index == 12);
    }
    // doublle composition lambda, outer depth 0, occurrance found
    {
        // it should be noted:
        // when saying here that l_func has a body local of index 0,
        // that the local does NOT reference the binder introduced by
        // l_func. This is because, when subbing, it is implied that
        // there USED to be an even more outer binder (which 0 would
        // have been bound to, hence the substitution DOES take place)

        lambda::func l_func{lambda::func{lambda::local{0}.clone()}.clone()};
        lambda::local l_local{11};

        const auto l_subbed = l_func.substitute(0, l_local.clone());

        const lambda::func* l_subbed_func =
            dynamic_cast<lambda::func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        const lambda::func* l_subbed_func_2 =
            dynamic_cast<lambda::func*>(l_subbed_func->m_body.get());

        // get body
        const lambda::local* l_subbed_local =
            dynamic_cast<lambda::local*>(l_subbed_func_2->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_local != nullptr);

        // the index of the substitute is lifted by 2 (two binders)
        assert(l_subbed_local->m_index == 13);
    }

    // single composition lambda, outer depth 0, occurrance not found
    {
        lambda::func l_func{lambda::local{1}.clone()};
        lambda::global l_global{11};

        const auto l_subbed = l_func.substitute(0, l_global.clone());

        const lambda::func* l_subbed_func =
            dynamic_cast<lambda::func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const lambda::local* l_subbed_local =
            dynamic_cast<lambda::local*>(l_subbed_func->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_local != nullptr);

        // the local got decremented since it was not the thing to replace.
        assert(l_subbed_local->m_index == 0);
    }
}

void test_app_substitute()
{
    // app of locals, both are occurrances
    {
        lambda::local l_lhs{0};
        lambda::local l_rhs{0};
        lambda::app l_app{l_lhs.clone(), l_rhs.clone()};
        lambda::local l_sub{11};
        const auto l_subbed = l_app.substitute(0, l_sub.clone());

        // get the outer app
        const lambda::app* l_subbed_app =
            dynamic_cast<lambda::app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const lambda::local* l_subbed_lhs =
            dynamic_cast<lambda::local*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const lambda::local* l_subbed_rhs =
            dynamic_cast<lambda::local*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 11);
        assert(l_subbed_rhs->m_index == 11);
    }

    // app of locals, lhs is an occurrance
    {
        lambda::local l_lhs{0};
        lambda::local l_rhs{1};
        lambda::app l_app{l_lhs.clone(), l_rhs.clone()};
        lambda::local l_sub{11};
        const auto l_subbed = l_app.substitute(0, l_sub.clone());

        // get the outer app
        const lambda::app* l_subbed_app =
            dynamic_cast<lambda::app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const lambda::local* l_subbed_lhs =
            dynamic_cast<lambda::local*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const lambda::local* l_subbed_rhs =
            dynamic_cast<lambda::local*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 11);
        assert(l_subbed_rhs->m_index == 0);
    }

    // app of locals, rhs is an occurrance
    {
        lambda::local l_lhs{1};
        lambda::local l_rhs{0};
        lambda::app l_app{l_lhs.clone(), l_rhs.clone()};
        lambda::local l_sub{11};
        const auto l_subbed = l_app.substitute(0, l_sub.clone());

        // get the outer app
        const lambda::app* l_subbed_app =
            dynamic_cast<lambda::app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const lambda::local* l_subbed_lhs =
            dynamic_cast<lambda::local*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const lambda::local* l_subbed_rhs =
            dynamic_cast<lambda::local*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 0);
        assert(l_subbed_rhs->m_index == 11);
    }

    // app of locals, neither are occurrances
    {
        lambda::local l_lhs{1};
        lambda::local l_rhs{1};
        lambda::app l_app{l_lhs.clone(), l_rhs.clone()};
        lambda::local l_sub{11};
        const auto l_subbed = l_app.substitute(0, l_sub.clone());

        // get the outer app
        const lambda::app* l_subbed_app =
            dynamic_cast<lambda::app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const lambda::local* l_subbed_lhs =
            dynamic_cast<lambda::local*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const lambda::local* l_subbed_rhs =
            dynamic_cast<lambda::local*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 0);
        assert(l_subbed_rhs->m_index == 0);
    }

    // app of funcs, both with occurrances
    {
        lambda::func l_lhs{lambda::local{0}.clone()};
        lambda::func l_rhs{lambda::local{0}.clone()};
        lambda::app l_app{l_lhs.clone(), l_rhs.clone()};
        lambda::local l_sub{11};
        const auto l_subbed = l_app.substitute(0, l_sub.clone());

        // get the outer app
        const lambda::app* l_subbed_app =
            dynamic_cast<lambda::app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const lambda::func* l_subbed_lhs =
            dynamic_cast<lambda::func*>(l_subbed_app->m_func.get());

        // make sure lhs is a func
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const lambda::func* l_subbed_rhs =
            dynamic_cast<lambda::func*>(l_subbed_app->m_arg.get());

        // make sure rhs is a func
        assert(l_subbed_rhs != nullptr);

        const lambda::local* l_lhs_local =
            dynamic_cast<lambda::local*>(l_subbed_lhs->m_body.get());

        // make sure body of lhs is a local
        assert(l_lhs_local != nullptr);

        const lambda::local* l_rhs_local =
            dynamic_cast<lambda::local*>(l_subbed_rhs->m_body.get());

        // make sure body of rhs is a local
        assert(l_rhs_local != nullptr);

        // make sure they have correct indices (lifted by 1 due to binders)
        assert(l_lhs_local->m_index == 12);
        assert(l_rhs_local->m_index == 12);
    }
}

void test_hole_substitute()
{
    // empty captures
    {
        lambda::hole l_hole{{}};
        lambda::hole l_substitute{std::set<size_t>{}};
        assert_throws(l_hole.substitute(0, l_substitute.clone()),
                      std::runtime_error);
    }
}

void test_local_reduce()
{
    // local with var 0
    {
        lambda::local l_expr{0};
        const auto l_reduced = l_expr.reduce({});

        // cast the pointer
        const lambda::local* l_local =
            dynamic_cast<lambda::local*>(l_reduced.get());
        assert(l_local != nullptr);

        // make sure it has the same index
        assert(l_local->m_index == 0);
    }

    // local with var 1
    {
        lambda::local l_expr{1};
        const auto l_reduced = l_expr.reduce({});

        // cast the pointer
        const lambda::local* l_local =
            dynamic_cast<lambda::local*>(l_reduced.get());
        assert(l_local != nullptr);

        // make sure it has the same index
        assert(l_local->m_index == 1);
    }
}

void test_global_reduce()
{
    // global with index 0
    {
        // create global definitions
        lambda::expr::global_map l_globals{};
        l_globals.emplace_back(lambda::func{lambda::local{0}.clone()}.clone());
        l_globals.emplace_back(lambda::func{lambda::local{13}.clone()}.clone());

        // set up reduction
        lambda::global l_expr{0};
        const auto l_reduced = l_expr.reduce(l_globals);

        // cast the pointer
        const lambda::func* l_casted =
            dynamic_cast<lambda::func*>(l_reduced.get());
        assert(l_casted != nullptr);

        // get body
        const lambda::local* l_local =
            dynamic_cast<lambda::local*>(l_casted->m_body.get());
        assert(l_local != nullptr);

        // make sure it has the same index
        assert(l_local->m_index == 0);
    }

    // global with index 1
    {
        // create global definitions
        lambda::expr::global_map l_globals{};
        l_globals.emplace_back(lambda::func{lambda::local{0}.clone()}.clone());
        l_globals.emplace_back(lambda::func{lambda::local{13}.clone()}.clone());

        // set up reduction
        lambda::global l_expr{1};
        const auto l_reduced = l_expr.reduce(l_globals);

        // cast the pointer
        const lambda::func* l_casted =
            dynamic_cast<lambda::func*>(l_reduced.get());
        assert(l_casted != nullptr);

        // get body
        const lambda::local* l_local =
            dynamic_cast<lambda::local*>(l_casted->m_body.get());
        assert(l_local != nullptr);

        // make sure it has the same index
        assert(l_local->m_index == 13);
    }
}

void lambda_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_local_constructor);
    TEST(test_global_constructor);
    TEST(test_func_constructor);
    TEST(test_app_constructor);
    TEST(test_hole_constructor);
    TEST(test_local_lift);
    TEST(test_global_lift);
    TEST(test_func_lift);
    TEST(test_app_lift);
    TEST(test_hole_lift);
    TEST(test_local_substitute);
    TEST(test_global_substitute);
    TEST(test_hole_substitute);
    TEST(test_func_substitute);
    TEST(test_app_substitute);
    TEST(test_local_reduce);
    TEST(test_global_reduce);
}

#endif
