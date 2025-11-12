#include "../include/lambda.hpp"

#define VERBOSE_LOGS 1
#if VERBOSE_LOGS

#include <iostream>

#define LOG_EXPR(a_depth, a_expr)                                              \
    std::cout << "DEPTH: " << a_depth << " -> ";                               \
    a_expr->print(std::cout);                                                  \
    std::cout << std::endl;

#else

// no-op
#define LOG_EXPR(a_depth, a_expr)

#endif

namespace lambda
{

// EQUALS METHODS

bool local::equals(const std::unique_ptr<expr>& a_other) const
{
    const local* l_casted = dynamic_cast<const local*>(a_other.get());

    if(!l_casted)
        return false;

    return m_index == l_casted->m_index;
}

bool global::equals(const std::unique_ptr<expr>& a_other) const
{
    const global* l_casted = dynamic_cast<const global*>(a_other.get());

    if(!l_casted)
        return false;

    return m_index == l_casted->m_index;
}

bool func::equals(const std::unique_ptr<expr>& a_other) const
{
    const func* l_casted = dynamic_cast<const func*>(a_other.get());

    if(!l_casted)
        return false;

    return m_body->equals(l_casted->m_body);
}

bool app::equals(const std::unique_ptr<expr>& a_other) const
{
    const app* l_casted = dynamic_cast<const app*>(a_other.get());

    if(!l_casted)
        return false;

    return m_func->equals(l_casted->m_func) && m_arg->equals(l_casted->m_arg);
}

// PRINT METHODS

void local::print(std::ostream& a_ostream) const
{
    a_ostream << m_index;
}

void global::print(std::ostream& a_ostream) const
{
    a_ostream << "G" << m_index;
}

void func::print(std::ostream& a_ostream) const
{
    a_ostream << "λ.(";
    m_body->print(a_ostream);
    a_ostream << ")";
}

void app::print(std::ostream& a_ostream) const
{
    a_ostream << "(";
    m_func->print(a_ostream);
    a_ostream << " ";
    m_arg->print(a_ostream);
    a_ostream << ")";
}

// LIFT METHODS

std::unique_ptr<expr> local::lift(size_t a_lift_amount, size_t a_cutoff) const
{
    if(m_index < a_cutoff)
        return l(m_index);

    return l(m_index + a_lift_amount);
}

std::unique_ptr<expr> global::lift(size_t a_lift_amount, size_t a_cutoff) const
{
    return g(m_index);
}

std::unique_ptr<expr> func::lift(size_t a_lift_amount, size_t a_cutoff) const
{
    // we don't increment here, since the goal is to lift the WHOLE function
    // (all locals inside) by the same amount (provided they are >= cutoff).
    return f(m_body->lift(a_lift_amount, a_cutoff));
}

std::unique_ptr<expr> app::lift(size_t a_lift_amount, size_t a_cutoff) const
{
    return a(m_func->lift(a_lift_amount, a_cutoff),
             m_arg->lift(a_lift_amount, a_cutoff));
}

// SUBSTITUTE METHODS

std::unique_ptr<expr>
local::substitute(size_t a_lift_amount, size_t a_var_index,
                  const std::unique_ptr<expr>& a_arg) const
{
    if(m_index > a_var_index)
        // this var is defined inside the redex, so it is
        //     now 1 level shallower.
        return l(m_index - 1);

    if(m_index < a_var_index)
        // leave the var alone, it was declared outside the redex
        return clone();

    // this var is the one we are substituting, so we must substitute it
    return a_arg->lift(a_lift_amount, a_var_index);
}

std::unique_ptr<expr>
global::substitute(size_t a_lift_amount, size_t a_var_index,
                   const std::unique_ptr<expr>& a_arg) const
{
    // doing a local substitution on a global variable is a no-op
    return clone();
}

std::unique_ptr<expr> func::substitute(size_t a_lift_amount, size_t a_var_index,
                                       const std::unique_ptr<expr>& a_arg) const
{
    // increment the binder depth
    return f(m_body->substitute(a_lift_amount + 1, a_var_index, a_arg));
}

std::unique_ptr<expr> app::substitute(size_t a_lift_amount, size_t a_var_index,
                                      const std::unique_ptr<expr>& a_arg) const
{
    // just substitute the function and argument
    return a(m_func->substitute(a_lift_amount, a_var_index, a_arg),
             m_arg->substitute(a_lift_amount, a_var_index, a_arg));
}

// REDUCE METHODS

std::unique_ptr<expr> local::reduce(size_t a_depth,
                                    const global_map& a_globals) const
{
    // log the current expr
    LOG_EXPR(a_depth, this);

    auto l_result = clone();

    // log the result
    LOG_EXPR(a_depth, l_result);

    return l_result;
}

std::unique_ptr<expr> global::reduce(size_t a_depth,
                                     const global_map& a_globals) const
{
    // log the current expr
    LOG_EXPR(a_depth, this);

    // look up the definition of the global variable
    const auto& l_definition = a_globals[m_index];

    // lift the definition (interestingly, here we set cutoff to zero since
    // all vars are defined in another scope, making it so we actually need to
    // lift them all)
    auto l_lifted_def = l_definition->lift(a_depth, 0);

    // reduce the lifted definition
    auto l_result = l_lifted_def->reduce(a_depth, a_globals);

    // log the result
    LOG_EXPR(a_depth, l_result);

    return l_result;
}

std::unique_ptr<expr> func::reduce(size_t a_depth,
                                   const global_map& a_globals) const
{
    // log the current expr
    LOG_EXPR(a_depth, this);

    // reduce the body, incrementing the depth
    auto l_result = f(m_body->reduce(a_depth + 1, a_globals));

    // log the result
    LOG_EXPR(a_depth, l_result);

    return l_result;
}

std::unique_ptr<expr> app::reduce(size_t a_depth,
                                  const global_map& a_globals) const
{
    // log the current expr
    LOG_EXPR(a_depth, this);

    // reduce the function to NF
    auto l_reduced_func = m_func->reduce(a_depth, a_globals);

    // check if the lhs is a beta-redex
    const func* l_beta_redex = dynamic_cast<func*>(l_reduced_func.get());

    if(!l_beta_redex)
        // leave the lhs in NF and reduce the rhs to NF
        return a(l_reduced_func, m_arg->reduce(a_depth, a_globals));

    // beta-contract the body (DON'T REDUCE ARG HERE, DUE TO NORMAL ORDER)
    std::unique_ptr<expr> l_substituted_body =
        l_beta_redex->m_body->substitute(0, a_depth, m_arg);

    // reduce the reduced contracted body
    auto l_result = l_substituted_body->reduce(a_depth, a_globals);

    // log the result
    LOG_EXPR(a_depth, l_result);

    return l_result;
}

// EXPR CLONE METHOD
std::unique_ptr<expr> expr::clone() const
{
    return lift(0, 0);
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

// FACTORY FUNCTIONS

std::unique_ptr<expr> l(size_t a_index)
{
    return std::unique_ptr<expr>(new local(a_index));
}

std::unique_ptr<expr> g(size_t a_index)
{
    return std::unique_ptr<expr>(new global(a_index));
}

std::unique_ptr<expr> f(const std::unique_ptr<expr>& a_body)
{
    return std::unique_ptr<expr>(new func(a_body->clone()));
}

std::unique_ptr<expr> a(const std::unique_ptr<expr>& a_func,
                        const std::unique_ptr<expr>& a_arg)
{
    return std::unique_ptr<expr>(new app(a_func->clone(), a_arg->clone()));
}

} // namespace lambda

#ifdef UNIT_TEST

#include "test_utils.hpp"
#include <iostream>

using namespace lambda;

void test_local_constructor()
{
    // index 0
    {
        auto l_local = l(0);
        const local* l_local_casted = dynamic_cast<local*>(l_local.get());
        assert(l_local_casted != nullptr);
        assert(l_local_casted->m_index == 0);
    }

    // index 1
    {
        auto l_local = l(1);
        const local* l_local_casted = dynamic_cast<local*>(l_local.get());
        assert(l_local_casted != nullptr);
        assert(l_local_casted->m_index == 1);
    }
}

void test_global_constructor()
{
    // index 0
    {
        auto l_global = g(0);
        const global* l_global_casted = dynamic_cast<global*>(l_global.get());
        assert(l_global_casted != nullptr);
        assert(l_global_casted->m_index == 0);
    }

    // index 1
    {
        auto l_global = g(1);
        const global* l_global_casted = dynamic_cast<global*>(l_global.get());
        assert(l_global_casted != nullptr);
        assert(l_global_casted->m_index == 1);
    }
}

void test_func_constructor()
{
    // local body
    {
        auto l_func = f(l(0));
        // get body
        const func* l_func_casted = dynamic_cast<func*>(l_func.get());
        assert(l_func_casted != nullptr);
        const auto& l_body = l_func_casted->m_body;
        // check if the body is a local
        const local* l_local = dynamic_cast<local*>(l_body.get());
        assert(l_local != nullptr);
        // check if the index is correct
        assert(l_local->m_index == 0);
    }
}

void test_app_constructor()
{
    // local application
    {
        auto l_app = a(l(0), l(1));
        // get the lhs
        const app* l_app_casted = dynamic_cast<app*>(l_app.get());
        assert(l_app_casted != nullptr);
        const auto& l_lhs = l_app_casted->m_func;
        // get the rhs
        const auto& l_rhs = l_app_casted->m_arg;

        // make sure they both are locals
        const local* l_lhs_local = dynamic_cast<local*>(l_lhs.get());
        const local* l_rhs_local = dynamic_cast<local*>(l_rhs.get());
        assert(l_lhs_local != nullptr);
        assert(l_rhs_local != nullptr);

        // make sure the indices are correct
        assert(l_lhs_local->m_index == 0);
        assert(l_rhs_local->m_index == 1);
    }
}

void test_local_equals()
{
    // index 0, equals index 0
    {
        auto l_local = l(0);
        auto l_local_other = l(0);
        assert(l_local->equals(l_local_other->clone()));
    }

    // index 0, equals index 1
    {
        auto l_local = l(0);
        auto l_local_other = l(1);
        assert(!l_local->equals(l_local_other->clone()));
    }

    // index 1, equals index 1
    {
        auto l_local = l(1);
        auto l_local_other = l(1);
        assert(l_local->equals(l_local_other->clone()));
    }

    // local equals global
    {
        auto l_local = l(0);
        auto l_global = g(0);
        assert(!l_local->equals(l_global->clone()));
    }

    // local equals func
    {
        auto l_local = l(0);
        auto l_func = f(l(0));
        assert(!l_local->equals(l_func->clone()));
    }

    // local equals app
    {
        auto l_local = l(0);
        auto l_app = a(l(0), l(0));
        assert(!l_local->equals(l_app->clone()));
    }
}

void test_global_equals()
{
    // index 0, equals index 0
    {
        auto l_global = g(0);
        auto l_global_other = g(0);
        assert(l_global->equals(l_global_other->clone()));
    }

    // index 0, equals index 1
    {
        auto l_global = g(0);
        auto l_global_other = g(1);
        assert(!l_global->equals(l_global_other->clone()));
    }

    // global equals local
    {
        auto l_global = g(0);
        auto l_local = l(0);
        assert(!l_global->equals(l_local->clone()));
    }
}

void test_func_equals()
{
    // local body, equals local body
    {
        auto l_local = l(0);
        auto l_func = f(l_local->clone());
        auto l_func_other = f(l_local->clone());
        assert(l_func->equals(l_func_other->clone()));
    }

    // global body, equals global body
    {
        auto l_global = g(0);
        auto l_func = f(l_global->clone());
        auto l_func_other = f(l_global->clone());
        assert(l_func->equals(l_func_other->clone()));
    }

    // func equals local
    {
        auto l_func = f(l(0));
        auto l_local = l(0);
        assert(!l_func->equals(l_local->clone()));
    }

    // func equals global
    {
        auto l_func = f(g(0));
        auto l_global = g(0);
        assert(!l_func->equals(l_global->clone()));
    }

    // func with different bodies
    {
        auto l_local = l(0);
        auto l_local_other = l(1);
        auto l_func = f(l_local->clone());
        auto l_func_other = f(l_local_other->clone());
        assert(!l_func->equals(l_func_other->clone()));
    }
}

void test_app_equals()
{
    // local lhs, local rhs, equals local lhs, local rhs
    {
        auto l_lhs_local = l(0);
        auto l_rhs_local = l(0);
        auto l_app = a(l_lhs_local->clone(), l_rhs_local->clone());
        auto l_app_other = a(l_lhs_local->clone(), l_rhs_local->clone());
        assert(l_app->equals(l_app_other->clone()));
    }

    // local lhs, local rhs, equals global lhs, local rhs
    {
        auto l_first_lhs_local = l(0);
        auto l_first_rhs_local = l(0);
        auto l_second_lhs_global = g(0);
        auto l_second_rhs_local = l(0);
        auto l_app = a(l_first_lhs_local->clone(), l_first_rhs_local->clone());
        auto l_app_other =
            a(l_second_lhs_global->clone(), l_second_rhs_local->clone());
        assert(!l_app->equals(l_app_other->clone()));
    }

    // local lhs, local rhs, equals global lhs, local rhs
    {
        auto l_first_lhs_local = l(0);
        auto l_first_rhs_local = l(0);
        auto l_second_lhs_global = g(0);
        auto l_second_rhs_local = l(0);
        auto l_app = a(l_first_lhs_local->clone(), l_first_rhs_local->clone());
        auto l_app_other =
            a(l_second_lhs_global->clone(), l_second_rhs_local->clone());
        assert(!l_app->equals(l_app_other->clone()));
    }
}

void test_local_lift()
{
    // index 0, lift 1 level
    {
        auto l_local = l(0);
        auto l_lifted = l_local->lift(1, 0);
        const local* l_lifted_local = dynamic_cast<local*>(l_lifted.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 1);
    }

    // index 1, lift 1 level
    {
        auto l_local = l(1);
        auto l_lifted = l_local->lift(1, 0);
        const local* l_lifted_local = dynamic_cast<local*>(l_lifted.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 2);
    }

    // index 1, lift 0 levels
    {
        auto l_local = l(1);
        auto l_lifted = l_local->lift(0, 0);
        const local* l_lifted_local = dynamic_cast<local*>(l_lifted.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 1);
    }
}

void test_global_lift()
{
    // index 0, lift 1 level
    {
        auto l_global = g(0);
        auto l_lifted = l_global->lift(1, 0);
        const global* l_lifted_global = dynamic_cast<global*>(l_lifted.get());
        assert(l_lifted_global != nullptr);

        // globals are left unchanged by lifting
        assert(l_lifted_global->m_index == 0);
    }

    // index 3, lift 2 levels
    {
        auto l_global = g(3);
        auto l_lifted = l_global->lift(2, 0);
        const global* l_lifted_global = dynamic_cast<global*>(l_lifted.get());
        assert(l_lifted_global != nullptr);

        // globals are left unchanged by lifting
        assert(l_lifted_global->m_index == 3);
    }
}

void test_func_lift()
{
    // local body, lift 1 level
    {
        auto l_local = l(0);
        auto l_func = f(l_local->clone());
        auto l_lifted = l_func->lift(1, 0);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const local* l_lifted_local =
            dynamic_cast<local*>(l_lifted_func->m_body.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 1);
    }

    // local body, lift 2 levels
    {
        auto l_local = l(1);
        auto l_func = f(l_local->clone());
        auto l_lifted = l_func->lift(2, 0);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const local* l_lifted_local =
            dynamic_cast<local*>(l_lifted_func->m_body.get());
        assert(l_lifted_local != nullptr);
        assert(l_lifted_local->m_index == 3);
    }
}

void test_app_lift()
{
    // local lhs, local rhs, lift 1 level
    {
        auto l_lhs_local = l(1);
        auto l_rhs_local = l(2);
        auto l_app = a(l_lhs_local->clone(), l_rhs_local->clone());
        auto l_lifted = l_app->lift(1, 0);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_func;
        const local* l_lifted_lhs_local =
            dynamic_cast<local*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 2);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_arg;
        const local* l_lifted_rhs_local =
            dynamic_cast<local*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 3);
    }

    // local lhs, local rhs, lift 2 levels
    {
        auto l_lhs_local = l(1);
        auto l_rhs_local = l(2);
        auto l_app = a(l_lhs_local->clone(), l_rhs_local->clone());
        auto l_lifted = l_app->lift(2, 0);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_func;
        const local* l_lifted_lhs_local =
            dynamic_cast<local*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 3);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_arg;
        const local* l_lifted_rhs_local =
            dynamic_cast<local*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 4);
    }
}

