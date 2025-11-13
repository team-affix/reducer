#include "../include/lambda.hpp"

#define VERBOSE_LOGS 1
#if VERBOSE_LOGS

#include <iostream>

#define LOG_EXPR(a_expr)                                                       \
    a_expr->print(std::cout);                                                  \
    std::cout << std::endl;

#else

// no-op
#define LOG_EXPR(a_expr)

#endif

namespace lambda
{

// EQUALS METHODS

bool var::equals(const std::unique_ptr<expr>& a_other) const
{
    const var* l_casted = dynamic_cast<const var*>(a_other.get());

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

void var::print(std::ostream& a_ostream) const
{
    a_ostream << m_index;
}

void func::print(std::ostream& a_ostream) const
{
    a_ostream << "λ.";
    m_body->print(a_ostream);
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

std::unique_ptr<expr> var::lift(size_t a_lift_amount, size_t a_cutoff) const
{
    if(m_index < a_cutoff)
        return v(m_index);

    return v(m_index + a_lift_amount);
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

std::unique_ptr<expr> var::substitute(size_t a_lift_amount, size_t a_var_index,
                                      const std::unique_ptr<expr>& a_arg) const
{
    if(m_index > a_var_index)
        // this var is defined inside the redex, so it is
        //     now 1 level shallower.
        return v(m_index - 1);

    if(m_index < a_var_index)
        // leave the var alone, it was declared outside the redex
        return clone();

    // this var is the one we are substituting, so we must substitute it
    return a_arg->lift(a_lift_amount, a_var_index);
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

std::unique_ptr<expr> var::reduce_one_step(size_t a_depth) const
{
    // variables cannot reduce
    return nullptr;
}

std::unique_ptr<expr> func::reduce_one_step(size_t a_depth) const
{
    // check if body can reduce
    auto l_reduced_body = m_body->reduce_one_step(a_depth + 1);

    if(!l_reduced_body)
        // body cannot reduce, so this function cannot reduce
        return nullptr;

    // return the new function with the reduced body
    return f(std::move(l_reduced_body));
}

std::unique_ptr<expr> app::reduce_one_step(size_t a_depth) const
{
    // see if this app is a beta-redex
    const func* l_lhs_func = dynamic_cast<const func*>(m_func.get());

    // if the lhs is a function, beta-contract the body
    if(l_lhs_func)
        return l_lhs_func->m_body->substitute(0, a_depth, m_arg);

    // try to reduce lhs
    auto l_reduced_lhs = m_func->reduce_one_step(a_depth);

    // if lhs can reduce, leave rhs alone and return
    if(l_reduced_lhs)
        return a(std::move(l_reduced_lhs), m_arg->clone());

    // try to reduce rhs
    auto l_reduced_rhs = m_arg->reduce_one_step(a_depth);

    // if rhs can reduce, leave lhs alone and return
    if(l_reduced_rhs)
        return a(m_func->clone(), std::move(l_reduced_rhs));

    // otherwise, return nullptr
    return nullptr;
}

// EXPR CLONE METHOD
std::unique_ptr<expr> expr::clone() const
{
    return lift(0, 0);
}

// EXPR NORMALIZE METHOD
std::unique_ptr<expr> expr::normalize() const
{
    // start with the original expression
    std::unique_ptr<expr> l_result = clone();

    // log the original expression
    LOG_EXPR(l_result);

    // reduce the expression until it cannot reduce anymore
    while(true)
    {
        // try to reduce the expression by one step
        auto l_reduced = l_result->reduce_one_step(0);

        // if the expression cannot reduce, break
        if(!l_reduced)
            break;

        // set the result to the reduced expression and continue
        l_result = std::move(l_reduced);

        // log the reduction
        LOG_EXPR(l_result);
    }

    // return the normalized expression
    return l_result;
}

// CONSTRUCTORS
expr::expr()
{
}

var::var(size_t a_index) : expr(), m_index(a_index)
{
}

func::func(std::unique_ptr<expr>&& a_body) : expr(), m_body(std::move(a_body))
{
}

app::app(std::unique_ptr<expr>&& a_func, std::unique_ptr<expr>&& a_arg)
    : expr(), m_func(std::move(a_func)), m_arg(std::move(a_arg))
{
}

// FACTORY FUNCTIONS

std::unique_ptr<expr> v(size_t a_index)
{
    return std::unique_ptr<expr>(new var(a_index));
}

std::unique_ptr<expr> f(std::unique_ptr<expr>&& a_body)
{
    return std::unique_ptr<expr>(new func(std::move(a_body)));
}

std::unique_ptr<expr> a(std::unique_ptr<expr>&& a_func,
                        std::unique_ptr<expr>&& a_arg)
{
    return std::unique_ptr<expr>(new app(std::move(a_func), std::move(a_arg)));
}

} // namespace lambda

#ifdef UNIT_TEST

#include "test_utils.hpp"
#include <iostream>
#include <list>

using namespace lambda;

void test_var_constructor()
{
    // index 0
    {
        auto l_var = v(0);
        const var* l_var_casted = dynamic_cast<var*>(l_var.get());
        assert(l_var_casted != nullptr);
        assert(l_var_casted->m_index == 0);
    }

    // index 1
    {
        auto l_var = v(1);
        const var* l_var_casted = dynamic_cast<var*>(l_var.get());
        assert(l_var_casted != nullptr);
        assert(l_var_casted->m_index == 1);
    }
}

void test_func_constructor()
{
    // local body
    {
        auto l_func = f(v(0));
        // get body
        const func* l_func_casted = dynamic_cast<func*>(l_func.get());
        assert(l_func_casted != nullptr);
        const auto& l_body = l_func_casted->m_body;
        // check if the body is a local
        const var* l_var = dynamic_cast<var*>(l_body.get());
        assert(l_var != nullptr);
        // check if the index is correct
        assert(l_var->m_index == 0);
    }
}

void test_app_constructor()
{
    // local application
    {
        auto l_app = a(v(0), v(1));
        // get the lhs
        const app* l_app_casted = dynamic_cast<app*>(l_app.get());
        assert(l_app_casted != nullptr);
        const auto& l_lhs = l_app_casted->m_func;
        // get the rhs
        const auto& l_rhs = l_app_casted->m_arg;

        // make sure they both are locals
        const var* l_lhs_var = dynamic_cast<var*>(l_lhs.get());
        const var* l_rhs_var = dynamic_cast<var*>(l_rhs.get());
        assert(l_lhs_var != nullptr);
        assert(l_rhs_var != nullptr);

        // make sure the indices are correct
        assert(l_lhs_var->m_index == 0);
        assert(l_rhs_var->m_index == 1);
    }
}

void test_var_equals()
{
    // index 0, equals index 0
    {
        auto l_var = v(0);
        auto l_local_other = v(0);
        assert(l_var->equals(l_local_other->clone()));
    }

    // index 0, equals index 1
    {
        auto l_var = v(0);
        auto l_local_other = v(1);
        assert(!l_var->equals(l_local_other->clone()));
    }

    // index 1, equals index 1
    {
        auto l_var = v(1);
        auto l_local_other = v(1);
        assert(l_var->equals(l_local_other->clone()));
    }

    // var equals global
    // {
    //     auto l_var = l(0);
    //     auto l_global = g(0);
    //     assert(!l_var->equals(l_global->clone()));
    // }

    // local equals func
    {
        auto l_var = v(0);
        auto l_func = f(v(0));
        assert(!l_var->equals(l_func->clone()));
    }

    // local equals app
    {
        auto l_var = v(0);
        auto l_app = a(v(0), v(0));
        assert(!l_var->equals(l_app->clone()));
    }
}

void test_func_equals()
{
    // local body, equals local body
    {
        auto l_var = v(0);
        auto l_func = f(l_var->clone());
        auto l_func_other = f(l_var->clone());
        assert(l_func->equals(l_func_other->clone()));
    }

    // func equals local
    {
        auto l_func = f(v(0));
        auto l_var = v(0);
        assert(!l_func->equals(l_var->clone()));
    }

    // func with different bodies
    {
        auto l_var = v(0);
        auto l_local_other = v(1);
        auto l_func = f(l_var->clone());
        auto l_func_other = f(l_local_other->clone());
        assert(!l_func->equals(l_func_other->clone()));
    }
}

void test_app_equals()
{
    // local lhs, local rhs, equals local lhs, local rhs
    {
        auto l_lhs_var = v(0);
        auto l_rhs_var = v(0);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_app_other = a(l_lhs_var->clone(), l_rhs_var->clone());
        assert(l_app->equals(l_app_other->clone()));
    }

    // different applications
    {
        auto l_lhs_lhs = v(1);
        auto l_lhs_rhs = v(0);
        auto l_rhs_lhs = v(0);
        auto l_rhs_rhs = v(0);
        auto l_lhs = a(l_lhs_lhs->clone(), l_lhs_rhs->clone());
        auto l_rhs = a(l_rhs_lhs->clone(), l_rhs_rhs->clone());
        assert(!l_lhs->equals(l_rhs));
    }
}

void test_var_lift()
{
    // index 0, lift 1 level
    {
        auto l_var = v(0);
        auto l_lifted = l_var->lift(1, 0);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 1);
    }

    // index 1, lift 1 level
    {
        auto l_var = v(1);
        auto l_lifted = l_var->lift(1, 0);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 2);
    }

    // index 1, lift 0 levels
    {
        auto l_var = v(1);
        auto l_lifted = l_var->lift(0, 0);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 1);
    }

    // index 0, lift 1 level, cutoff 1
    {
        auto l_var = v(0);
        auto l_lifted = l_var->lift(1, 1);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 0);
    }

