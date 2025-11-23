#include "../include/predef.hpp"
#include <limits>

// local variable macro
#define L(x) v(a_binder_depth + x)

using namespace lambda;

// helper to apply something to a bunch of expressions
template <typename... ARGS>
std::unique_ptr<lambda::expr> a_twr(std::unique_ptr<lambda::expr>&& a_func,
                                    std::unique_ptr<lambda::expr>&& a_arg,
                                    ARGS&&... a_rest)
{
    // start with function
    auto l_result = a(std::move(a_func), std::move(a_arg));
    if constexpr(sizeof...(ARGS) == 0)
        return l_result;
    else
        return a_twr(std::move(l_result), std::move(a_rest)...);
}

namespace dml
{
namespace predef
{

// Y combinator
std::unique_ptr<lambda::expr> y_combinator(size_t a_binder_depth)
{
    // Y ≡ λf. (λx. f (x x)) (λx. f (x x))
    return f(a(f(a(L(0), a(L(1), L(1)))), f(a(L(0), a(L(1), L(1))))));
}

// church true
std::unique_ptr<lambda::expr> church_true(size_t a_binder_depth)
{
    return f(f(L(0)));
}

// church false
std::unique_ptr<lambda::expr> church_false(size_t a_binder_depth)
{
    return f(f(L(1)));
}

// church not
std::unique_ptr<lambda::expr> church_not(size_t a_binder_depth)
{
    return f(a(a(L(0), church_false(a_binder_depth + 1)),
               church_true(a_binder_depth + 1)));
}

// church and
std::unique_ptr<lambda::expr> church_and(size_t a_binder_depth)
{
    return f(f(a(a(L(0), L(1)), church_false(a_binder_depth + 2))));
}

// church or
std::unique_ptr<lambda::expr> church_or(size_t a_binder_depth)
{
    return f(f(a(a(L(0), church_true(a_binder_depth + 2)), L(1))));
}

// church xor
std::unique_ptr<lambda::expr> church_xor(size_t a_binder_depth)
{
    return f(f(a(a(L(0), a(church_not(a_binder_depth + 2), L(1))), L(1))));
}

// full adder
std::unique_ptr<lambda::expr> church_full_adder(size_t a_binder_depth)
{
    // NOTE: in sum and carry exprs, a,b,c are **captured** from the final
    // expr which introduces them.

    // bitSum   ≡
    //              XOR a (XOR b c)
    auto l_bit_sum = a_twr(church_xor(a_binder_depth + 3), L(0),
                           a_twr(church_xor(a_binder_depth + 3), L(1), L(2)));

    // bitCarry ≡
    //              OR (AND a b)
    //                 (OR (AND a c)
    //                     (AND b c))
    auto l_bit_carry =
        a_twr(church_or(a_binder_depth + 3),
              a_twr(church_and(a_binder_depth + 3), L(0), L(1)),
              a_twr(church_or(a_binder_depth + 3),
                    a_twr(church_and(a_binder_depth + 3), L(0), L(2)),
                    a_twr(church_and(a_binder_depth + 3), L(1), L(2))));

    // BIT_ADD3 ≡ λa. λb. λc.
    //              PAIR (bitSum a b c) (bitCarry a b c)
    return f(f(f(a_twr(church_pair(a_binder_depth + 3), l_bit_sum->clone(),
                       l_bit_carry->clone()))));
}

// full subtractor
std::unique_ptr<lambda::expr> church_full_subtractor(size_t a_binder_depth)
{
    return f(  // X
        f(     // Y
            f( // Bin
                a_twr(church_pair(a_binder_depth + 3),
                      a_twr( // diff
                          church_xor(a_binder_depth + 3),
                          L(0), // X
                          a_twr(church_xor(a_binder_depth + 3),
                                L(1), // Y
                                L(2)  // Bin
                                )),
                      a_twr( // Bout
                          church_or(a_binder_depth + 3),
                          a_twr(church_and(a_binder_depth + 3),
                                a(church_not(a_binder_depth + 3), L(0)), L(1)),
                          a_twr(church_or(a_binder_depth + 3),
                                a_twr(church_and(a_binder_depth + 3),
                                      a(church_not(a_binder_depth + 3),
                                        L(0) // X
                                        ),
                                      L(2) // Bin
                                      ),
                                a_twr(church_and(a_binder_depth + 3),
                                      L(1), // Y
                                      L(2)  // Bin
                                      )))

                          ))));
}

// church zero
std::unique_ptr<lambda::expr> church_zero(size_t a_binder_depth)
{
    return f(f(L(1)));
}

// church succ
std::unique_ptr<lambda::expr> church_succ(size_t a_binder_depth)
{
    return f(f(f(a(L(1), a(a(L(0), L(1)), L(2))))));
}

// church is_zero
std::unique_ptr<lambda::expr> church_is_zero(size_t a_binder_depth)
{
    return f(a(a(L(0), f(church_false(a_binder_depth + 2))),
               church_true(a_binder_depth + 1)));
}

// church pair
std::unique_ptr<lambda::expr> church_pair(size_t a_binder_depth)
{
    return f(f(f(a(a(L(2), L(0)), L(1)))));
}

// church fst
std::unique_ptr<lambda::expr> church_fst(size_t a_binder_depth)
{
    return f(a(L(0), church_true(a_binder_depth + 1)));
}

// church snd
std::unique_ptr<lambda::expr> church_snd(size_t a_binder_depth)
{
    return f(a(L(0), church_false(a_binder_depth + 1)));
}

// church pred (predecessor)
// λn. fst (n (λp. pair (snd p) (succ (snd p))) (pair 0 0))
std::unique_ptr<lambda::expr> church_pred(size_t a_binder_depth)
{
    return f(a(
        church_fst(a_binder_depth + 1),
        a(a(L(0), // n
            f(a(a(church_pair(a_binder_depth + 2),
                  a(church_snd(a_binder_depth + 2), L(1))), // p
                a(church_succ(a_binder_depth + 2),
                  a(church_snd(a_binder_depth + 2), L(1)))))), // p
          a(a(church_pair(a_binder_depth + 1), church_zero(a_binder_depth + 1)),
            church_zero(a_binder_depth + 1)))));
}

// church sub (subtraction)
// λn.λm. n pred m
std::unique_ptr<lambda::expr> church_sub(size_t a_binder_depth)
{
    return f(f(a(a(L(1), // n
                   church_pred(a_binder_depth + 2)),
                 L(0)))); // m
}

// church less_than
// λn.λm. not (is_zero (sub n m))
std::unique_ptr<lambda::expr> church_less_than(size_t a_binder_depth)
{
    return f(f(a(church_not(a_binder_depth + 2),
                 a(church_is_zero(a_binder_depth + 2),
                   a(a(church_sub(a_binder_depth + 2), L(1)), // n
                     L(0))))));                               // m
}

// some constructor for Maybe
std::unique_ptr<lambda::expr> some(size_t a_binder_depth)
{
    return f(  // x
        f(     // somecase
            f( // nonecase
                a(L(1), L(0)))));
}

// none constructor for Maybe
std::unique_ptr<lambda::expr> none(size_t a_binder_depth)
{
    return f( // somecase
        f(    // nonecase
            L(1)));
}

// scott nil
std::unique_ptr<lambda::expr> scott_nil(size_t a_binder_depth)
{
    // λnilCase. λconsCase. nilCase
    return f(f(L(0)));
}

// scott cons
std::unique_ptr<lambda::expr> scott_cons(size_t a_binder_depth)
{
    // λh. λt. λnilCase. λconsCase. consCase h t
    return f(f(f(f(a(a(L(3), L(0)), L(1))))));
}

// binary zero
std::unique_ptr<lambda::expr> binary_zero(size_t a_binder_depth)
{
    return scott_nil(a_binder_depth);
}

// binary is zero
std::unique_ptr<lambda::expr> binary_is_zero(size_t a_binder_depth)
{
    // λn. n TRUE (λh. λt. FALSE)
    // If n is NIL → returns TRUE
    // If n is CONS h t → returns FALSE
    return f(
        a(a(L(0),                                   // n
            church_true(a_binder_depth + 1)),       // nilCase: TRUE
          f(f(church_false(a_binder_depth + 3))))); // consCase: λh.λt.FALSE
}

// binary succ
std::unique_ptr<lambda::expr> binary_succ(size_t a_binder_depth)
{
    // SUCC_LOG ≡
    //   Y (λrec. λn.
    //     n
    //       (CONS BIT1 NIL)                 ; [] → [1]
    //       (λh. λt.
    //          h
    //            (CONS BIT0 (rec t))        ; h = 1 → 0 :: succ t
    //            (CONS BIT1 t)))            ; h = 0 → 1 :: t
    return a(y_combinator(a_binder_depth),
             f(                // self
                 f(            // n
                     a(a(L(1), // n
                         a(a(scott_cons(a_binder_depth + 2),
                             church_true(a_binder_depth + 2)),
                           scott_nil(a_binder_depth + 2))),
                       f(                // h
                           f(            // t
                               a(a(L(2), // h
                                   a(a(scott_cons(a_binder_depth + 4),
                                       church_false(a_binder_depth + 4)),
                                     a(L(0), // self
                                       L(3)  // t
                                       ))),
                                 a(a(scott_cons(a_binder_depth + 4),
                                     church_true(a_binder_depth + 4)),
                                   L(3) // t
                                   ))))))));
}

// binary canonicalize
std::unique_ptr<lambda::expr> binary_canonicalize(size_t a_binder_depth)
{
    return a(
        y_combinator(a_binder_depth),
        f(     // self
            f( // x
                a_twr(
                    L(1), // x
                    scott_nil(a_binder_depth + 2),
                    f(                  // xh
                        f(              // xt
                            a_twr(L(2), // xh
                                  a_twr(scott_cons(a_binder_depth + 4),
                                        L(2),   // xh
                                        a(L(0), // self
                                          L(3)  // xt
                                          )),
                                  a_twr(a(L(0), // self
                                          L(3)  // xt
                                          ),
                                        scott_nil(a_binder_depth + 4),
                                        f(     // yh
                                            f( // yt
                                                a_twr(scott_cons(
                                                          a_binder_depth + 6),
                                                      L(2), // xh
                                                      a_twr(scott_cons(
                                                                a_binder_depth +
                                                                6),
                                                            L(4), // yh
                                                            L(5)  // yt
                                                            ))))))))))));
}

// binary pred
std::unique_ptr<lambda::expr> binary_pred(size_t a_binder_depth)
{
    //     PRED_LOG ≡
    //         Y (λrec. λn.
    //              n
    //                NIL                             ; pred 0 = 0 (saturating)
    //                (λh. λt.
    //                   h
    //                     (CONS BIT0 t)              ; h = 1 → 0 :: t
    //                     (CONS BIT1 (rec t))))      ; h = 0 → 1 :: pred t
    return a(y_combinator(a_binder_depth),
             f(                // rec
                 f(            // n
                     a(a(L(1), // n
                         scott_nil(a_binder_depth + 2)),
                       f(     // h
                           f( // t
                               a(a(L(2), a(a(scott_cons(a_binder_depth + 4),
                                             church_false(a_binder_depth + 4)),
                                           L(3) // t
                                           )),
                                 a(a(scott_cons(a_binder_depth + 4),
                                     church_true(a_binder_depth + 4)),
                                   a(L(0), // rec
                                     L(3)  // t
                                     )))))))));
}

std::unique_ptr<lambda::expr> binary_add(size_t a_binder_depth)
{
    return a(
        y_combinator(a_binder_depth),
        f(             // self
            f(         // x
                f(     // y
                    f( // c
                        a_twr(
                            L(1),             // x
                            a_twr(L(3),       // c
                                  a_twr(L(0), // self
                                        a_twr(scott_cons(a_binder_depth + 4),
                                              church_true(a_binder_depth + 4),
                                              scott_nil(a_binder_depth + 4)),
                                        L(2), // y
                                        church_false(a_binder_depth + 4)),
                                  L(2) // y
                                  ),
                            f(     // xh
                                f( // xt
                                    a_twr(
                                        L(2), // y
                                        a_twr(
                                            L(3), // c
                                            a_twr(
                                                L(0), // self
                                                L(1), // x
                                                a_twr(scott_cons(
                                                          a_binder_depth + 6),
                                                      church_true(
                                                          a_binder_depth + 6),
                                                      scott_nil(a_binder_depth +
                                                                6)),
                                                church_false(a_binder_depth +
                                                             6)),
                                            L(1) // x
                                            ),
                                        f(     // yh
                                            f( // yt
                                                a(a_twr(church_full_adder(
                                                            a_binder_depth + 8),
                                                        L(4), // xh
                                                        L(6), // yh
                                                        L(3)  // c
                                                        ),
                                                  f(     // sum
                                                      f( // cout
                                                          a_twr(
                                                              scott_cons(
                                                                  a_binder_depth +
                                                                  10),
                                                              L(8), // sum
                                                              a_twr(
                                                                  L(0), // self
                                                                  L(5), // xt
                                                                  L(7), // yt
                                                                  L(9)  // cout
                                                                  ))))))))))))))));
}

// // binary sub
// std::unique_ptr<lambda::expr> binary_try_subtract(size_t a_binder_depth)
// {
//     return a(
//         y_combinator(a_binder_depth),
//         f(             // self
//             f(         // x
//                 f(     // y
//                     f( // b
//                         a_twr(
//                             L(1), // x
//                             scott_nil(a_binder_depth + 4),
//                             f(     // xh
//                                 f( // xt
//                                     a_twr(
//                                         L(2),            // y
//                                         a_twr(L(3),      // b
//                                               a_twr(     // b is true
//                                                   L(0),  // self
//                                                   L(1),  // x
//                                                   a_twr( // cons true nil
//                                                       scott_cons(
//                                                           a_binder_depth +
//                                                           6),
//                                                       church_true(
//                                                           a_binder_depth +
//                                                           6),
//                                                       scott_nil(a_binder_depth
//                                                       +
//                                                                 6)),
//                                                   church_false(a_binder_depth
//                                                   +
//                                                                6)),
//                                               L(1) // x, if y=nil and b=false
//                                               ),
//                                         f(     // yh
//                                             f( // yt
//                                                 a(a_twr(church_full_subtractor(
//                                                             a_binder_depth +
//                                                             8),
//                                                         L(4), // xh
//                                                         L(6), // yh
//                                                         L(3)  // b
//                                                         ),
//                                                   f(     // diff
//                                                       f( // bout
//                                                           a_twr(
//                                                               scott_cons(
//                                                                   a_binder_depth
//                                                                   + 10),
//                                                               L(8), // diff
//                                                               a_twr(
//                                                                   L(0), //
//                                                                   self L(5),
//                                                                   // xt L(7),
//                                                                   // yt L(9)
//                                                                   // bout
//                                                                   ))))))))))))))));
// }

} // namespace predef
} // namespace dml