void test_local_substitute()
{
    // index 0, occurrance depth 0, substitute with a local
    {
        auto l_local = l(0);
        auto l_substitute = l(1);
        auto l_substituted = l_local->substitute(0, 0, l_substitute->clone());

        const local* l_substituted_local =
            dynamic_cast<local*>(l_substituted.get());
        assert(l_substituted_local != nullptr);
        assert(l_substituted_local->m_index == 1);
    }
    // index 0, occurrance depth 10, substitute with a local
    {
        auto l_local = l(0);
        auto l_substitute = l(1);
        auto l_substituted = l_local->substitute(10, 0, l_substitute->clone());

        const local* l_substituted_local =
            dynamic_cast<local*>(l_substituted.get());
        assert(l_substituted_local != nullptr);
        // it would be 1 if occurrance depth == 0, but since 10,
        //     l_substitute had to be lifted.
        assert(l_substituted_local->m_index == 11);
    }

    // index 2, occurrance depth 0, substitute with a local
    {
        auto l_local = l(2);
        auto l_substitute = l(3);
        auto l_substituted = l_local->substitute(0, 0, l_substitute->clone());

        const local* l_substituted_local =
            dynamic_cast<local*>(l_substituted.get());
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
        auto l_local = l(1);
        auto l_substitute = l(3);
        auto l_substituted = l_local->substitute(0, 0, l_substitute->clone());

        const local* l_substituted_local =
            dynamic_cast<local*>(l_substituted.get());
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
        auto l_local = l(2);
        auto l_substitute = l(3);
        auto l_substituted = l_local->substitute(10, 0, l_substitute->clone());

        const local* l_substituted_local =
            dynamic_cast<local*>(l_substituted.get());
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
        auto l_local = l(1);
        auto l_substitute = l(3);
        auto l_substituted = l_local->substitute(10, 0, l_substitute->clone());

        const local* l_substituted_local =
            dynamic_cast<local*>(l_substituted.get());
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
        auto l_global = g(0);
        auto l_local = l(1);
        const auto l_substituted = l_global->substitute(0, 0, l_local->clone());

        // should be global still
        const global* l_subbed_global =
            dynamic_cast<global*>(l_substituted.get());
        assert(l_subbed_global != nullptr);
        // globals are unaffected by substitution.
        assert(l_subbed_global->m_index == 0);
    }

    // index 0, depth 10, substitute with a local
    {
        auto l_global = g(0);
        auto l_local = l(1);
        const auto l_substituted =
            l_global->substitute(10, 0, l_local->clone());

        // should be global still
        const global* l_subbed_global =
            dynamic_cast<global*>(l_substituted.get());
        assert(l_subbed_global != nullptr);
        // globals are unaffected by substitution.
        assert(l_subbed_global->m_index == 0);
    }

    // index 10, depth 10, substitute with a local
    {
        auto l_global = g(10);
        auto l_local = l(1);
        const auto l_substituted =
            l_global->substitute(10, 0, l_local->clone());

        // should be global still
        const global* l_subbed_global =
            dynamic_cast<global*>(l_substituted.get());
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

        auto l_func = f(l(0)->clone());
        auto l_local = l(11);

        const auto l_subbed = l_func->substitute(0, 0, l_local->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const local* l_subbed_local =
            dynamic_cast<local*>(l_subbed_func->m_body.get());

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

        auto l_func = f(f(l(0)->clone())->clone());
        auto l_local = l(11);

        const auto l_subbed = l_func->substitute(0, 0, l_local->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        const func* l_subbed_func_2 =
            dynamic_cast<func*>(l_subbed_func->m_body.get());

        // get body
        const local* l_subbed_local =
            dynamic_cast<local*>(l_subbed_func_2->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_local != nullptr);

        // the index of the substitute is lifted by 2 (two binders)
        assert(l_subbed_local->m_index == 13);
    }

    // single composition lambda, outer depth 0, occurrance not found
    {
        auto l_func = f(l(1)->clone());
        auto l_global = g(11);

        const auto l_subbed = l_func->substitute(0, 0, l_global->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const local* l_subbed_local =
            dynamic_cast<local*>(l_subbed_func->m_body.get());

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
        auto l_lhs = l(0);
        auto l_rhs = l(0);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = l(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const local* l_subbed_lhs =
            dynamic_cast<local*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const local* l_subbed_rhs =
            dynamic_cast<local*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 11);
        assert(l_subbed_rhs->m_index == 11);
    }

    // app of locals, lhs is an occurrance
    {
        auto l_lhs = l(0);
        auto l_rhs = l(1);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = l(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const local* l_subbed_lhs =
            dynamic_cast<local*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const local* l_subbed_rhs =
            dynamic_cast<local*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 11);
        assert(l_subbed_rhs->m_index == 0);
    }

    // app of locals, rhs is an occurrance
    {
        auto l_lhs = l(1);
        auto l_rhs = l(0);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = l(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const local* l_subbed_lhs =
            dynamic_cast<local*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const local* l_subbed_rhs =
            dynamic_cast<local*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 0);
        assert(l_subbed_rhs->m_index == 11);
    }

    // app of locals, neither are occurrances
    {
        auto l_lhs = l(1);
        auto l_rhs = l(1);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = l(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const local* l_subbed_lhs =
            dynamic_cast<local*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const local* l_subbed_rhs =
            dynamic_cast<local*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 0);
        assert(l_subbed_rhs->m_index == 0);
    }

    // app of funcs, both with occurrances
    {
        auto l_lhs = f(l(0)->clone());
        auto l_rhs = f(l(0)->clone());
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = l(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const func* l_subbed_lhs =
            dynamic_cast<func*>(l_subbed_app->m_func.get());

        // make sure lhs is a func
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const func* l_subbed_rhs =
            dynamic_cast<func*>(l_subbed_app->m_arg.get());

        // make sure rhs is a func
        assert(l_subbed_rhs != nullptr);

        const local* l_lhs_local =
            dynamic_cast<local*>(l_subbed_lhs->m_body.get());

        // make sure body of lhs is a local
        assert(l_lhs_local != nullptr);

        const local* l_rhs_local =
            dynamic_cast<local*>(l_subbed_rhs->m_body.get());

        // make sure body of rhs is a local
        assert(l_rhs_local != nullptr);

        // make sure they have correct indices (lifted by 1 due to binders)
        assert(l_lhs_local->m_index == 12);
        assert(l_rhs_local->m_index == 12);
    }
}

void test_local_reduce()
{
    // local with var 0
    {
        auto l_expr = l(0);
        const auto l_reduced = l_expr->reduce(0, {});

        // cast the pointer
        const local* l_local = dynamic_cast<local*>(l_reduced.get());
        assert(l_local != nullptr);

        // make sure it has the same index
        assert(l_local->m_index == 0);
    }

    // local with var 1
    {
        auto l_expr = l(1);
        const auto l_reduced = l_expr->reduce(0, {});

        // cast the pointer
        const local* l_local = dynamic_cast<local*>(l_reduced.get());
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
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());
        l_globals.emplace_back(f(l(13)->clone())->clone());

        // set up reduction
        auto l_expr = g(0);
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // cast the pointer
        const func* l_casted = dynamic_cast<func*>(l_reduced.get());
        assert(l_casted != nullptr);

        // get body
        const local* l_local = dynamic_cast<local*>(l_casted->m_body.get());
        assert(l_local != nullptr);

        // make sure it has the same index
        assert(l_local->m_index == 0);
    }

    // global with index 1
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());
        l_globals.emplace_back(f(l(13)->clone())->clone());

        // set up reduction
        auto l_expr = g(1);
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // cast the pointer
        const func* l_casted = dynamic_cast<func*>(l_reduced.get());
        assert(l_casted != nullptr);

        // get body
        const local* l_local = dynamic_cast<local*>(l_casted->m_body.get());
        assert(l_local != nullptr);

        // make sure it has the same index
        assert(l_local->m_index == 13);
    }

    // 1 cascading global reduction
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());
        l_globals.emplace_back(g(0)->clone());

        // set up reduction
        auto l_expr = g(1);
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // cast the pointer
        const func* l_casted = dynamic_cast<func*>(l_reduced.get());
        assert(l_casted != nullptr);

        // get body
        const local* l_local = dynamic_cast<local*>(l_casted->m_body.get());
        assert(l_local != nullptr);

        // make sure it has the same index
        assert(l_local->m_index == 0);
    }

    // 2 cascading global reductions
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());
        l_globals.emplace_back(g(0)->clone());
        l_globals.emplace_back(g(1)->clone());

        // set up reduction
        auto l_expr = g(2);
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // cast the pointer
        const func* l_casted = dynamic_cast<func*>(l_reduced.get());
        assert(l_casted != nullptr);

        // get body
        const local* l_local = dynamic_cast<local*>(l_casted->m_body.get());
        assert(l_local != nullptr);

        // make sure it has the same index
        assert(l_local->m_index == 0);
    }

    // 2 cascading global reductions
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());
        l_globals.emplace_back(f(g(0)->clone())->clone());

        // set up reduction
        auto l_expr = g(1);
        const auto l_reduced = l_expr->reduce(0, l_globals);

        assert(l_reduced->equals(f(f(l(1)))));
    }
}