    // index 1, lift 2 levels, cutoff 1
    {
        auto l_var = v(1);
        auto l_lifted = l_var->lift(2, 1);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 3);
    }

    // index 1, lift 2 levels, cutoff 2
    {
        auto l_var = v(1);
        auto l_lifted = l_var->lift(2, 2);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 1);
    }

    // Edge case: index equals cutoff, should be lifted (>= cutoff)
    // index 3, lift 5 levels, cutoff 3
    {
        auto l_var = v(3);
        auto l_lifted = l_var->lift(5, 3);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 8); // 3 + 5
    }

    // Edge case: index just below cutoff
    // index 4, lift 3 levels, cutoff 5
    {
        auto l_var = v(4);
        auto l_lifted = l_var->lift(3, 5);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 4); // not lifted
    }

    // Higher cutoff value test
    // index 7, lift 10 levels, cutoff 3
    {
        auto l_var = v(7);
        auto l_lifted = l_var->lift(10, 3);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 17); // 7 + 10
    }

    // Multiple different cutoffs - lower index below cutoff
    // index 2, lift 4 levels, cutoff 10
    {
        auto l_var = v(2);
        auto l_lifted = l_var->lift(4, 10);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 2); // not lifted
    }
}

void test_func_lift()
{
    // local body, lift 1 level
    {
        auto l_var = v(0);
        auto l_func = f(l_var->clone());
        auto l_lifted = l_func->lift(1, 0);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->m_body.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 1);
    }

    // local body, lift 2 levels
    {
        auto l_var = v(1);
        auto l_func = f(l_var->clone());
        auto l_lifted = l_func->lift(2, 0);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->m_body.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 3);
    }

    // local body, lift 1 level, cutoff 1
    {
        auto l_var = v(0);
        auto l_func = f(l_var->clone());
        auto l_lifted = l_func->lift(1, 1);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->m_body.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 0);
    }

    // local body, lift 2 levels, cutoff 2
    {
        auto l_var = v(2);
        auto l_func = f(l_var->clone());
        auto l_lifted = l_func->lift(2, 2);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->m_body.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 4);
    }

    // func with app body, mixed locals with various cutoffs
    // Body: (2 5 8), lift 3, cutoff 5
    {
        auto l_body = a(a(v(2)->clone(), v(5)->clone()), v(8)->clone());
        auto l_func = f(l_body->clone());
        auto l_lifted = l_func->lift(3, 5);

        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);

        // Expected: (2 8 11)
        // l(2) < 5, not lifted
        // l(5) >= 5, lifted to 8
        // l(8) >= 5, lifted to 11
        auto l_expected = f(a(a(v(2)->clone(), v(8)->clone()), v(11)->clone()));
        assert(l_lifted->equals(l_expected));
    }

    // func with nested func, testing cutoff propagation
    // f(f((1 3 6))), lift 2, cutoff 3
    {
        auto l_inner_body = a(a(v(1)->clone(), v(3)->clone()), v(6)->clone());
        auto l_body = f(l_inner_body->clone());
        auto l_func = f(l_body->clone());
        auto l_lifted = l_func->lift(2, 3);

        // Expected: f(f((1 5 8)))
        // l(1) < 3, not lifted
        // l(3) >= 3, lifted to 5
        // l(6) >= 3, lifted to 8
        auto l_expected =
            f(f(a(a(v(1)->clone(), v(5)->clone()), v(8)->clone())));
        assert(l_lifted->equals(l_expected));
    }

    // func with higher cutoff than any local
    // f(l(2)), lift 5, cutoff 10
    {
        auto l_func = f(v(2)->clone());
        auto l_lifted = l_func->lift(5, 10);

        // l(2) < 10, not lifted
        auto l_expected = f(v(2)->clone());
        assert(l_lifted->equals(l_expected));
    }
}