#ifdef UNIT_TEST

#include "test_utils.hpp"

// Helper to wrap expression with n lambdas
std::unique_ptr<lambda::expr> wrap_lambdas(std::unique_ptr<lambda::expr>&& expr,
                                           size_t n)
{
    for(size_t i = 0; i < n; ++i)
    {
        expr = f(std::move(expr));
    }
    return std::move(expr);
}

void test_y_combinator()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_y = y_combinator(depth);
        auto expected = f(a(f(a(v(depth), a(v(depth + 1), v(depth + 1)))),
                            f(a(v(depth), a(v(depth + 1), v(depth + 1))))));
        assert(l_y->equals(expected));

        // apply Y to v(10) (MUST have step limit or will run forever)
        // this should build a tower of applications of v(10) basically
        auto l_y_of_v10_result =
            wrap_lambdas(a(l_y->clone(), v(depth + 10)), depth)
                ->normalize(
                    4, std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>& a_expr)
                        -> void { /* std::cout << *a_expr << std::endl;*/ });
        assert(l_y_of_v10_result.m_expr->equals(
            wrap_lambdas(a(v(depth + 10),
                           a(v(depth + 10),
                             a(v(depth + 10),
                               a(f(a(v(depth + 11), a(v(depth), v(depth)))),
                                 f(a(v(depth + 11), a(v(depth), v(depth)))))))),
                         depth)));
        {
            // try something that terminates
            // take a number, and if it is zero, return v(22), else decrement
            // F ≡ λself. λn.
            //     if isZero n
            //         then v(22)
            //         else self (pred n)
            auto l_F =
                f(f(a(a(a(church_is_zero(depth + 2), v(depth + 1)),
                        v(depth + 2 + 22)),
                      a(v(depth), a(church_pred(depth + 2), v(depth + 1))))));

            // G ≡ Y F
            auto l_G = a(l_y->clone(), l_F->clone());

            // construct number 10
            auto l_ten = v(depth + 1);
            for(size_t i = 0; i < 10; ++i)
                l_ten = a(v(depth), std::move(l_ten));
            l_ten = f(f(std::move(l_ten)));

            // apply G to 10 (should terminate)
            auto l_G_result =
                wrap_lambdas(a(l_G->clone(), l_ten->clone()), depth)
                    ->normalize(
                        std::numeric_limits<size_t>::max(),
                        std::numeric_limits<size_t>::max(),
                        [](const std::unique_ptr<lambda::expr>& a_expr)
                            -> void { /* std::cout << *a_expr << std::endl;*/ });
            assert(
                l_G_result.m_expr->equals(wrap_lambdas(v(depth + 22), depth)));
        }

        // try computing a tower of applications of size n
        {
            // F ≡ λself. λf. λx. λn.
            //     if isZero n
            //         then x
            //         else f (self f x (pred n))
            auto l_F = f(f(f(
                f(a(a(a(church_is_zero(depth + 4), v(depth + 3)), v(depth + 2)),
                    a(v(depth + 1),
                      a(a(a(v(depth), v(depth + 1)), v(depth + 2)),
                        a(church_pred(depth + 4), v(depth + 3)))))))));

            // G ≡ Y F
            auto l_G = a(l_y->clone(), l_F->clone());

            // construct number 5
            auto l_five = v(depth + 1);
            for(size_t i = 0; i < 5; ++i)
                l_five = a(v(depth), std::move(l_five));
            l_five = f(f(std::move(l_five)));

            // apply G to v(33), v(44), and 5
            auto l_G_result =
                wrap_lambdas(a(a(a(l_G->clone(), v(depth + 33)), v(depth + 44)),
                               l_five->clone()),
                             depth)
                    ->normalize(
                        std::numeric_limits<size_t>::max(),
                        std::numeric_limits<size_t>::max(),
                        [](const std::unique_ptr<lambda::expr>& a_expr)
                            -> void { /* std::cout << *a_expr << std::endl;*/ });
            assert(l_G_result.m_expr->equals(wrap_lambdas(
                a(v(33 + depth),
                  a(v(33 + depth),
                    a(v(33 + depth),
                      a(v(33 + depth), a(v(33 + depth), v(44 + depth)))))),
                depth)));
        }
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_true()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_true = church_true(depth);
        auto expected = f(f(v(depth)));
        assert(l_true->equals(expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_false()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_false = church_false(depth);
        auto expected = f(f(v(depth + 1)));
        assert(l_false->equals(expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_not()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_not = church_not(depth);
        auto expected =
            f(a(a(v(depth), f(f(v(depth + 2)))), f(f(v(depth + 1)))));
        assert(l_not->equals(expected));
        auto l_true = church_true(depth);
        auto l_not_true =
            wrap_lambdas(a(l_not->clone(), l_true->clone()), depth)
                ->normalize();
        assert(l_not_true.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
        auto l_false = church_false(depth);
        auto l_not_false =
            wrap_lambdas(a(l_not->clone(), l_false->clone()), depth)
                ->normalize();
        assert(l_not_false.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_and()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_and = church_and(depth);
        auto expected = f(f(a(a(v(depth), v(depth + 1)), f(f(v(depth + 3))))));
        assert(l_and->equals(expected));
        auto l_true = church_true(depth);
        auto l_false = church_false(depth);
        auto l_and_true_true =
            wrap_lambdas(a(a(l_and->clone(), l_true->clone()), l_true->clone()),
                         depth)
                ->normalize();
        assert(l_and_true_true.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_and_false_true =
            wrap_lambdas(
                a(a(l_and->clone(), l_false->clone()), l_true->clone()), depth)
                ->normalize();
        assert(l_and_false_true.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
        auto l_and_true_false =
            wrap_lambdas(
                a(a(l_and->clone(), l_true->clone()), l_false->clone()), depth)
                ->normalize();
        assert(l_and_true_false.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
        auto l_and_false_false =
            wrap_lambdas(
                a(a(l_and->clone(), l_false->clone()), l_false->clone()), depth)
                ->normalize();
        assert(l_and_false_false.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_or()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_or = church_or(depth);
        auto expected = f(f(a(a(v(depth), f(f(v(depth + 2)))), v(depth + 1))));
        assert(l_or->equals(expected));
        auto l_true = church_true(depth);
        auto l_false = church_false(depth);
        auto l_or_true_true =
            wrap_lambdas(a(a(l_or->clone(), l_true->clone()), l_true->clone()),
                         depth)
                ->normalize();
        assert(l_or_true_true.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_or_true_false =
            wrap_lambdas(a(a(l_or->clone(), l_true->clone()), l_false->clone()),
                         depth)
                ->normalize();
        assert(l_or_true_false.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_or_false_true =
            wrap_lambdas(a(a(l_or->clone(), l_false->clone()), l_true->clone()),
                         depth)
                ->normalize();
        assert(l_or_false_true.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_or_false_false =
            wrap_lambdas(
                a(a(l_or->clone(), l_false->clone()), l_false->clone()), depth)
                ->normalize();
        assert(l_or_false_false.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_xor()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_xor = church_xor(depth);
        auto expected =
            f(f(a(a(v(depth), a(church_not(depth + 2), v(depth + 1))),
                  v(depth + 1))));
        assert(l_xor->equals(expected));

        // true
        auto l_true = church_true(depth);

        // false
        auto l_false = church_false(depth);

        // Test xor(true, true) = false
        auto l_xor_true_true =
            wrap_lambdas(a(a(l_xor->clone(), l_true->clone()), l_true->clone()),
                         depth)
                ->normalize();
        assert(l_xor_true_true.m_expr->equals(
            wrap_lambdas(l_false->clone(), depth)));

        // Test xor(true, false) = true
        auto l_xor_true_false =
            wrap_lambdas(
                a(a(l_xor->clone(), l_true->clone()), l_false->clone()), depth)
                ->normalize();
        assert(l_xor_true_false.m_expr->equals(
            wrap_lambdas(l_true->clone(), depth)));

        // Test xor(false, true) = true
        auto l_xor_false_true =
            wrap_lambdas(
                a(a(l_xor->clone(), l_false->clone()), l_true->clone()), depth)
                ->normalize();
        assert(l_xor_false_true.m_expr->equals(
            wrap_lambdas(l_true->clone(), depth)));

        // Test xor(false, false) = false
        auto l_xor_false_false =
            wrap_lambdas(
                a(a(l_xor->clone(), l_false->clone()), l_false->clone()), depth)
                ->normalize();
        assert(l_xor_false_false.m_expr->equals(
            wrap_lambdas(l_false->clone(), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_zero()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_zero = church_zero(depth);
        auto expected = f(f(v(depth + 1)));
        assert(l_zero->equals(expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_succ()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_succ = church_succ(depth);
        auto expected = f(
            f(f(a(v(depth + 1), a(a(v(depth), v(depth + 1)), v(depth + 2))))));
        assert(l_succ->equals(expected));
        auto l_zero = church_zero(depth);
        auto l_one = wrap_lambdas(a(l_succ->clone(), l_zero->clone()), depth)
                         ->normalize();
        auto expected_one = f(f(a(v(depth), v(depth + 1))));
        assert(
            l_one.m_expr->equals(wrap_lambdas(std::move(expected_one), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_is_zero()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_is_zero = church_is_zero(depth);
        auto expected =
            f(a(a(v(depth), f(f(f(v(depth + 3))))), f(f(v(depth + 1)))));
        assert(l_is_zero->equals(expected));
        auto l_zero = church_zero(depth);
        auto l_is_zero_zero =
            wrap_lambdas(a(l_is_zero->clone(), l_zero->clone()), depth)
                ->normalize();
        assert(l_is_zero_zero.m_expr->equals(
            wrap_lambdas(church_true(depth), depth)));
        auto l_one = f(f(a(v(depth), v(depth + 1))));
        auto l_is_zero_one =
            wrap_lambdas(a(l_is_zero->clone(), l_one->clone()), depth)
                ->normalize();
        assert(l_is_zero_one.m_expr->equals(
            wrap_lambdas(church_false(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_pair()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_pair = church_pair(depth);
        auto expected = f(f(f(a(a(v(depth + 2), v(depth)), v(depth + 1)))));
        assert(l_pair->equals(expected));
        auto l_first = wrap_lambdas(a(a(a(l_pair->clone(), church_false(depth)),
                                        church_true(depth)),
                                      church_true(depth)),
                                    depth)
                           ->normalize();
        assert(
            l_first.m_expr->equals(wrap_lambdas(church_false(depth), depth)));
        auto l_second =
            wrap_lambdas(a(a(a(l_pair->clone(), church_false(depth)),
                             church_true(depth)),
                           church_false(depth)),
                         depth)
                ->normalize();
        assert(
            l_second.m_expr->equals(wrap_lambdas(church_true(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_fst()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_fst = church_fst(depth);
        auto expected = f(a(v(depth), church_true(depth + 1)));
        assert(l_fst->equals(expected));
        // Create a pair (false, true)
        auto l_pair_ft = church_pair(depth);
        auto l_pair_created =
            a(a(l_pair_ft->clone(), church_false(depth)), church_true(depth));
        // Apply fst to the pair
        auto l_result =
            wrap_lambdas(a(l_fst->clone(), std::move(l_pair_created)), depth)
                ->normalize();
        assert(
            l_result.m_expr->equals(wrap_lambdas(church_false(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_snd()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_snd = church_snd(depth);
        auto expected = f(a(v(depth), church_false(depth + 1)));
        assert(l_snd->equals(expected));
        // Create a pair (false, true)
        auto l_pair_ft = church_pair(depth);
        auto l_pair_created =
            a(a(l_pair_ft->clone(), church_false(depth)), church_true(depth));
        // Apply snd to the pair
        auto l_result =
            wrap_lambdas(a(l_snd->clone(), std::move(l_pair_created)), depth)
                ->normalize();
        assert(
            l_result.m_expr->equals(wrap_lambdas(church_true(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_pred()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        // Test pred(0) = 0
        auto l_zero = church_zero(depth);
        auto l_pred = church_pred(depth);
        auto l_pred_zero =
            wrap_lambdas(a(l_pred->clone(), l_zero->clone()), depth)
                ->normalize();
        assert(l_pred_zero.m_expr->equals(
            wrap_lambdas(church_zero(depth), depth)));

        // Test pred(1) = 0
        auto l_one = f(f(a(v(depth), v(depth + 1))));
        auto l_pred_one =
            wrap_lambdas(a(l_pred->clone(), l_one->clone()), depth)
                ->normalize();
        assert(
            l_pred_one.m_expr->equals(wrap_lambdas(church_zero(depth), depth)));

        // Test pred(2) = 1
        auto l_two = f(f(a(v(depth), a(v(depth), v(depth + 1)))));
        auto l_pred_two =
            wrap_lambdas(a(l_pred->clone(), l_two->clone()), depth)
                ->normalize();
        assert(l_pred_two.m_expr->equals(wrap_lambdas(l_one->clone(), depth)));

        // Test pred(3) = 2
        auto l_three =
            f(f(a(v(depth), a(v(depth), a(v(depth), v(depth + 1))))));
        auto l_pred_three =
            wrap_lambdas(a(l_pred->clone(), l_three->clone()), depth)
                ->normalize();
        assert(
            l_pred_three.m_expr->equals(wrap_lambdas(l_two->clone(), depth)));

        // Test pred(4) = 3
        auto l_four = f(f(
            a(v(depth), a(v(depth), a(v(depth), a(v(depth), v(depth + 1)))))));
        auto l_pred_four =
            wrap_lambdas(a(l_pred->clone(), l_four->clone()), depth)
                ->normalize();
        assert(
            l_pred_four.m_expr->equals(wrap_lambdas(l_three->clone(), depth)));

        // Test pred(5) = 4
        auto l_five = f(f(a(
            v(depth),
            a(v(depth), a(v(depth), a(v(depth), a(v(depth), v(depth + 1))))))));
        auto l_pred_five =
            wrap_lambdas(a(l_pred->clone(), l_five->clone()), depth)
                ->normalize();
        assert(
            l_pred_five.m_expr->equals(wrap_lambdas(l_four->clone(), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_sub()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_sub = church_sub(depth);
        auto l_zero = church_zero(depth);
        auto l_one = f(f(a(v(depth), v(depth + 1))));
        auto l_two = f(f(a(v(depth), a(v(depth), v(depth + 1)))));
        auto l_three =
            f(f(a(v(depth), a(v(depth), a(v(depth), v(depth + 1))))));

        // Test sub(3, 1) = 2
        auto l_sub_3_1 =
            wrap_lambdas(a(a(l_sub->clone(), l_three->clone()), l_one->clone()),
                         depth)
                ->normalize();
        assert(l_sub_3_1.m_expr->equals(wrap_lambdas(l_two->clone(), depth)));

        // Test sub(3, 2) = 1
        auto l_sub_3_2 =
            wrap_lambdas(a(a(l_sub->clone(), l_three->clone()), l_two->clone()),
                         depth)
                ->normalize();
        assert(l_sub_3_2.m_expr->equals(wrap_lambdas(l_one->clone(), depth)));

        // Test sub(2, 2) = 0
        auto l_sub_2_2 =
            wrap_lambdas(a(a(l_sub->clone(), l_two->clone()), l_two->clone()),
                         depth)
                ->normalize();
        assert(
            l_sub_2_2.m_expr->equals(wrap_lambdas(church_zero(depth), depth)));

        // Test sub(1, 2) = 0 (clamped at 0)
        auto l_sub_1_2 =
            wrap_lambdas(a(a(l_sub->clone(), l_one->clone()), l_two->clone()),
                         depth)
                ->normalize();
        assert(
            l_sub_1_2.m_expr->equals(wrap_lambdas(church_zero(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_less_than()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_less_than = church_less_than(depth);
        auto l_zero = church_zero(depth);
        auto l_one = f(f(a(v(depth), v(depth + 1))));
        auto l_two = f(f(a(v(depth), a(v(depth), v(depth + 1)))));

        // Test less_than(1, 2) = true
        auto l_lt_1_2 = wrap_lambdas(a(a(l_less_than->clone(), l_one->clone()),
                                       l_two->clone()),
                                     depth)
                            ->normalize();
        assert(
            l_lt_1_2.m_expr->equals(wrap_lambdas(church_true(depth), depth)));

        // Test less_than(2, 1) = false
        auto l_lt_2_1 = wrap_lambdas(a(a(l_less_than->clone(), l_two->clone()),
                                       l_one->clone()),
                                     depth)
                            ->normalize();
        assert(
            l_lt_2_1.m_expr->equals(wrap_lambdas(church_false(depth), depth)));

        // Test less_than(2, 2) = false
        auto l_lt_2_2 = wrap_lambdas(a(a(l_less_than->clone(), l_two->clone()),
                                       l_two->clone()),
                                     depth)
                            ->normalize();
        assert(
            l_lt_2_2.m_expr->equals(wrap_lambdas(church_false(depth), depth)));

        // Test less_than(0, 1) = true
        auto l_lt_0_1 = wrap_lambdas(a(a(l_less_than->clone(), l_zero->clone()),
                                       l_one->clone()),
                                     depth)
                            ->normalize();
        assert(
            l_lt_0_1.m_expr->equals(wrap_lambdas(church_true(depth), depth)));

        // Tests involving 3
        auto l_three =
            f(f(a(v(depth), a(v(depth), a(v(depth), v(depth + 1))))));

        // Test less_than(0, 3) = true
        auto l_lt_0_3 = wrap_lambdas(a(a(l_less_than->clone(), l_zero->clone()),
                                       l_three->clone()),
                                     depth)
                            ->normalize();
        assert(
            l_lt_0_3.m_expr->equals(wrap_lambdas(church_true(depth), depth)));

        // Test less_than(1, 3) = true
        auto l_lt_1_3 = wrap_lambdas(a(a(l_less_than->clone(), l_one->clone()),
                                       l_three->clone()),
                                     depth)
                            ->normalize();
        assert(
            l_lt_1_3.m_expr->equals(wrap_lambdas(church_true(depth), depth)));

        // Test less_than(2, 3) = true
        auto l_lt_2_3 = wrap_lambdas(a(a(l_less_than->clone(), l_two->clone()),
                                       l_three->clone()),
                                     depth)
                            ->normalize();
        assert(
            l_lt_2_3.m_expr->equals(wrap_lambdas(church_true(depth), depth)));

        // Test less_than(3, 0) = false
        auto l_lt_3_0 =
            wrap_lambdas(
                a(a(l_less_than->clone(), l_three->clone()), l_zero->clone()),
                depth)
                ->normalize();
        assert(
            l_lt_3_0.m_expr->equals(wrap_lambdas(church_false(depth), depth)));

        // Test less_than(3, 1) = false
        auto l_lt_3_1 =
            wrap_lambdas(
                a(a(l_less_than->clone(), l_three->clone()), l_one->clone()),
                depth)
                ->normalize();
        assert(
            l_lt_3_1.m_expr->equals(wrap_lambdas(church_false(depth), depth)));

        // Test less_than(3, 2) = false
        auto l_lt_3_2 =
            wrap_lambdas(
                a(a(l_less_than->clone(), l_three->clone()), l_two->clone()),
                depth)
                ->normalize();
        assert(
            l_lt_3_2.m_expr->equals(wrap_lambdas(church_false(depth), depth)));

        // Test less_than(3, 3) = false
        auto l_lt_3_3 =
            wrap_lambdas(
                a(a(l_less_than->clone(), l_three->clone()), l_three->clone()),
                depth)
                ->normalize();
        assert(
            l_lt_3_3.m_expr->equals(wrap_lambdas(church_false(depth), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_some()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_some = some(depth);
        // Structure: λx. λsomeCase. λnoneCase. someCase x
        auto expected = f(f(f(a(v(depth + 1), v(depth + 0)))));
        assert(l_some->equals(expected));

        // Test behavior: construct (some v(10))
        // When applied to someCase=v(20) and noneCase=v(30),
        // it should reduce to: someCase v(10) = v(20) v(10)
        auto l_some_x = a(l_some->clone(), v(depth + 10));

        // Apply to someCase and noneCase
        auto l_result =
            wrap_lambdas(a(a(l_some_x->clone(), v(depth + 20)), v(depth + 30)),
                         depth)
                ->normalize();

        // Expected: v(20) applied to v(10)
        assert(l_result.m_expr->equals(
            wrap_lambdas(a(v(depth + 20), v(depth + 10)), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_none()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_none = none(depth);
        // Structure: λsomeCase. λnoneCase. noneCase
        auto expected = f(f(v(depth + 1)));
        assert(l_none->equals(expected));

        // Test behavior: none applied to someCase=v(20) and noneCase=v(30)
        // should reduce to: noneCase = v(30)
        auto l_result =
            wrap_lambdas(a(a(l_none->clone(), v(depth + 20)), v(depth + 30)),
                         depth)
                ->normalize();

        // Expected: v(30) (the noneCase)
        assert(l_result.m_expr->equals(wrap_lambdas(v(depth + 30), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_scott_nil()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_nil = scott_nil(depth);
        auto expected = f(f(v(depth)));
        assert(l_nil->equals(expected));

        // check if nil (supply v(10) and v(11)) to see what it resolves to
        auto l_nil_result =
            wrap_lambdas(a(a(l_nil->clone(), v(depth + 10)), v(depth + 11)),
                         depth)
                ->normalize();

        // nil returns the first argument
        assert(l_nil_result.m_expr->equals(wrap_lambdas(v(depth + 10), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_scott_cons()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_cons = scott_cons(depth);
        auto expected =
            f(f(f(f(a(a(v(depth + 3), v(depth + 0)), v(depth + 1))))));
        assert(l_cons->equals(expected));

        // construct cons (v(10), v(11))
        auto l_cons_created =
            a(a(l_cons->clone(), v(depth + 10)), v(depth + 11));

        // interrogate cons with (v(12), v(13)) to see what it resolves to
        auto l_cons_result =
            wrap_lambdas(
                a(a(std::move(l_cons_created), v(depth + 12)), v(depth + 13)),
                depth)
                ->normalize();
        // cons returns the second argument applied to the head and tail
        assert(l_cons_result.m_expr->equals(wrap_lambdas(
            a(a(v(depth + 13), v(depth + 10)), v(depth + 11)), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_binary_zero()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_zero = binary_zero(depth);
        // binary_zero is scott_nil
        auto expected = f(f(v(depth)));
        assert(l_zero->equals(expected));

        // Test behavioral: binary_zero interrogated should return nilCase
        auto l_result =
            wrap_lambdas(a(a(l_zero->clone(), v(depth + 10)), v(depth + 11)),
                         depth)
                ->normalize();
        // Should return the nilCase (first argument)
        assert(l_result.m_expr->equals(wrap_lambdas(v(depth + 10), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_binary_is_zero()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_is_zero = binary_is_zero(depth);
        // Structure: λn. n TRUE (λh. λt. FALSE)
        auto expected = f(a(a(v(depth + 0), church_true(depth + 1)),
                            f(f(church_false(depth + 3)))));
        assert(l_is_zero->equals(expected));

        // Test behavioral: is_zero(binary_zero) = true
        // Apply binary_is_zero to binary_zero - it should return church_true
        auto l_zero = binary_zero(depth);

        // Now apply the result to two test values to check if it's TRUE
        // TRUE v(10) v(11) should return v(10)
        auto l_result = wrap_lambdas(a(a(a(l_is_zero->clone(), l_zero->clone()),
                                         v(depth + 10)),
                                       v(depth + 11)),
                                     depth)
                            ->normalize();

        // Should return the first argument (v(depth + 10)) since it's TRUE
        assert(l_result.m_expr->equals(wrap_lambdas(v(depth + 10), depth)));

        // Test behavioral: is_zero([1]) = false
        // [1] = CONS BIT1 NIL
        auto l_one =
            a(a(scott_cons(depth), church_true(depth)), scott_nil(depth));

        // Now apply the result to two test values to check if it's FALSE
        // FALSE v(10) v(11) should return v(11)
        auto l_result_one =
            wrap_lambdas(
                a(a(a(l_is_zero->clone(), l_one->clone()), v(depth + 10)),
                  v(depth + 11)),
                depth)
                ->normalize();

        // Should return the second argument (v(depth + 11)) since it's FALSE
        assert(l_result_one.m_expr->equals(wrap_lambdas(v(depth + 11), depth)));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_binary_succ()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_succ = binary_succ(depth);

        // Test behavioral: succ([]) = [1]
        auto l_zero = binary_zero(depth);
        auto l_succ_zero =
            wrap_lambdas(a(l_succ->clone(), l_zero->clone()), depth)
                ->normalize();
        // [1] = CONS BIT1 NIL in beta-normal form:
        // λnilCase.λconsCase. consCase TRUE NIL
        auto l_expected_one =
            wrap_lambdas(f(f(a(a(v(depth + 1), church_true(depth + 2)),
                               scott_nil(depth + 2)))),
                         depth);
        assert(l_succ_zero.m_expr->equals(l_expected_one));

        // Test behavioral: succ([1]) = [0,1]
        // [1] = CONS BIT1 NIL
        auto l_one =
            a(a(scott_cons(depth), church_true(depth)), scott_nil(depth));
        auto l_succ_one =
            wrap_lambdas(a(l_succ->clone(), l_one->clone()), depth)
                ->normalize();
        // [0,1] = CONS FALSE (CONS TRUE NIL) in beta-normal form:
        // λnilCase.λconsCase. consCase FALSE [1]
        auto l_expected_two =
            wrap_lambdas(f(f(a(a(v(depth + 1), church_false(depth + 2)),
                               f(f(a(a(v(depth + 3), church_true(depth + 4)),
                                     scott_nil(depth + 4))))))),
                         depth);
        assert(l_succ_one.m_expr->equals(l_expected_two));

        // Test behavioral: succ([0,1]) = [1,1]
        // [0,1] = CONS BIT0 (CONS BIT1 NIL)
        auto l_two =
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_true(depth)), scott_nil(depth)));
        auto l_succ_two =
            wrap_lambdas(a(l_succ->clone(), l_two->clone()), depth)
                ->normalize();
        // [1,1] = CONS TRUE (CONS TRUE NIL) in beta-normal form:
        // λnilCase.λconsCase. consCase TRUE [1]
        auto l_expected_three =
            wrap_lambdas(f(f(a(a(v(depth + 1), church_true(depth + 2)),
                               f(f(a(a(v(depth + 3), church_true(depth + 4)),
                                     scott_nil(depth + 4))))))),
                         depth);
        assert(l_succ_two.m_expr->equals(l_expected_three));

        // Test behavioral: succ([1,1]) = [0,0,1]
        // [1,1] = CONS BIT1 (CONS BIT1 NIL)
        auto l_three =
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_true(depth)), scott_nil(depth)));
        auto l_succ_three =
            wrap_lambdas(a(l_succ->clone(), l_three->clone()), depth)
                ->normalize();
        // [0,0,1] = CONS FALSE (CONS FALSE (CONS TRUE NIL)) in beta-normal
        // form: λnilCase.λconsCase. consCase FALSE [0,1]
        auto l_expected_four = wrap_lambdas(
            f(f(a(a(v(depth + 1), church_false(depth + 2)),
                  f(f(a(a(v(depth + 3), church_false(depth + 4)),
                        f(f(a(a(v(depth + 5), church_true(depth + 6)),
                              scott_nil(depth + 6)))))))))),
            depth);
        assert(l_succ_three.m_expr->equals(l_expected_four));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_binary_canonicalize()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_canonicalize = binary_canonicalize(depth);

        // Expected canonical forms (targets for all tests below)
        auto l_expected_empty = wrap_lambdas(scott_nil(depth), depth);

        auto l_expected_one =
            wrap_lambdas(f(f(a(a(v(depth + 1), church_true(depth + 2)),
                               scott_nil(depth + 2)))),
                         depth);

        auto l_expected_two =
            wrap_lambdas(f(f(a(a(v(depth + 1), church_false(depth + 2)),
                               f(f(a(a(v(depth + 3), church_true(depth + 4)),
                                     scott_nil(depth + 4))))))),
                         depth);

        auto l_expected_three =
            wrap_lambdas(f(f(a(a(v(depth + 1), church_true(depth + 2)),
                               f(f(a(a(v(depth + 3), church_true(depth + 4)),
                                     scott_nil(depth + 4))))))),
                         depth);

        auto l_expected_four = wrap_lambdas(
            f(f(a(a(v(depth + 1), church_false(depth + 2)),
                  f(f(a(a(v(depth + 3), church_false(depth + 4)),
                        f(f(a(a(v(depth + 5), church_true(depth + 6)),
                              scott_nil(depth + 6)))))))))),
            depth);

        // Test case 1: [] -> [] (zero stays zero)
        auto l_empty = scott_nil(depth);
        auto l_result_empty =
            wrap_lambdas(a(l_canonicalize->clone(), l_empty->clone()), depth)
                ->normalize();
        assert(l_result_empty.m_expr->equals(l_expected_empty));

        // Test case 2: [1] -> [1] (one stays one)
        auto l_one =
            a(a(scott_cons(depth), church_true(depth)), scott_nil(depth));
        auto l_result_one =
            wrap_lambdas(a(l_canonicalize->clone(), l_one->clone()), depth)
                ->normalize();
        assert(l_result_one.m_expr->equals(l_expected_one));

        // Test case 3: [0] -> [] (single trailing zero is trimmed)
        auto l_single_zero =
            a(a(scott_cons(depth), church_false(depth)), scott_nil(depth));
        auto l_result_single_zero =
            wrap_lambdas(a(l_canonicalize->clone(), l_single_zero->clone()),
                         depth)
                ->normalize();
        assert(l_result_single_zero.m_expr->equals(l_expected_empty));

        // Test case 4: [1, 0] -> [1] (one with one trailing zero)
        auto l_one_trailing_zero =
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_false(depth)), scott_nil(depth)));
        auto l_result_one_trailing =
            wrap_lambdas(
                a(l_canonicalize->clone(), l_one_trailing_zero->clone()), depth)
                ->normalize();
        assert(l_result_one_trailing.m_expr->equals(l_expected_one));

        // Test case 5: [1, 0, 0] -> [1] (one with two trailing zeros)
        auto l_one_two_trailing = a(
            a(scott_cons(depth), church_true(depth)),
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_false(depth)), scott_nil(depth))));
        auto l_result_one_two_trailing =
            wrap_lambdas(
                a(l_canonicalize->clone(), l_one_two_trailing->clone()), depth)
                ->normalize();
        assert(l_result_one_two_trailing.m_expr->equals(l_expected_one));

        // Test case 6: [0, 1] -> [0, 1] (two, already canonical)
        auto l_two =
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_true(depth)), scott_nil(depth)));
        auto l_result_two =
            wrap_lambdas(a(l_canonicalize->clone(), l_two->clone()), depth)
                ->normalize();
        assert(l_result_two.m_expr->equals(l_expected_two));

        // Test case 7: [0, 1, 0] -> [0, 1] (two with trailing zero)
        auto l_two_trailing = a(
            a(scott_cons(depth), church_false(depth)),
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_false(depth)), scott_nil(depth))));
        auto l_result_two_trailing =
            wrap_lambdas(a(l_canonicalize->clone(), l_two_trailing->clone()),
                         depth)
                ->normalize();
        assert(l_result_two_trailing.m_expr->equals(l_expected_two));

        // Test case 8: [1, 1] -> [1, 1] (three, already canonical)
        auto l_three =
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_true(depth)), scott_nil(depth)));
        auto l_result_three =
            wrap_lambdas(a(l_canonicalize->clone(), l_three->clone()), depth)
                ->normalize();
        assert(l_result_three.m_expr->equals(l_expected_three));

        // Test case 9: [1, 1, 0] -> [1, 1] (three with trailing zero)
        auto l_three_trailing = a(
            a(scott_cons(depth), church_true(depth)),
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_false(depth)), scott_nil(depth))));
        auto l_result_three_trailing =
            wrap_lambdas(a(l_canonicalize->clone(), l_three_trailing->clone()),
                         depth)
                ->normalize();
        assert(l_result_three_trailing.m_expr->equals(l_expected_three));

        // Test case 10: [0, 0, 1, 0, 0] -> [0, 0, 1] (four with two trailing
        // zeros)
        auto l_four_two_trailing =
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_false(depth)),
                a(a(scott_cons(depth), church_true(depth)),
                  a(a(scott_cons(depth), church_false(depth)),
                    a(a(scott_cons(depth), church_false(depth)),
                      scott_nil(depth))))));
        auto l_result_four_two_trailing =
            wrap_lambdas(
                a(l_canonicalize->clone(), l_four_two_trailing->clone()), depth)
                ->normalize();
        assert(l_result_four_two_trailing.m_expr->equals(l_expected_four));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_binary_pred()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_pred = binary_pred(depth);

        // Test behavioral: pred([]) = [] (saturating, 0 stays 0)
        auto l_zero = binary_zero(depth);
        auto l_pred_zero =
            wrap_lambdas(a(l_pred->clone(), l_zero->clone()), depth)
                ->normalize();
        // Expected: [] = NIL
        auto l_expected_zero = wrap_lambdas(scott_nil(depth), depth);
        assert(l_pred_zero.m_expr->equals(l_expected_zero));

        // Test behavioral: pred([1]) = [0]
        // [1] = CONS BIT1 NIL
        auto l_one =
            a(a(scott_cons(depth), church_true(depth)), scott_nil(depth));
        auto l_pred_one =
            wrap_lambdas(a(l_pred->clone(), l_one->clone()), depth)
                ->normalize();
        // [0] = CONS FALSE NIL in beta-normal form:
        // λnilCase.λconsCase. consCase FALSE NIL
        auto l_expected_one =
            wrap_lambdas(f(f(a(a(v(depth + 1), church_false(depth + 2)),
                               scott_nil(depth + 2)))),
                         depth);
        assert(l_pred_one.m_expr->equals(l_expected_one));

        // Test behavioral: pred([0,1]) = [1,0]
        // [0,1] = CONS BIT0 (CONS BIT1 NIL), represents 2
        auto l_two =
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_true(depth)), scott_nil(depth)));
        auto l_pred_two =
            wrap_lambdas(a(l_pred->clone(), l_two->clone()), depth)
                ->normalize();
        // [1,0] = CONS TRUE (CONS FALSE NIL) in beta-normal form:
        // λnilCase.λconsCase. consCase TRUE [0]
        auto l_expected_two =
            wrap_lambdas(f(f(a(a(v(depth + 1), church_true(depth + 2)),
                               f(f(a(a(v(depth + 3), church_false(depth + 4)),
                                     scott_nil(depth + 4))))))),
                         depth);
        assert(l_pred_two.m_expr->equals(l_expected_two));

        // Test behavioral: pred([1,1]) = [0,1]
        // [1,1] = CONS BIT1 (CONS BIT1 NIL), represents 3
        auto l_three =
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_true(depth)), scott_nil(depth)));
        auto l_pred_three =
            wrap_lambdas(a(l_pred->clone(), l_three->clone()), depth)
                ->normalize();
        // [0,1] = CONS FALSE (CONS TRUE NIL) in beta-normal form:
        // λnilCase.λconsCase. consCase FALSE [1]
        auto l_expected_three =
            wrap_lambdas(f(f(a(a(v(depth + 1), church_false(depth + 2)),
                               f(f(a(a(v(depth + 3), church_true(depth + 4)),
                                     scott_nil(depth + 4))))))),
                         depth);
        assert(l_pred_three.m_expr->equals(l_expected_three));

        // Test behavioral: pred([0,0,1]) = [1,1,0]
        // [0,0,1] = CONS BIT0 (CONS BIT0 (CONS BIT1 NIL)), represents 4
        auto l_four =
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_false(depth)),
                a(a(scott_cons(depth), church_true(depth)), scott_nil(depth))));
        auto l_pred_four =
            wrap_lambdas(a(l_pred->clone(), l_four->clone()), depth)
                ->normalize();
        // [1,1,0] = CONS TRUE (CONS TRUE (CONS FALSE NIL)) in beta-normal form:
        // λnilCase.λconsCase. consCase TRUE [1,0]
        auto l_expected_four = wrap_lambdas(
            f(f(a(a(v(depth + 1), church_true(depth + 2)),
                  f(f(a(a(v(depth + 3), church_true(depth + 4)),
                        f(f(a(a(v(depth + 5), church_false(depth + 6)),
                              scott_nil(depth + 6)))))))))),
            depth);
        assert(l_pred_four.m_expr->equals(l_expected_four));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_full_adder()
{
    using namespace dml::predef;

    auto test_at_depth = [](size_t depth)
    {
        auto l_full_adder = church_full_adder(depth);

        // Test structure: λa.λb.λc. PAIR (sum) (carry)
        // sum = a XOR (b XOR c)
        auto sum_expr =
            a(a(church_xor(depth + 3), v(depth)),
              a(a(church_xor(depth + 3), v(depth + 1)), v(depth + 2)));

        // carry = (a AND b) OR ((a AND c) OR (b AND c))
        auto carry_expr =
            a(a(church_or(depth + 3),
                a(a(church_and(depth + 3), v(depth)), v(depth + 1))),
              a(a(church_or(depth + 3),
                  a(a(church_and(depth + 3), v(depth)), v(depth + 2))),
                a(a(church_and(depth + 3), v(depth + 1)), v(depth + 2))));

        auto expected = f(f(f(a(a(church_pair(depth + 3), sum_expr->clone()),
                                carry_expr->clone()))));
        assert(l_full_adder->equals(expected));

        // Test all 8 combinations of (a, b, carry_in)
        // Full adder truth table:
        // a b c | sum carry
        // 0 0 0 |  0    0
        // 0 0 1 |  1    0
        // 0 1 0 |  1    0
        // 0 1 1 |  0    1
        // 1 0 0 |  1    0
        // 1 0 1 |  0    1
        // 1 1 0 |  0    1
        // 1 1 1 |  1    1

        auto test_case = [&](bool a_val, bool b_val, bool c_val,
                             bool expected_sum, bool expected_carry)
        {
            auto bool_to_church = [&](bool val)
            { return val ? church_true(depth) : church_false(depth); };

            // Apply full_adder to three booleans (don't wrap, apply directly)
            auto l_result = a(a(a(l_full_adder->clone(), bool_to_church(a_val)),
                                bool_to_church(b_val)),
                              bool_to_church(c_val));

            // Extract sum using FST and wrap for normalization
            auto l_sum =
                wrap_lambdas(a(church_fst(depth), l_result->clone()), depth)
                    ->normalize();

            auto l_expected_sum =
                wrap_lambdas(bool_to_church(expected_sum), depth)->normalize();

            assert(l_sum.m_expr->equals(l_expected_sum.m_expr));

            // Extract carry using SND and wrap for normalization
            auto l_carry =
                wrap_lambdas(a(church_snd(depth), l_result->clone()), depth)
                    ->normalize();

            auto l_expected_carry =
                wrap_lambdas(bool_to_church(expected_carry), depth)
                    ->normalize();

            assert(l_carry.m_expr->equals(l_expected_carry.m_expr));
        };

        // Test all 8 cases
        test_case(false, false, false, false, false); // 0 + 0 + 0 = 0, carry 0
        test_case(false, false, true, true, false);   // 0 + 0 + 1 = 1, carry 0
        test_case(false, true, false, true, false);   // 0 + 1 + 0 = 1, carry 0
        test_case(false, true, true, false, true);    // 0 + 1 + 1 = 0, carry 1
        test_case(true, false, false, true, false);   // 1 + 0 + 0 = 1, carry 0
        test_case(true, false, true, false, true);    // 1 + 0 + 1 = 0, carry 1
        test_case(true, true, false, false, true);    // 1 + 1 + 0 = 0, carry 1
        test_case(true, true, true, true, true);      // 1 + 1 + 1 = 1, carry 1
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_church_full_subtractor()
{
    using namespace dml::predef;

    auto test_at_depth = [](size_t depth)
    {
        // get the full subtractor function
        auto l_full_subtractor = church_full_subtractor(depth);

        // Test all 8 combinations of (a, b, carry_in)
        // Full adder truth table:
        // a b c | difference borrow
        // 0 0 0 |   0           0
        // 0 0 1 |   1           1
        // 0 1 0 |   1           1
        // 0 1 1 |   0           1
        // 1 0 0 |   1           0
        // 1 0 1 |   0           0
        // 1 1 0 |   0           0
        // 1 1 1 |   1           1

        auto test_case = [&](bool a_val, bool b_val, bool c_val,
                             bool expected_difference, bool expected_borrow)
        {
            auto bool_to_church = [&](bool val)
            { return val ? church_true(depth) : church_false(depth); };

            // apply the full subtractor to the three booleans
            auto l_result =
                wrap_lambdas(a_twr(l_full_subtractor->clone(),
                                   bool_to_church(a_val), bool_to_church(b_val),
                                   bool_to_church(c_val)),
                             depth)
                    ->normalize()
                    .m_expr;

            auto l_expected =
                wrap_lambdas(a_twr(church_pair(depth),
                                   bool_to_church(expected_difference),
                                   bool_to_church(expected_borrow)),
                             depth)
                    ->normalize()
                    .m_expr;

            // std::cout << *l_result << std::endl;
            // std::cout << *l_expected << std::endl;
            assert(l_result->equals(l_expected));
        };

        // Test all 8 cases
        test_case(false, false, false, false, false); // 0 - 0 - 0 = 0, borrow 0
        test_case(false, false, true, true, true);    // 0 - 0 - 1 = 1, borrow 1
        test_case(false, true, false, true, true);    // 0 - 1 - 0 = 1, borrow 1
        test_case(false, true, true, false, true);    // 0 - 1 - 1 = 0, borrow 1
        test_case(true, false, false, true, false);   // 1 - 0 - 0 = 1, borrow 0
        test_case(true, false, true, false, false);   // 1 - 0 - 1 = 0, borrow 0
        test_case(true, true, false, false, false);   // 1 - 1 - 0 = 0, borrow 0
        test_case(true, true, true, true, true);      // 1 - 1 - 1 = 1, borrow 1
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_binary_add()
{
    using namespace dml::predef;

    auto l_test_at_depth = [](size_t a_depth)
    {
        auto l_zero = binary_zero(a_depth);
        auto l_one = a(binary_succ(a_depth), l_zero->clone());
        auto l_two = a(binary_succ(a_depth), l_one->clone());
        auto l_three = a(binary_succ(a_depth), l_two->clone());
        auto l_four = a(binary_succ(a_depth), l_three->clone());
        auto l_five = a(binary_succ(a_depth), l_four->clone());

        auto l_norm_number_0 =
            wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_norm_number_0 << std::endl;

        auto l_norm_number_1 =
            wrap_lambdas(l_one->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_norm_number_1 << std::endl;

        auto l_norm_number_2 =
            wrap_lambdas(l_two->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_norm_number_2 << std::endl;

        auto l_norm_number_3 =
            wrap_lambdas(l_three->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_norm_number_3 << std::endl;

        auto l_norm_number_4 =
            wrap_lambdas(l_four->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_norm_number_4 << std::endl;

        auto l_norm_number_5 =
            wrap_lambdas(l_five->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_norm_number_5 << std::endl;

        // compute 0+0
        auto l_zero_plus_zero =
            wrap_lambdas(a_twr(binary_add(a_depth), l_zero->clone(),
                               l_zero->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_zero_plus_zero_expected =
            wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_zero << std::endl;
        assert(l_zero_plus_zero->equals(l_zero_plus_zero_expected));

        // compute 0+1
        auto l_zero_plus_one =
            wrap_lambdas(a_twr(binary_add(a_depth), l_zero->clone(),
                               l_one->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_zero_plus_one_expected =
            wrap_lambdas(l_one->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_zero_plus_one_expected << std::endl;
        assert(l_zero_plus_one->equals(l_zero_plus_one_expected));

        // compute 0+2
        auto l_zero_plus_two =
            wrap_lambdas(a_twr(binary_add(a_depth), l_zero->clone(),
                               l_two->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_zero_plus_two_expected =
            wrap_lambdas(l_two->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_zero_plus_two_expected << std::endl;
        assert(l_zero_plus_two->equals(l_zero_plus_two_expected));

        // compute 0+3
        auto l_zero_plus_three =
            wrap_lambdas(a_twr(binary_add(a_depth), l_zero->clone(),
                               l_three->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_zero_plus_three_expected =
            wrap_lambdas(l_three->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_zero_plus_three_expected << std::endl;
        assert(l_zero_plus_three->equals(l_zero_plus_three_expected));

        // compute 1+1
        auto l_one_plus_one =
            wrap_lambdas(a_twr(binary_add(a_depth), l_one->clone(),
                               l_one->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_one_plus_one_expected =
            wrap_lambdas(l_two->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_one_plus_one_expected << std::endl;
        assert(l_one_plus_one->equals(l_one_plus_one_expected));

        // compute 1+2
        auto l_one_plus_two =
            wrap_lambdas(a_twr(binary_add(a_depth), l_one->clone(),
                               l_two->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_one_plus_two_expected =
            wrap_lambdas(l_three->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_one_plus_two_expected << std::endl;
        assert(l_one_plus_two->equals(l_one_plus_two_expected));

        // compute 1+0
        auto l_one_plus_zero =
            wrap_lambdas(a_twr(binary_add(a_depth), l_one->clone(),
                               l_zero->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_one_plus_zero_expected =
            wrap_lambdas(l_one->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_one_plus_zero_expected << std::endl;
        assert(l_one_plus_zero->equals(l_one_plus_zero_expected));

        // compute 2+1
        auto l_two_plus_one =
            wrap_lambdas(a_twr(binary_add(a_depth), l_two->clone(),
                               l_one->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_two_plus_one_expected =
            wrap_lambdas(l_three->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_two_plus_one_expected << std::endl;
        assert(l_two_plus_one->equals(l_two_plus_one_expected));

        // compute 2+3
        auto l_two_plus_three =
            wrap_lambdas(a_twr(binary_add(a_depth), l_two->clone(),
                               l_three->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_two_plus_three_expected =
            wrap_lambdas(l_five->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_two_plus_three_expected << std::endl;
        assert(l_two_plus_three->equals(l_two_plus_three_expected));

        // compute 3+2
        auto l_three_plus_two =
            wrap_lambdas(a_twr(binary_add(a_depth), l_three->clone(),
                               l_two->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_three_plus_two_expected =
            wrap_lambdas(l_five->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_three_plus_two_expected << std::endl;
        assert(l_three_plus_two->equals(l_three_plus_two_expected));

        // compute 2+2 + carry
        auto l_two_plus_two_plus_carry =
            wrap_lambdas(a_twr(binary_add(a_depth), l_two->clone(),
                               l_two->clone(), church_true(a_depth)),
                         a_depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ })
                .m_expr;
        auto l_two_plus_two_plus_carry_expected =
            wrap_lambdas(l_five->clone(), a_depth)->normalize().m_expr;
        // std::cout << *l_two_plus_two_plus_carry_expected << std::endl;
        assert(l_two_plus_two_plus_carry->equals(
            l_two_plus_two_plus_carry_expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        l_test_at_depth(depth);
    }
}

// void test_binary_subtract()
// {
//     using namespace dml::predef;

//     auto l_test_at_depth = [](size_t a_depth)
//     {
//         auto l_zero = binary_zero(a_depth);
//         auto l_one = a(binary_succ(a_depth), l_zero->clone());
//         auto l_two = a(binary_succ(a_depth), l_one->clone());
//         auto l_three = a(binary_succ(a_depth), l_two->clone());
//         auto l_four = a(binary_succ(a_depth), l_three->clone());
//         auto l_five = a(binary_succ(a_depth), l_four->clone());

//         // compute 0-0
//         auto l_zero_minus_zero =
//             wrap_lambdas(a_twr(binary_subtract(a_depth), l_zero->clone(),
//                                l_zero->clone(), church_false(a_depth)),
//                          a_depth)
//                 ->normalize(
//                     std::numeric_limits<size_t>::max(),
//                     std::numeric_limits<size_t>::max(),
//                     [](const std::unique_ptr<lambda::expr>&
//                            a_expr) { /*std::cout << *a_expr << std::endl;*/
//                            })
//                 .m_expr;
//         auto l_zero_minus_zero_expected =
//             wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
//         // std::cout << *l_zero_minus_zero_expected << std::endl;
//         assert(l_zero_minus_zero->equals(l_zero_minus_zero_expected));

//         // compute 0-1
//         auto l_zero_minus_one =
//             wrap_lambdas(a_twr(binary_subtract(a_depth), l_zero->clone(),
//                                l_one->clone(), church_false(a_depth)),
//                          a_depth)
//                 ->normalize(
//                     std::numeric_limits<size_t>::max(),
//                     std::numeric_limits<size_t>::max(),
//                     [](const std::unique_ptr<lambda::expr>&
//                            a_expr) { /*std::cout << *a_expr << std::endl;*/
//                            })
//                 .m_expr;
//         auto l_zero_minus_one_expected =
//             wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
//         // std::cout << *l_zero_minus_zero_expected << std::endl;
//         assert(l_zero_minus_one->equals(l_zero_minus_one_expected));

//         // compute 0-2
//         auto l_zero_minus_two =
//             wrap_lambdas(a_twr(binary_subtract(a_depth), l_zero->clone(),
//                                l_two->clone(), church_false(a_depth)),
//                          a_depth)
//                 ->normalize(
//                     std::numeric_limits<size_t>::max(),
//                     std::numeric_limits<size_t>::max(),
//                     [](const std::unique_ptr<lambda::expr>&
//                            a_expr) { /*std::cout << *a_expr << std::endl;*/
//                            })
//                 .m_expr;
//         auto l_zero_minus_two_expected =
//             wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
//         // std::cout << *l_zero_minus_zero_expected << std::endl;
//         assert(l_zero_minus_two->equals(l_zero_minus_two_expected));

//         // compute 0-3
//         auto l_zero_minus_three =
//             wrap_lambdas(a_twr(binary_subtract(a_depth), l_zero->clone(),
//                                l_three->clone(), church_false(a_depth)),
//                          a_depth)
//                 ->normalize(
//                     std::numeric_limits<size_t>::max(),
//                     std::numeric_limits<size_t>::max(),
//                     [](const std::unique_ptr<lambda::expr>&
//                            a_expr) { /*std::cout << *a_expr << std::endl;*/
//                            })
//                 .m_expr;
//         auto l_zero_minus_three_expected =
//             wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
//         // std::cout << *l_zero_minus_zero_expected << std::endl;
//         assert(l_zero_minus_three->equals(l_zero_minus_three_expected));

//         // compute 1-0
//         auto l_one_minus_zero =
//             wrap_lambdas(a_twr(binary_subtract(a_depth), l_one->clone(),
//                                l_zero->clone(), church_false(a_depth)),
//                          a_depth)
//                 ->normalize(
//                     std::numeric_limits<size_t>::max(),
//                     std::numeric_limits<size_t>::max(),
//                     [](const std::unique_ptr<lambda::expr>&
//                            a_expr) { /*std::cout << *a_expr << std::endl;*/
//                            })
//                 .m_expr;
//         auto l_one_minus_zero_expected =
//             wrap_lambdas(l_one->clone(), a_depth)->normalize().m_expr;
//         std::cout << *l_one_minus_zero_expected << std::endl;
//         assert(l_one_minus_zero->equals(l_one_minus_zero_expected));

//         // compute 1-1
//         auto l_one_minus_one =
//             wrap_lambdas(a_twr(binary_subtract(a_depth), l_one->clone(),
//                                l_one->clone(), church_false(a_depth)),
//                          a_depth)
//                 ->normalize(
//                     std::numeric_limits<size_t>::max(),
//                     std::numeric_limits<size_t>::max(),
//                     [](const std::unique_ptr<lambda::expr>&
//                            a_expr) { /*std::cout << *a_expr << std::endl;*/
//                            })
//                 .m_expr;
//         auto l_one_minus_one_expected =
//             wrap_lambdas(a_twr(scott_cons(a_depth), church_false(a_depth),
//                                scott_nil(a_depth)),
//                          a_depth)
//                 ->normalize()
//                 .m_expr;
//         std::cout << *l_one_minus_one_expected << std::endl;
//         assert(l_one_minus_one->equals(l_one_minus_one_expected));

//         // compute 1-2
//         auto l_one_minus_two =
//             wrap_lambdas(a_twr(binary_subtract(a_depth), l_one->clone(),
//                                l_two->clone(), church_false(a_depth)),
//                          a_depth)
//                 ->normalize(std::numeric_limits<size_t>::max(),
//                             std::numeric_limits<size_t>::max(),
//                             [](const std::unique_ptr<lambda::expr>& a_expr)
//                             { std::cout << *a_expr << std::endl; })
//                 .m_expr;
//         auto l_one_minus_two_expected =
//             wrap_lambdas(a_twr(scott_cons(a_depth), church_false(a_depth),
//                                scott_nil(a_depth)),
//                          a_depth)
//                 ->normalize()
//                 .m_expr;
//         std::cout << *l_one_minus_two_expected << std::endl;
//         assert(l_one_minus_two->equals(l_one_minus_two_expected));
//     };

//     for(size_t depth = 0; depth <= 5; ++depth)
//     {
//         l_test_at_depth(depth);
//     }
// }

void predef_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;
    TEST(test_y_combinator);
    TEST(test_church_true);
    TEST(test_church_false);
    TEST(test_church_not);
    TEST(test_church_and);
    TEST(test_church_or);
    TEST(test_church_xor);
    TEST(test_church_full_adder);
    TEST(test_church_full_subtractor);
    TEST(test_church_zero);
    TEST(test_church_succ);
    TEST(test_church_is_zero);
    TEST(test_church_pair);
    TEST(test_church_fst);
    TEST(test_church_snd);
    TEST(test_church_pred);
    TEST(test_church_sub);
    TEST(test_church_less_than);
    TEST(test_some);
    TEST(test_none);
    TEST(test_scott_nil);
    TEST(test_scott_cons);
    TEST(test_binary_zero);
    TEST(test_binary_is_zero);
    TEST(test_binary_succ);
    TEST(test_binary_canonicalize);
    TEST(test_binary_pred);
    TEST(test_binary_add);
    // TEST(test_binary_subtract);
}

#endif // UNIT_TEST