void test_func_reduce()
{
    // func with body of a local
    {
        auto l_expr = f(l(0)->clone());
        const auto l_reduced = l_expr->reduce(0, {});

        // make sure still a func
        const auto* l_func = dynamic_cast<func*>(l_reduced.get());
        assert(l_func != nullptr);

        // get body
        const auto* l_body = dynamic_cast<local*>(l_func->m_body.get());
        assert(l_body != nullptr);

        // make sure body is still same thing
        assert(l_body->m_index == 0);
    }

    // func with body of a global
    {
        // define globals
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());
        l_globals.emplace_back(f(l(13)->clone())->clone());

        auto l_expr = f(g(1)->clone());
        const auto l_reduced = l_expr->reduce(0, l_globals);

        assert(l_reduced->equals(f(f(l(14)))));
    }
}

void test_app_reduce()
{
    // app with lhs and rhs both locals
    {
        auto l_lhs = l(0);
        auto l_rhs = l(1);
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, {});

        const app* l_app = dynamic_cast<app*>(l_reduced.get());
        assert(l_app != nullptr);

        // lhs should be local
        const local* l_reduced_lhs = dynamic_cast<local*>(l_app->m_func.get());
        assert(l_reduced_lhs != nullptr);

        // rhs should be local
        const local* l_reduced_rhs = dynamic_cast<local*>(l_app->m_arg.get());
        assert(l_reduced_rhs != nullptr);

        // same on both (no reduction occurred)
        assert(l_reduced_lhs->m_index == 0);
        assert(l_reduced_rhs->m_index == 1);
    }

    // app with lhs global and rhs local
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = g(0);
        auto l_rhs = l(1);
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // make sure beta reduction did occurred
        assert(l_reduced->equals(l(1)));
    }

    // app with lhs func and rhs local
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = f(l(0)->clone());
        auto l_rhs = l(1);
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // make sure nothing changed
        // (beta reduction did not occur since rhs is local)
        assert(l_reduced->equals(l(1)));
    }

    // app with lhs func and rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = f(l(0)->clone());
        auto l_rhs = f(l(1)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // make sure beta-reduction occurred, with no lifting of indices
        assert(l_reduced->equals(l_rhs->clone()));
    }

    // app with lhs func (without occurrences of var 0) and rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = f(l(3)->clone());
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // make sure beta-reduction occurred, but no replacements.
        // other vars decremented by 1.
        assert(l_reduced->equals(l(2)->clone()));
    }

    // app with lhs (nested func with occurrences of var 0) and rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = f(f(l(0)->clone())->clone());
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // make sure beta-reduction occurred, with replacement,
        // and a lifting of 1 level
        assert(l_reduced->equals(f(f(l(6)->clone()))->clone()));
    }

    // app with lhs (nested func without occurrences of var 0) and rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = f(f(l(3)->clone())->clone());
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // make sure beta-reduction occurred, no replacements.
        // other vars decremented by 1.
        assert(l_reduced->equals(f(l(2)->clone())));
    }

    // app with lhs (app that doesnt reduce to func) and rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = a(l(3)->clone(), l(4)->clone());
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // make sure nothing changed
        // (both lhs and rhs were fully reduced already)
        assert(l_reduced->equals(l_expr->clone()));
    }

    // app with lhs (app with lhs (func without occurrances), rhs local)
    // and rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = a(f(l(3)->clone())->clone(), l(4)->clone());
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // lhs should have beta-reduced, but cannot consume rhs of app
        assert(l_reduced->equals(a(l(2), f(l(5)))));
    }

    // app with lhs (app with lhs (func without occurrances), rhs func)
    // and rhs func, where there are too many arguments supplied
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = a(f(l(3)->clone())->clone(), f(l(4)->clone())->clone());
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // lhs of app should beta-reduce, but lhs is not capable of consuming 2
        // args. Thus NF is an application with LHS beta-reduced once.
        assert(l_reduced->equals(a(l(2)->clone(), f(l(5)->clone()))->clone()));
    }

    // app with lhs (app with lhs (func without occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs =
            a(f(f(l(3)->clone())->clone())->clone(), f(l(4)->clone())->clone());
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // should beta-reduce twice, consuming all args. No replacements, only
        // decrementing twice.
        assert(l_reduced->equals(l(1)->clone()));
    }

    // app with lhs (app with lhs (func WITH occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs =
            a(f(f(l(0)->clone())->clone())->clone(), f(l(4)->clone())->clone());
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // should beta-reduce twice, consuming all args.
        // becomes first arg, without lifting. (lifting occurred but was undone
        // by second arg)
        assert(l_reduced->equals(f(l(4)->clone())));
    }

    // app with lhs (app with lhs (func WITH occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs =
            a(f(f(l(1)->clone())->clone())->clone(), f(l(4)->clone())->clone());
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // should beta-reduce twice, consuming all args.
        // becomes second arg, without lifting. (lifting occurred but was undone
        // by second arg)
        assert(l_reduced->equals(l_rhs->clone()));
    }

    // app with lhs (global that directly reduces to func with occurrances) and
    // rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());

        auto l_lhs = g(0);
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // should beta-reduce, consuming the arg.
        // A replacement occurred, and no lifting occurred.
        assert(l_reduced->equals(l_rhs->clone()));
    }

    // app with lhs (global that indirectly reduces to func with occurrances)
    // and rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(0)->clone())->clone());
        l_globals.emplace_back(g(0)->clone());

        auto l_lhs = g(1);
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // firstly, a delta cascade occurs, reducing the global to a func.
        // then, the func should beta-reduce, consuming the arg.
        // A replacement occurred, and no lifting occurred.
        assert(l_reduced->equals(l_rhs->clone()));
    }

    // app with lhs (global that indirectly reduces to func without occurrances)
    // and rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(l(1)->clone())->clone());
        l_globals.emplace_back(g(0)->clone());

        auto l_lhs = g(1);
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // firstly, a delta cascade occurs, reducing the global to a func.
        // then, the func should beta-reduce, consuming the arg.
        // No replacements, only decrementing.
        assert(l_reduced->equals(l(0)->clone()));
    }

    // app with lhs (global that indirectly reduces to func with occurrances,
    // but needs lifting) and rhs func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(f(l(0)->clone()))->clone());
        l_globals.emplace_back(g(0)->clone());

        auto l_lhs = g(1);
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // firstly, a delta cascade occurs, reducing the global to a func.
        // then, the func should beta-reduce, consuming the arg.
        // A replacement occurred, and a lifting of 1 level occurred.
        assert(l_reduced->equals(f(f(l(6)->clone()))->clone()));
    }

    // app with lhs (global that reduces to app which reduces to func) and rhs
    // func
    {
        // create global definitions
        expr::global_map l_globals{};
        l_globals.emplace_back(f(f(l(1)->clone()))->clone());
        l_globals.emplace_back(a(g(0)->clone(), g(10)->clone())->clone());

        auto l_lhs = g(1);
        auto l_rhs = f(l(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->reduce(0, l_globals);

        // firstly, a delta cascade occurs, reducing the global to a func.
        // then, a beta-reduction occurs, consuming the arg defined in global1
        // def. then, the returned func should beta-reduce, consuming the arg.
        // A replacement occurred, and no lifting occurred.
        assert(l_reduced->equals(f(l(5)->clone())));
    }
}

void generic_use_case_test()
{
    using namespace lambda;
    expr::global_map l_globals{};

    // church booleans

    // true
    l_globals.emplace_back(f(f(l(0))));
    const auto TRUE = g(0);
    // false
    l_globals.emplace_back(f(f(l(1))));
    const auto FALSE = g(1);

    // test the church bools
    {
        // true case
        const auto l_true_case = f(l(10));

        // false case
        const auto l_false_case = f(l(11));

        // test the true case
        const auto l_true_case_app =
            a(a(TRUE, l_true_case), l_false_case)->reduce(0, l_globals);

        // test the false case
        const auto l_false_case_app =
            a(a(FALSE, l_true_case), l_false_case)->reduce(0, l_globals);

        std::cout << "true case app: ";
        l_true_case_app->print(std::cout);
        std::cout << std::endl;

        std::cout << "false case app: ";
        l_false_case_app->print(std::cout);
        std::cout << std::endl;

        // test the true case
        assert(l_true_case_app->equals(l_true_case->clone()));
        assert(l_false_case_app->equals(l_false_case->clone()));
    }

    // add church numerals

    // 0
    l_globals.emplace_back(f(f(l(1))));
    const auto ZERO = g(2);

    // succ
    l_globals.emplace_back(f(f(f(a(l(1), a(a(l(0), l(1)), l(2)))))));
    const auto SUCC = g(3);

    // test succ church numerals
    {

        std::cout << "zero: ";
        ZERO->print(std::cout);
        std::cout << std::endl;
        std::cout << "G2: ";
        l_globals.at(2)->print(std::cout);
        std::cout << std::endl;
        std::cout << "succ: ";
        SUCC->print(std::cout);
        std::cout << std::endl;
        std::cout << "G3: ";
        l_globals.at(3)->print(std::cout);
        std::cout << std::endl;

        // reduce zero
        const auto ZERO_REDUCED = ZERO->reduce(0, l_globals);
        std::cout << "zero reduced: ";
        ZERO_REDUCED->print(std::cout);
        std::cout << std::endl;

        // define one
        const auto ONE = a(SUCC, ZERO)->reduce(0, l_globals);
        std::cout << "one: ";
        ONE->print(std::cout);
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        // define two
        const auto TWO = a(SUCC, ONE)->reduce(0, l_globals);
        std::cout << "two: ";
        TWO->print(std::cout);
        std::cout << std::endl;

        // define three
        const auto THREE = a(SUCC, TWO)->reduce(0, l_globals);
        std::cout << "three: ";
        THREE->print(std::cout);
        std::cout << std::endl;

        // define four
        const auto FOUR = a(SUCC, THREE)->reduce(0, l_globals);
        std::cout << "four: ";
        FOUR->print(std::cout);
        std::cout << std::endl;
        // define five
        const auto FIVE = a(SUCC, FOUR)->reduce(0, l_globals);
        std::cout << "five: ";
        FIVE->print(std::cout);
        std::cout << std::endl;

        assert(ONE->equals(f(f(a(l(0), l(1))))));
        assert(TWO->equals(f(f(a(l(0), a(l(0), l(1)))))));
        assert(THREE->equals(f(f(a(l(0), a(l(0), a(l(0), l(1))))))));
        assert(FOUR->equals(f(f(a(l(0), a(l(0), a(l(0), a(l(0), l(1)))))))));
        assert(FIVE->equals(
            f(f(a(l(0), a(l(0), a(l(0), a(l(0), a(l(0), l(1))))))))));
    }

    // add
    l_globals.emplace_back(
        f(f(f(f(a(a(l(0), l(2)), a(a(l(1), l(2)), l(3))))))));
    const auto ADD = g(4);

    // test add church numerals
    {
        // define one
        const auto ONE = a(SUCC, ZERO)->reduce(0, l_globals);
        // define two
        const auto TWO = a(SUCC, ONE)->reduce(0, l_globals);
        // define three
        const auto THREE = a(SUCC, TWO)->reduce(0, l_globals);
        // define four
        const auto FOUR = a(SUCC, THREE)->reduce(0, l_globals);
        // define five
        const auto FIVE = a(SUCC, FOUR)->reduce(0, l_globals);

        // add one and one
        const auto ADD_ONE_ONE = a(a(ADD, ONE), ONE)->reduce(0, l_globals);

        std::cout << "add one one: ";
        ADD_ONE_ONE->print(std::cout);
        std::cout << std::endl;

        // add one and two
        const auto ADD_ONE_TWO = a(a(ADD, ONE), TWO)->reduce(0, l_globals);

        std::cout << "add one two: ";
        ADD_ONE_TWO->print(std::cout);
        std::cout << std::endl;

        // add two and two
        const auto ADD_TWO_TWO = a(a(ADD, TWO), TWO)->reduce(0, l_globals);

        std::cout << "add two two: ";
        ADD_TWO_TWO->print(std::cout);
        std::cout << std::endl;

        // add three and two
        const auto ADD_THREE_TWO = a(a(ADD, THREE), TWO)->reduce(0, l_globals);

        std::cout << "add three two: ";
        ADD_THREE_TWO->print(std::cout);
        std::cout << std::endl;

        // add five and five
        const auto ADD_FIVE_FIVE = a(a(ADD, FIVE), FIVE)->reduce(0, l_globals);

        std::cout << "add five five: ";
        ADD_FIVE_FIVE->print(std::cout);
        std::cout << std::endl;

        // assertions
        assert(ADD_ONE_ONE->equals(f(f(a(l(0), a(l(0), l(1)))))));
        assert(ADD_ONE_TWO->equals(f(f(a(l(0), a(l(0), a(l(0), l(1))))))));
        assert(ADD_TWO_TWO->equals(
            f(f(a(l(0), a(l(0), a(l(0), a(l(0), l(1)))))))));
        assert(ADD_THREE_TWO->equals(
            f(f(a(l(0), a(l(0), a(l(0), a(l(0), a(l(0), l(1))))))))));
        assert(ADD_FIVE_FIVE->equals(f(f(a(
            l(0),
            a(l(0),
              a(l(0),
                a(l(0),
                  a(l(0),
                    a(l(0), a(l(0), a(l(0), a(l(0), a(l(0), l(1)))))))))))))));
    }
}

void lambda_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_local_constructor);
    TEST(test_global_constructor);
    TEST(test_func_constructor);
    TEST(test_app_constructor);

    TEST(test_local_equals);
    TEST(test_global_equals);
    TEST(test_func_equals);
    TEST(test_app_equals);

    TEST(test_local_lift);
    TEST(test_global_lift);
    TEST(test_func_lift);
    TEST(test_app_lift);

    TEST(test_local_substitute);
    TEST(test_global_substitute);
    TEST(test_func_substitute);
    TEST(test_app_substitute);

    TEST(test_local_reduce);
    TEST(test_global_reduce);
    TEST(test_func_reduce);
    TEST(test_app_reduce);
    TEST(generic_use_case_test);
}

#endif