void test_app_lift()
{
    // local lhs, local rhs, lift 1 level
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_lifted = l_app->lift(1, 0);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_func;
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 2);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_arg;
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 3);
    }

    // local lhs, local rhs, lift 2 levels
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_lifted = l_app->lift(2, 0);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_func;
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 3);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_arg;
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 4);
    }

    // local lhs, local rhs, lift 1 level, cutoff 1
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_lifted = l_app->lift(1, 1);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_func;
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 2);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_arg;
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 3);
    }

    // local lhs, local rhs, lift 2 levels, cutoff 2
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_lifted = l_app->lift(2, 2);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_func;
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 1);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_arg;
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 4);
    }

    // app with mixed locals, lift 4, cutoff 3
    // (1 2 3 4 5) - mix below, at, and above cutoff
    {
        auto l_app = a(
            a(a(a(v(1)->clone(), v(2)->clone()), v(3)->clone()), v(4)->clone()),
            v(5)->clone());
        auto l_lifted = l_app->lift(4, 3);

        // Expected: (1 2 7 8 9)
        // l(1), l(2) < 3, not lifted
        // l(3), l(4), l(5) >= 3, lifted by 4
        auto l_expected = a(
            a(a(a(v(1)->clone(), v(2)->clone()), v(7)->clone()), v(8)->clone()),
            v(9)->clone());
        assert(l_lifted->equals(l_expected));
    }

    // app with nested funcs, cutoff applies throughout
    // (f(l(2)) f(l(4))), lift 3, cutoff 3
    {
        auto l_lhs = f(v(2)->clone());
        auto l_rhs = f(v(4)->clone());
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_lifted = l_app->lift(3, 3);

        // Expected: (f(l(2)) f(l(7)))
        // l(2) < 3, not lifted
        // l(4) >= 3, lifted to 7
        auto l_expected = a(f(v(2)->clone()), f(v(7)->clone()));
        assert(l_lifted->equals(l_expected));
    }

    // app with complex nested structure, various cutoffs
    // ((1 6) (f(3) f(8))), lift 2, cutoff 5
    {
        auto l_func_left = a(v(1)->clone(), v(6)->clone());
        auto l_func_right = a(f(v(3)->clone()), f(v(8)->clone()));
        auto l_app = a(l_func_left->clone(), l_func_right->clone());
        auto l_lifted = l_app->lift(2, 5);

        // Expected: ((1 8) (f(3) f(10)))
        // l(1) < 5, not lifted
        // l(6) >= 5, lifted to 8
        // l(3) < 5, not lifted
        // l(8) >= 5, lifted to 10
        auto l_expected = a(a(v(1)->clone(), v(8)->clone()),
                            a(f(v(3)->clone()), f(v(10)->clone())));
        assert(l_lifted->equals(l_expected));
    }

    // app with high cutoff, nothing gets lifted
    // (3 4 5), lift 10, cutoff 20
    {
        auto l_app = a(a(v(3)->clone(), v(4)->clone()), v(5)->clone());
        auto l_lifted = l_app->lift(10, 20);

        // All < 20, none lifted
        auto l_expected = a(a(v(3)->clone(), v(4)->clone()), v(5)->clone());
        assert(l_lifted->equals(l_expected));
    }

    // app with cutoff 0, everything gets lifted
    // (0 1 2), lift 5, cutoff 0
    {
        auto l_app = a(a(v(0)->clone(), v(1)->clone()), v(2)->clone());
        auto l_lifted = l_app->lift(5, 0);

        // All >= 0, all lifted by 5
        auto l_expected = a(a(v(5)->clone(), v(6)->clone()), v(7)->clone());
        assert(l_lifted->equals(l_expected));
    }
}

void test_var_substitute()
{
    // index 0, occurrance depth 0, substitute with a local
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        auto l_substituted = l_var->substitute(0, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);
        assert(l_substituted_var->m_index == 1);
    }

    // index 0, occurrance depth 10, substitute with a local
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        auto l_substituted = l_var->substitute(10, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);
        // it would be 1 if occurrance depth == 0, but since 10,
        //     l_substitute had to be lifted.
        assert(l_substituted_var->m_index == 11);
    }

    // index 2, occurrance depth 0, substitute with a local
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(0, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->m_index == 1);
    }

    // index 1, occurrance depth 0, substitute with a local
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(0, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->m_index == 0);
    }

    // index 2, occurrance depth 10, substitute with a local
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(10, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->m_index == 1);
    }

    // index 1, occurrance depth 10, substitute with a local
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(10, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->m_index == 0);
    }

    // index 0, occurrance depth 0, substitute with a local, a_var_index 1
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        auto l_substituted = l_var->substitute(0, 1, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);
        assert(l_substituted_var->m_index == 0);
    }

    // index 0, occurrance depth 10, substitute with a local, a_var_index 1
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        auto l_substituted = l_var->substitute(10, 1, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);
        // it would be 10 if a_var_index == 0, but since 1,
        //     not only were there no occurrances, but the
        //     var(0) was bound before cutoff (a_var_index == 1)
        //     so no lifting occurred.
        assert(l_substituted_var->m_index == 0);
    }

    // index 2, occurrance depth 0, substitute with a local, a_var_index 2
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(0, 2, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution trivially finds var(2) and substitutes with var(3),
        // without lifting as the redex body has no binders.
        assert(l_substituted_var->m_index == 3);
    }

    // index 1, occurrance depth 0, substitute with a local, a_var_index 2
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(0, 2, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution does not find var(2),
        // but var(1) is left alone since it was bound before cutoff
        // (a_var_index == 2)
        assert(l_substituted_var->m_index == 1);
    }

    // index 2, occurrance depth 10, substitute with a local, a_var_index 2
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(10, 2, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution finds var(2) and substitutes with var(3),
        // and lifts by 10 levels since the redex body has 10 binders.
        assert(l_substituted_var->m_index == 13);
    }

    // index 1, occurrance depth 10, substitute with a local, a_var_index 2
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(10, 2, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // no var(2) was found, so var(1) is left alone since it was bound
        // before cutoff (a_var_index == 2)
        assert(l_substituted_var->m_index == 1);
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

        auto l_func = f(v(0)->clone());
        auto l_var = v(11);

        const auto l_subbed = l_func->substitute(0, 0, l_var->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the index of the substitute is lifted by 1 (one binder)
        assert(l_subbed_var->m_index == 12);
    }
    // doublle composition lambda, outer depth 0, occurrance found
    {
        // it should be noted:
        // when saying here that l_func has a body local of index 0,
        // that the local does NOT reference the binder introduced by
        // l_func. This is because, when subbing, it is implied that
        // there USED to be an even more outer binder (which 0 would
        // have been bound to, hence the substitution DOES take place)

        auto l_func = f(f(v(0)->clone())->clone());
        auto l_var = v(11);

        const auto l_subbed = l_func->substitute(0, 0, l_var->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        const func* l_subbed_func_2 =
            dynamic_cast<func*>(l_subbed_func->m_body.get());

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func_2->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the index of the substitute is lifted by 2 (two binders)
        assert(l_subbed_var->m_index == 13);
    }

    // single composition lambda, outer depth 0, occurrance NOT found,
    // a_var_index 1
    {
        // it should be noted:
        // when saying here that l_func has a body local of index 0,
        // that the local does NOT reference the binder introduced by
        // l_func. This is because, when subbing, it is implied that
        // there USED to be an even more outer binder (which 0 would
        // have been bound to, hence the substitution DOES take place)

        auto l_func = f(v(0)->clone());
        auto l_var = v(11);

        const auto l_subbed = l_func->substitute(0, 1, l_var->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the local is unchanged as it was not replaced and it was bound before
        // cutoff (a_var_index == 1) so no lifting occurred.
        assert(l_subbed_var->m_index == 0);
    }

    // doublle composition lambda, outer depth 0, occurrance not found,
    // a_var_index 1
    {
        // it should be noted:
        // when saying here that l_func has a body local of index 0,
        // that the local does NOT reference the binder introduced by
        // l_func. This is because, when subbing, it is implied that
        // there USED to be an even more outer binder (which 0 would
        // have been bound to, hence the substitution DOES take place)

        auto l_func = f(f(v(0)->clone())->clone());
        auto l_var = v(11);

        const auto l_subbed = l_func->substitute(0, 1, l_var->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        const func* l_subbed_func_2 =
            dynamic_cast<func*>(l_subbed_func->m_body.get());

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func_2->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the local is unchanged as it was not replaced and it was bound before
        // cutoff (a_var_index == 1) so no lifting occurred.
        assert(l_subbed_var->m_index == 0);
    }

    // func with occurrence in its body, a_lift_amount > 0
    {
        // Create a function whose body contains a local with index 2 (is the
        // occurrence)
        auto l_func = f(v(2)->clone());
        auto l_sub = v(7);

        // substitute at depth = 6 (5 + 1), var_index=2, so a_lift_amount=6
        const auto l_subbed = l_func->substitute(5, 2, l_sub->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());
        assert(l_subbed_func != nullptr);

        // get body, should be a local with index = 7 + 6 = 13
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func->m_body.get());
        assert(l_subbed_var != nullptr);
        assert(l_subbed_var->m_index == 13);
    }

    // func with app body containing mixed locals, var_index=3
    // Tests that locals < var_index are left alone
    {
        // Body: (0 1 2 3 4) - mix of locals below, at, and above var_index=3
        auto l_body = a(
            a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()), v(3)->clone()),
            v(4)->clone());
        auto l_func = f(l_body->clone());
        auto l_sub = v(99);

        // substitute var_index=3 with l(99) at depth 0
        const auto l_subbed = l_func->substitute(0, 3, l_sub->clone());

        // Expected: (0 1 2 100 3)
        // - l(0), l(1), l(2) stay unchanged (< 3)
        // - l(3) gets replaced with l(99) lifted by 1 (func binder) = l(100)
        // - l(4) decrements to l(3) (> 3)
        auto l_expected =
            f(a(a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()),
                  v(100)->clone()),
                v(3)->clone()));

        assert(l_subbed->equals(l_expected));
    }

    // func with nested func, testing locals < var_index preservation
    {
        // Body: λ.(0 2 3) with outer var_index=2
        // Inner l(0) is bound by inner lambda
        // Outer l(2) and l(3) are from outer scope
        auto l_inner_body = a(a(v(0)->clone(), v(2)->clone()), v(3)->clone());
        auto l_body = f(l_inner_body->clone());
        auto l_func = f(l_body->clone());
        auto l_sub = v(88);

        // substitute var_index=2 at depth=0
        const auto l_subbed = l_func->substitute(0, 2, l_sub->clone());

        // Expected: λ.λ.(0 89 2)
        // - Inner l(0) unchanged (bound by inner lambda)
        // - l(2) at depth 2 (due to 2 binders) should match var 2
        //   and be replaced with l(88) lifted by 2 = l(90)
        // - l(3) decrements to l(2)
        auto l_expected =
            f(f(a(a(v(0)->clone(), v(90)->clone()), v(2)->clone())));

        assert(l_subbed->equals(l_expected));
    }
}

void test_app_substitute()
{
    // app of locals, both are occurrances
    {
        auto l_lhs = v(0);
        auto l_rhs = v(0);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs =
            dynamic_cast<var*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 11);
        assert(l_subbed_rhs->m_index == 11);
    }

    // app of locals, lhs is an occurrance
    {
        auto l_lhs = v(0);
        auto l_rhs = v(1);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs =
            dynamic_cast<var*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 11);
        assert(l_subbed_rhs->m_index == 0);
    }

    // app of locals, rhs is an occurrance
    {
        auto l_lhs = v(1);
        auto l_rhs = v(0);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs =
            dynamic_cast<var*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 0);
        assert(l_subbed_rhs->m_index == 11);
    }

    // app of locals, neither are occurrances
    {
        auto l_lhs = v(1);
        auto l_rhs = v(1);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs =
            dynamic_cast<var*>(l_subbed_app->m_func.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->m_arg.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 0);
        assert(l_subbed_rhs->m_index == 0);
    }

    // app of funcs, both with occurrances
    {
        auto l_lhs = f(v(0)->clone());
        auto l_rhs = f(v(0)->clone());
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
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

        const var* l_lhs_var = dynamic_cast<var*>(l_subbed_lhs->m_body.get());

        // make sure body of lhs is a local
        assert(l_lhs_var != nullptr);

        const var* l_rhs_var = dynamic_cast<var*>(l_subbed_rhs->m_body.get());

        // make sure body of rhs is a local
        assert(l_rhs_var != nullptr);

        // make sure they have correct indices (lifted by 1 due to binders)
        assert(l_lhs_var->m_index == 12);
        assert(l_rhs_var->m_index == 12);
    }

    ////////////////////////////////////
    // Testing substitute with various binder depths
    ////////////////////////////////////

    // Test 1: substitute at depth 0, var_index 0 - basic substitution
    // (0 0) with var 0 -> l(5) should give (5 5)
    {
        auto l_app = a(v(0)->clone(), v(0)->clone());
        auto l_sub = v(5);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected = a(v(5)->clone(), v(5)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 2: substitute at depth 0, var_index 1 - substituting higher var
    // (1 2) with var 1 -> l(7) should give (7 1)
    {
        auto l_app = a(v(1)->clone(), v(2)->clone());
        auto l_sub = v(7);
        const auto l_subbed = l_app->substitute(0, 1, l_sub->clone());
        auto l_expected = a(v(7)->clone(), v(1)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 3: substitute at depth 1, var_index 0 - inside a binder context
    // (λ.0 λ.1) with var 0 at depth 1 -> l(3) should give (λ.5 λ.0)
    {
        auto l_app = a(f(v(0)->clone()), f(v(1)->clone()));
        auto l_sub = v(3);
        const auto l_subbed = l_app->substitute(1, 0, l_sub->clone());
        auto l_expected = a(f(v(5)->clone()), f(v(0)->clone()));
        assert(l_subbed->equals(l_expected));
    }

    // Test 5: substitute with complex expression (app as substitution)
    // (0 1) with var 0 -> (2 3) should give ((2 3) 0)
    {
        auto l_app = a(v(0)->clone(), v(1)->clone());
        auto l_sub = a(v(2)->clone(), v(3)->clone());
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected = a(a(v(2)->clone(), v(3)->clone()), v(0)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 6: substitute with func as substitution
    // (0 0) with var 0 -> λ.5 should give (λ.5 λ.5)
    {
        auto l_app = a(v(0)->clone(), v(0)->clone());
        auto l_sub = f(v(5)->clone());
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected = a(f(v(5)->clone()), f(v(5)->clone()));
        assert(l_subbed->equals(l_expected));
    }

    // Test 7: substitute at depth 2, var_index 1 - deeply nested
    // (λ.λ.1 λ.λ.2) with var 1 at depth 2 -> l(10) should give (λ.λ.14 λ.λ.1)
    {
        auto l_app = a(f(f(v(1)->clone())), f(f(v(2)->clone())));
        auto l_sub = v(10);
        const auto l_subbed = l_app->substitute(2, 1, l_sub->clone());
        auto l_expected = a(f(f(v(14)->clone())), f(f(v(1)->clone())));
        assert(l_subbed->equals(l_expected));
    }

    // Test 8: no matching variable - all vars higher than target
    // (2 3) with var 0 -> l(99) should give (1 2)
    {
        auto l_app = a(v(2)->clone(), v(3)->clone());
        auto l_sub = v(99);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected = a(v(1)->clone(), v(2)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 9: no matching variable - all vars lower than target
    // (0 1) with var 5 -> l(99) should give (0 1)
    {
        auto l_app = a(v(0)->clone(), v(1)->clone());
        auto l_sub = v(99);
        const auto l_subbed = l_app->substitute(0, 5, l_sub->clone());
        auto l_expected = a(v(0)->clone(), v(1)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 10: nested app with mixed locals
    // ((0 1) (2 0)) with var 0 -> l(8) should give ((8 0) (1 8))
    {
        auto l_inner1 = a(v(0)->clone(), v(1)->clone());
        auto l_inner2 = a(v(2)->clone(), v(0)->clone());
        auto l_app = a(l_inner1->clone(), l_inner2->clone());
        auto l_sub = v(8);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected =
            a(a(v(8)->clone(), v(0)->clone()), a(v(1)->clone(), v(8)->clone()));
        assert(l_subbed->equals(l_expected));
    }

    // Test 11: substitute higher var with lower vars present
    // (0 2) with var 2 -> l(9) should give (0 9)
    // l(0) stays unchanged (< 2), l(2) gets replaced
    {
        auto l_app = a(v(0)->clone(), v(2)->clone());
        auto l_sub = v(9);
        const auto l_subbed = l_app->substitute(0, 2, l_sub->clone());
        auto l_expected = a(v(0)->clone(), v(9)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 12: substitute with var_index=4, multiple locals below and above
    // ((0 1 2 3) (4 5 6)) with var 4 -> l(77) should give ((0 1 2 3) (77 4 5))
    {
        auto l_func_app =
            a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()), v(3)->clone());
        auto l_arg_app = a(a(v(4)->clone(), v(5)->clone()), v(6)->clone());
        auto l_app = a(l_func_app->clone(), l_arg_app->clone());
        auto l_sub = v(77);
        const auto l_subbed = l_app->substitute(0, 4, l_sub->clone());

        // Expected: ((0 1 2 3) (77 4 5))
        // l(0), l(1), l(2), l(3) unchanged (< 4)
        // l(4) replaced with l(77)
        // l(5), l(6) decremented to l(4), l(5)
        auto l_expected = a(
            a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()), v(3)->clone()),
            a(a(v(77)->clone(), v(4)->clone()), v(5)->clone()));
        assert(l_subbed->equals(l_expected));
    }

    // Test 13: nested app with funcs, var_index=2
    // (λ.(0 1 2) λ.(1 2 3)) with var 2 -> l(55)
    {
        auto l_lhs_body = a(a(v(0)->clone(), v(1)->clone()), v(2)->clone());
        auto l_rhs_body = a(a(v(1)->clone(), v(2)->clone()), v(3)->clone());
        auto l_app = a(f(l_lhs_body->clone()), f(l_rhs_body->clone()));
        auto l_sub = v(55);
        const auto l_subbed = l_app->substitute(0, 2, l_sub->clone());

        // In lhs func: depth becomes 1, so var 2 at that depth
        // l(0), l(1) unchanged (< 2)
        // l(2) matches var 2, replaced with l(55) lifted by 1 = l(56)
        // In rhs func: same logic
        // l(1) unchanged (< 2)
        // l(2) matches var 2, replaced with l(55) lifted by 1 = l(56)
        // l(3) decrements to l(2)
        auto l_expected =
            a(f(a(a(v(0)->clone(), v(1)->clone()), v(56)->clone())),
              f(a(a(v(1)->clone(), v(56)->clone()), v(2)->clone())));
        assert(l_subbed->equals(l_expected));
    }
}

void test_var_normalize()
{
    // local with var 0
    {
        auto l_expr = v(0);
        const auto l_reduced = l_expr->normalize();

        // cast the pointer
        const var* l_var = dynamic_cast<var*>(l_reduced.get());
        assert(l_var != nullptr);

        // make sure it has the same index
        assert(l_var->m_index == 0);
    }

    // local with var 1
    {
        auto l_expr = v(1);
        const auto l_reduced = l_expr->normalize();

        // cast the pointer
        const var* l_var = dynamic_cast<var*>(l_reduced.get());
        assert(l_var != nullptr);

        // make sure it has the same index
        assert(l_var->m_index == 1);
    }
}

void test_func_normalize()
{
    // func with body of a local
    {
        auto l_expr = f(v(0)->clone());
        const auto l_reduced = l_expr->normalize();

        // make sure still a func
        const auto* l_func = dynamic_cast<func*>(l_reduced.get());
        assert(l_func != nullptr);

        // get body
        const auto* l_body = dynamic_cast<var*>(l_func->m_body.get());
        assert(l_body != nullptr);

        // make sure body is still same thing
        assert(l_body->m_index == 0);
    }
}

void test_app_normalize()
{
    // app with lhs and rhs both locals
    {
        auto l_lhs = v(0);
        auto l_rhs = v(1);
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        const app* l_app = dynamic_cast<app*>(l_reduced.get());
        assert(l_app != nullptr);

        // lhs should be local
        const var* l_reduced_lhs = dynamic_cast<var*>(l_app->m_func.get());
        assert(l_reduced_lhs != nullptr);

        // rhs should be local
        const var* l_reduced_rhs = dynamic_cast<var*>(l_app->m_arg.get());
        assert(l_reduced_rhs != nullptr);

        // same on both (no reduction occurred)
        assert(l_reduced_lhs->m_index == 0);
        assert(l_reduced_rhs->m_index == 1);
    }

    // app with lhs func and rhs local
    {
        auto l_lhs = f(v(0)->clone());
        auto l_rhs = v(1);
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // make sure nothing changed
        // (beta reduction did not occur since rhs is local)
        assert(l_reduced->equals(v(1)));
    }

    // app with lhs func and rhs func
    {
        auto l_lhs = f(v(0)->clone());
        auto l_rhs = f(v(1)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // make sure beta-reduction occurred, with no lifting of indices
        assert(l_reduced->equals(l_rhs->clone()));
    }

    // app with lhs func (without occurrences of var 0) and rhs func
    {
        auto l_lhs = f(v(3)->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // make sure beta-reduction occurred, but no replacements.
        // other vars decremented by 1.
        assert(l_reduced->equals(v(2)->clone()));
    }

    // app with lhs (nested func with occurrences of var 0) and rhs func
    {
        auto l_lhs = f(f(v(0)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // make sure beta-reduction occurred, with replacement,
        // and a lifting of 1 level
        assert(l_reduced->equals(f(f(v(6)->clone()))->clone()));
    }

    // app with lhs (nested func without occurrences of var 0) and rhs func
    {
        auto l_lhs = f(f(v(3)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // make sure beta-reduction occurred, no replacements.
        // other vars decremented by 1.
        assert(l_reduced->equals(f(v(2)->clone())));
    }

    // app with lhs (app that doesnt reduce to func) and rhs func
    {
        auto l_lhs = a(v(3)->clone(), v(4)->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // make sure nothing changed
        // (both lhs and rhs were fully reduced already)
        assert(l_reduced->equals(l_expr->clone()));
    }

    // app with lhs (app with lhs (func without occurrances), rhs local)
    // and rhs func
    {
        auto l_lhs = a(f(v(3)->clone())->clone(), v(4)->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // lhs should have beta-reduced, but cannot consume rhs of app
        assert(l_reduced->equals(a(v(2), f(v(5)))));
    }

    // app with lhs (app with lhs (func without occurrances), rhs func)
    // and rhs func, where there are too many arguments supplied
    {
        auto l_lhs = a(f(v(3)->clone())->clone(), f(v(4)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // lhs of app should beta-reduce, but lhs is not capable of consuming 2
        // args. Thus NF is an application with LHS beta-reduced once.
        assert(l_reduced->equals(a(v(2)->clone(), f(v(5)->clone()))->clone()));
    }

    // app with lhs (app with lhs (func without occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        auto l_lhs =
            a(f(f(v(3)->clone())->clone())->clone(), f(v(4)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // should beta-reduce twice, consuming all args. No replacements, only
        // decrementing twice.
        assert(l_reduced->equals(v(1)->clone()));
    }

    // app with lhs (app with lhs (func WITH occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        auto l_lhs =
            a(f(f(v(0)->clone())->clone())->clone(), f(v(4)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // should beta-reduce twice, consuming all args.
        // becomes first arg, without lifting. (lifting occurred but was undone
        // by second arg)
        assert(l_reduced->equals(f(v(4)->clone())));
    }

    // app with lhs (app with lhs (func WITH occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        auto l_lhs =
            a(f(f(v(1)->clone())->clone())->clone(), f(v(4)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app
        const auto l_reduced = l_expr->normalize();

        // should beta-reduce twice, consuming all args.
        // becomes second arg, without lifting. (lifting occurred but was undone
        // by second arg)
        assert(l_reduced->equals(l_rhs->clone()));
    }
}

std::unique_ptr<expr> construct_program(
    std::list<std::unique_ptr<expr>>::const_iterator a_helpers_begin,
    std::list<std::unique_ptr<expr>>::const_iterator a_helpers_end,
    const std::unique_ptr<expr>& a_main_fn)
{
    // we will construct a tower of abstractions,
    // with the main function at the bottom.
    if(a_helpers_begin == a_helpers_end)
        return a_main_fn->clone();

    // construct an abstraction and recur
    return a(f(construct_program(std::next(a_helpers_begin), a_helpers_end,
                                 a_main_fn)),
             (*a_helpers_begin)->clone());
}

void generic_use_case_test()
{
    using namespace lambda;
    std::list<std::unique_ptr<expr>> l_helpers{};

    auto l = [&l_helpers](size_t a_local_index)
    { return v(l_helpers.size() + a_local_index); };

    auto g = [&l_helpers](size_t a_global_index) { return v(a_global_index); };

    // church booleans

    // true
    const auto TRUE = g(l_helpers.size());
    l_helpers.emplace_back(f(f(l(0))));
    // false
    const auto FALSE = g(l_helpers.size());
    l_helpers.emplace_back(f(f(l(1))));

    // test the church bools
    {
        // true case
        const auto l_true_case = f(l(10));

        // false case
        const auto l_false_case = f(l(11));

        // test the true case
        const auto l_true_case_main =
            a(a(TRUE->clone(), l_true_case->clone()), l_false_case->clone());

        // test the false case
        const auto l_false_case_main =
            a(a(FALSE->clone(), l_true_case->clone()), l_false_case->clone());

        // construct the program
        const auto l_true_program = construct_program(
            l_helpers.begin(), l_helpers.end(), l_true_case_main);

        // construct the program
        const auto l_false_program = construct_program(
            l_helpers.begin(), l_helpers.end(), l_false_case_main);

        // reduce the programs
        const auto l_true_reduced = l_true_program->normalize();
        const auto l_false_reduced = l_false_program->normalize();

        std::cout << "true reduced: ";
        l_true_reduced->print(std::cout);
        std::cout << std::endl;

        std::cout << "false reduced: ";
        l_false_reduced->print(std::cout);
        std::cout << std::endl;

        // test the reductions.
        // NOTE: after reduction of a program, the main function's locals BECOME
        // globals.
        assert(l_true_reduced->equals(f(g(10))));
        assert(l_false_reduced->equals(f(g(11))));
    }

    // add church numerals

    // 0
    const auto ZERO = g(l_helpers.size());
    l_helpers.emplace_back(f(f(l(1))));

    // succ
    const auto SUCC = g(l_helpers.size());
    l_helpers.emplace_back(f(f(f(a(l(1), a(a(l(0), l(1)), l(2)))))));

    // test succ church numerals
    {
        // construct 1 - 5

        const auto ONE = a(SUCC->clone(), ZERO->clone());
        const auto TWO = a(SUCC->clone(), ONE->clone());
        const auto THREE = a(SUCC->clone(), TWO->clone());
        const auto FOUR = a(SUCC->clone(), THREE->clone());
        const auto FIVE = a(SUCC->clone(), FOUR->clone());

        // construct the programs
        const auto ZERO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ZERO);
        const auto ONE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ONE);
        const auto TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), TWO);
        const auto THREE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), THREE);
        const auto FOUR_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), FOUR);
        const auto FIVE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), FIVE);

        // reduce zero
        const auto ZERO_REDUCED = ZERO_PROGRAM->normalize();
        std::cout << "zero reduced: ";
        ZERO_REDUCED->print(std::cout);
        std::cout << std::endl;

        // define one
        const auto ONE_REDUCED = ONE_PROGRAM->normalize();
        std::cout << "one reduced: ";
        ONE_REDUCED->print(std::cout);
        std::cout << std::endl;

        // define two
        const auto TWO_REDUCED = TWO_PROGRAM->normalize();
        std::cout << "two reduced: ";
        TWO_REDUCED->print(std::cout);
        std::cout << std::endl;

        // define three
        const auto THREE_REDUCED = THREE_PROGRAM->normalize();
        std::cout << "three reduced: ";
        THREE_REDUCED->print(std::cout);
        std::cout << std::endl;

        // define four
        const auto FOUR_REDUCED = FOUR_PROGRAM->normalize();
        std::cout << "four reduced: ";
        FOUR_REDUCED->print(std::cout);
        std::cout << std::endl;
        // define five
        const auto FIVE_REDUCED = FIVE_PROGRAM->normalize();
        std::cout << "five reduced: ";
        FIVE_REDUCED->print(std::cout);
        std::cout << std::endl;

        assert(ONE_REDUCED->equals(f(f(a(g(0), g(1))))));
        assert(TWO_REDUCED->equals(f(f(a(g(0), a(g(0), g(1)))))));
        assert(THREE_REDUCED->equals(f(f(a(g(0), a(g(0), a(g(0), g(1))))))));
        assert(FOUR_REDUCED->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))));
        assert(FIVE_REDUCED->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), a(g(0), g(1))))))))));
    }

    // add
    const auto ADD = g(l_helpers.size());
    l_helpers.emplace_back(
        f(f(f(f(a(a(l(0), l(2)), a(a(l(1), l(2)), l(3))))))));

    // test add church numerals
    {
        // construct 1 - 5

        const auto ONE = a(SUCC->clone(), ZERO->clone());
        const auto TWO = a(SUCC->clone(), ONE->clone());
        const auto THREE = a(SUCC->clone(), TWO->clone());
        const auto FOUR = a(SUCC->clone(), THREE->clone());
        const auto FIVE = a(SUCC->clone(), FOUR->clone());

        // add one and one
        const auto ADD_ONE_ONE = a(a(ADD->clone(), ONE->clone()), ONE->clone());

        // add one and two
        const auto ADD_ONE_TWO = a(a(ADD->clone(), ONE->clone()), TWO->clone());

        // add two and two
        const auto ADD_TWO_TWO = a(a(ADD->clone(), TWO->clone()), TWO->clone());

        // add three and two
        const auto ADD_THREE_TWO =
            a(a(ADD->clone(), THREE->clone()), TWO->clone());

        // add five and five
        const auto ADD_FIVE_FIVE =
            a(a(ADD->clone(), FIVE->clone()), FIVE->clone());

        const auto ADD_ONE_ONE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ADD_ONE_ONE);
        const auto ADD_ONE_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ADD_ONE_TWO);
        const auto ADD_TWO_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ADD_TWO_TWO);
        const auto ADD_THREE_TWO_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), ADD_THREE_TWO);
        const auto ADD_FIVE_FIVE_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), ADD_FIVE_FIVE);

        // reduce the programs
        const auto ADD_ONE_ONE_REDUCED = ADD_ONE_ONE_PROGRAM->normalize();
        const auto ADD_ONE_TWO_REDUCED = ADD_ONE_TWO_PROGRAM->normalize();
        const auto ADD_TWO_TWO_REDUCED = ADD_TWO_TWO_PROGRAM->normalize();
        const auto ADD_THREE_TWO_REDUCED = ADD_THREE_TWO_PROGRAM->normalize();
        const auto ADD_FIVE_FIVE_REDUCED = ADD_FIVE_FIVE_PROGRAM->normalize();

        std::cout << "add one one: ";
        ADD_ONE_ONE_REDUCED->print(std::cout);
        std::cout << std::endl;

        std::cout << "add one two: ";
        ADD_ONE_TWO_REDUCED->print(std::cout);
        std::cout << std::endl;

        std::cout << "add two two: ";
        ADD_TWO_TWO_REDUCED->print(std::cout);
        std::cout << std::endl;

        std::cout << "add three two: ";
        ADD_THREE_TWO_REDUCED->print(std::cout);
        std::cout << std::endl;

        std::cout << "add five five: ";
        ADD_FIVE_FIVE_REDUCED->print(std::cout);
        std::cout << std::endl;

        // assertions
        assert(ADD_ONE_ONE_REDUCED->equals(f(f(a(g(0), a(g(0), g(1)))))));
        assert(
            ADD_ONE_TWO_REDUCED->equals(f(f(a(g(0), a(g(0), a(g(0), g(1))))))));
        assert(ADD_TWO_TWO_REDUCED->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))));
        assert(ADD_THREE_TWO_REDUCED->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), a(g(0), g(1))))))))));
        assert(ADD_FIVE_FIVE_REDUCED->equals(f(f(a(
            g(0),
            a(g(0),
              a(g(0),
                a(g(0),
                  a(g(0),
                    a(g(0), a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))))))))));
    }
}

void lambda_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_var_constructor);
    TEST(test_func_constructor);
    TEST(test_app_constructor);

    TEST(test_var_equals);
    TEST(test_func_equals);
    TEST(test_app_equals);

    TEST(test_var_lift);
    TEST(test_func_lift);
    TEST(test_app_lift);

    TEST(test_var_substitute);
    TEST(test_func_substitute);
    TEST(test_app_substitute);

    TEST(test_var_normalize);
    TEST(test_func_normalize);
    TEST(test_app_normalize);

    TEST(generic_use_case_test);
}

#endif
