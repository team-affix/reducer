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

// scott reverse
std::unique_ptr<lambda::expr> scott_reverse(size_t a_binder_depth)
{
    return f( // list
        a_twr(a(y_combinator(a_binder_depth + 1),
                f(                                    // self
                    f(                                // l
                        f(                            // r
                            a_twr(L(2),               // l
                                  L(3),               // r
                                  f(                  // lh
                                      f(              // lt
                                          a_twr(L(1), // self
                                                L(5), // lt
                                                a_twr(scott_cons(
                                                          a_binder_depth + 6),
                                                      L(4), // lh
                                                      L(3)  // r
                                                      ))))))))),
              L(0), scott_nil(a_binder_depth + 1)));
}

// scott compare lengths
std::unique_ptr<lambda::expr> scott_compare_lengths(size_t a_binder_depth)
{
    return a(y_combinator(a_binder_depth),
             f(                            // self
                 f(                        // x
                     f(                    // y
                         a_twr(L(1),       // x
                               a_twr(L(2), // y
                                     f(f(f(L(4)))),
                                     f(     // yh
                                         f( // yt
                                             f(f(f(L(5))))))),
                               f(                  // xh
                                   f(              // xt
                                       a_twr(L(2), // y
                                             f(f(f(L(7)))),
                                             f(                  // yh
                                                 f(              // yt
                                                     a_twr(L(0), // self
                                                           L(4), // xt
                                                           L(6)  // yt
                                                           )))))))))));
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

// binary compare <=>
std::unique_ptr<lambda::expr> binary_compare(size_t a_binder_depth)
{
    auto l_error = [](size_t a_depth) { return v(1000 + a_depth); };

    return f(          // x
        f(             // y
            f(         // lt
                f(     // eq
                    f( // gt
                        a_twr(
                            scott_compare_lengths(a_binder_depth + 5),
                            L(0), // x
                            L(1), // y
                            L(2), // lt
                            a_twr(
                                y_combinator(a_binder_depth + 5),
                                f(         // self
                                    f(     // x'
                                        f( // y'
                                            a_twr(
                                                L(6),       // x'
                                                a_twr(L(7), // y'
                                                      L(3), // eq
                                                      // x' nil, y' not nil?
                                                      // impossible
                                                      l_error(a_binder_depth +
                                                              8)),
                                                f(     // x'h
                                                    f( // x't
                                                        a_twr(
                                                            L(7), // y'
                                                            // x' not nil, y'
                                                            // nil? impossible
                                                            l_error(
                                                                a_binder_depth +
                                                                10),
                                                            f(     // y'h
                                                                f( // y't
                                                                    a_twr(
                                                                        L(8), // x'h
                                                                        a_twr(
                                                                            L(10), // y'h
                                                                            a_twr(
                                                                                L(5), // self
                                                                                L(9), // x't
                                                                                L(11) // y't
                                                                                ),
                                                                            L(4) // gt
                                                                            ),
                                                                        a_twr(
                                                                            L(10), // y'h
                                                                            L(2), // lt
                                                                            a_twr(
                                                                                L(5), // self
                                                                                L(9), // x't
                                                                                L(11) // y't
                                                                                )))))))))))),
                                a(/*reverse x*/
                                  scott_reverse(a_binder_depth + 5),
                                  L(0) // x
                                  ),
                                a(/*reverse y*/
                                  scott_reverse(a_binder_depth + 5),
                                  L(1) // y
                                  )),  // eq
                            L(4)       // gt
                            ))))));
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
    //                     (t NIL (λnth. λntt. CONS FALSE nt)) ; INLINE
    //                                              canonicalization,
    //                                              followed by h = 1 → 0 :: t
    //                     (CONS TRUE (rec t))))    ; h = 0 → 1 :: pred t
    return a(
        y_combinator(a_binder_depth),
        f(                  // self
            f(              // n
                a_twr(L(1), // n
                      scott_nil(a_binder_depth + 2),
                      f(                        // nh
                          f(                    // nt
                              a_twr(L(2),       // nh
                                    a_twr(L(3), // nt
                                          scott_nil(a_binder_depth + 4),
                                          f(     // nth (unused)
                                              f( // ntt (unused)
                                                  a_twr(scott_cons(
                                                            a_binder_depth + 6),
                                                        church_false(
                                                            a_binder_depth + 6),
                                                        L(3) // nt
                                                        )))),
                                    a_twr(scott_cons(a_binder_depth + 4),
                                          church_true(a_binder_depth + 4),
                                          a(L(0), // self
                                            L(3)  // nt

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

// binary subtract
std::unique_ptr<lambda::expr> binary_subtract(size_t a_binder_depth)
{
    return a(
        y_combinator(a_binder_depth),
        f(             // self
            f(         // x
                f(     // y
                    f( // b
                        a_twr(
                            L(1), // x
                            // x is NIL
                            scott_nil(a_binder_depth + 4),
                            f(     // xh
                                f( // xt
                                    a_twr(
                                        L(2),            // y
                                        a_twr(           // y is NIL
                                            L(3),        // b
                                            a_twr(L(0),  // self
                                                  L(1),  // x
                                                  a_twr( // CONS TRUE NIL
                                                      scott_cons(
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
                                                a(a_twr(church_full_subtractor(
                                                            a_binder_depth + 8),
                                                        L(4), // xh
                                                        L(6), // yh
                                                        L(3)  // b
                                                        ),
                                                  f(     // diff
                                                      f( // bout
                                                          a_twr(
                                                              L(8), // diff
                                                              a_twr(
                                                                  scott_cons(
                                                                      a_binder_depth +
                                                                      10),
                                                                  church_true(
                                                                      a_binder_depth +
                                                                      10),
                                                                  a_twr(
                                                                      L(0), // self
                                                                      L(5), // xt
                                                                      L(7), // yt
                                                                      L(9) // bout
                                                                      )),
                                                              a_twr(
                                                                  a_twr(
                                                                      L(0), // self
                                                                      L(5), // xt
                                                                      L(7), // yt
                                                                      L(9) // bout
                                                                      ),
                                                                  scott_nil(
                                                                      a_binder_depth +
                                                                      10),
                                                                  f(     // subh
                                                                      f( // subt
                                                                          a_twr(
                                                                              scott_cons(
                                                                                  a_binder_depth +
                                                                                  12),
                                                                              church_false(
                                                                                  a_binder_depth +
                                                                                  12),
                                                                              a_twr(
                                                                                  scott_cons(
                                                                                      a_binder_depth +
                                                                                      12),
                                                                                  L(10), // subh
                                                                                  L(11) // subt
                                                                                  ))))))))))))))))))));
}

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

void test_scott_compare_lengths()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_compare_lengths = scott_compare_lengths(depth);

        // Helper to build a list with n elements (using variables)
        auto build_list_of_length = [depth](size_t n, size_t start_var)
        {
            auto result = scott_nil(depth);
            for(int i = n - 1; i >= 0; --i)
            {
                result = a(a(scott_cons(depth), v(depth + start_var + i)),
                           std::move(result));
            }
            return result;
        };

        // Expected case markers (use distinct variables)
        auto v_less = v(depth + 100);    // Marker for less-than case
        auto v_same = v(depth + 200);    // Marker for same case
        auto v_greater = v(depth + 300); // Marker for greater-than case

        // Test case 1: [] vs [] = same (both empty)
        auto l_empty_a = scott_nil(depth);
        auto l_empty_b = scott_nil(depth);
        auto l_result_0_0 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_empty_a->clone()),
                        l_empty_b->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_0_0 = wrap_lambdas(v_same->clone(), depth);
        assert(l_result_0_0.m_expr->equals(l_expected_0_0));

        // Test case 2: [] vs [v(10)] = less
        auto l_list_1 = build_list_of_length(1, 10);
        auto l_result_0_1 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_empty_a->clone()),
                        l_list_1->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_0_1 = wrap_lambdas(v_less->clone(), depth);
        assert(l_result_0_1.m_expr->equals(l_expected_0_1));

        // Test case 3: [v(10)] vs [] = greater
        auto l_result_1_0 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_1->clone()),
                        l_empty_a->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_1_0 = wrap_lambdas(v_greater->clone(), depth);
        assert(l_result_1_0.m_expr->equals(l_expected_1_0));

        // Test case 4: [v(10)] vs [v(20)] = same (both length 1)
        auto l_list_1b = build_list_of_length(1, 20);
        auto l_result_1_1 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_1->clone()),
                        l_list_1b->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_1_1 = wrap_lambdas(v_same->clone(), depth);
        assert(l_result_1_1.m_expr->equals(l_expected_1_1));

        // Test case 5: [v(10)] vs [v(20), v(21)] = less
        auto l_list_2 = build_list_of_length(2, 20);
        auto l_result_1_2 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_1->clone()),
                        l_list_2->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_1_2 = wrap_lambdas(v_less->clone(), depth);
        assert(l_result_1_2.m_expr->equals(l_expected_1_2));

        // Test case 6: [v(20), v(21)] vs [v(10)] = greater
        auto l_result_2_1 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_2->clone()),
                        l_list_1->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_2_1 = wrap_lambdas(v_greater->clone(), depth);
        assert(l_result_2_1.m_expr->equals(l_expected_2_1));

        // Test case 7: [v(10), v(11)] vs [v(20), v(21)] = same (both length 2)
        auto l_list_2b = build_list_of_length(2, 10);
        auto l_result_2_2 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_2b->clone()),
                        l_list_2->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_2_2 = wrap_lambdas(v_same->clone(), depth);
        assert(l_result_2_2.m_expr->equals(l_expected_2_2));

        // Test case 8: length 2 vs length 3 = less
        auto l_list_3 = build_list_of_length(3, 30);
        auto l_result_2_3 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_2->clone()),
                        l_list_3->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_2_3 = wrap_lambdas(v_less->clone(), depth);
        assert(l_result_2_3.m_expr->equals(l_expected_2_3));

        // Test case 9: length 3 vs length 2 = greater
        auto l_result_3_2 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_3->clone()),
                        l_list_2->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_3_2 = wrap_lambdas(v_greater->clone(), depth);
        assert(l_result_3_2.m_expr->equals(l_expected_3_2));

        // Test case 10: length 3 vs length 3 = same
        auto l_list_3b = build_list_of_length(3, 40);
        auto l_result_3_3 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_3->clone()),
                        l_list_3b->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_3_3 = wrap_lambdas(v_same->clone(), depth);
        assert(l_result_3_3.m_expr->equals(l_expected_3_3));

        // Test case 11: length 1 vs length 5 = less (bigger gap)
        auto l_list_5 = build_list_of_length(5, 50);
        auto l_result_1_5 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_1->clone()),
                        l_list_5->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_1_5 = wrap_lambdas(v_less->clone(), depth);
        assert(l_result_1_5.m_expr->equals(l_expected_1_5));

        // Test case 12: length 5 vs length 1 = greater (bigger gap)
        auto l_result_5_1 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_5->clone()),
                        l_list_1->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_5_1 = wrap_lambdas(v_greater->clone(), depth);
        assert(l_result_5_1.m_expr->equals(l_expected_5_1));

        // Test case 13: length 5 vs length 5 = same
        auto l_list_5b = build_list_of_length(5, 60);
        auto l_result_5_5 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_5->clone()),
                        l_list_5b->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_5_5 = wrap_lambdas(v_same->clone(), depth);
        assert(l_result_5_5.m_expr->equals(l_expected_5_5));

        // Test case 14: length 0 vs length 3 = less
        auto l_result_0_3 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_empty_a->clone()),
                        l_list_3->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_0_3 = wrap_lambdas(v_less->clone(), depth);
        assert(l_result_0_3.m_expr->equals(l_expected_0_3));

        // Test case 15: length 3 vs length 0 = greater
        auto l_result_3_0 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_3->clone()),
                        l_empty_a->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_3_0 = wrap_lambdas(v_greater->clone(), depth);
        assert(l_result_3_0.m_expr->equals(l_expected_3_0));

        // Test case 16: length 4 vs length 4 = same
        auto l_list_4a = build_list_of_length(4, 70);
        auto l_list_4b = build_list_of_length(4, 80);
        auto l_result_4_4 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_4a->clone()),
                        l_list_4b->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_4_4 = wrap_lambdas(v_same->clone(), depth);
        assert(l_result_4_4.m_expr->equals(l_expected_4_4));

        // Test case 17: length 3 vs length 4 = less
        auto l_result_3_4 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_3->clone()),
                        l_list_4a->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_3_4 = wrap_lambdas(v_less->clone(), depth);
        assert(l_result_3_4.m_expr->equals(l_expected_3_4));

        // Test case 18: length 4 vs length 3 = greater
        auto l_result_4_3 =
            wrap_lambdas(
                a(a(a(a(a(l_compare_lengths->clone(), l_list_4a->clone()),
                        l_list_3->clone()),
                      v_less->clone()),
                    v_same->clone()),
                  v_greater->clone()),
                depth)
                ->normalize();
        auto l_expected_4_3 = wrap_lambdas(v_greater->clone(), depth);
        assert(l_result_4_3.m_expr->equals(l_expected_4_3));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_scott_reverse()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_reverse = scott_reverse(depth);

        // Helper to build a raw list from elements (no normalization)
        auto build_list =
            [depth](std::vector<std::unique_ptr<lambda::expr>> elements)
        {
            auto result = scott_nil(depth);
            // Build list in reverse order (since we cons from the back)
            for(int i = elements.size() - 1; i >= 0; --i)
            {
                result = a(a(scott_cons(depth), std::move(elements[i])),
                           std::move(result));
            }
            return result;
        };

        // Test case 1: reverse([]) = []
        auto l_empty = scott_nil(depth);
        auto l_result_empty =
            wrap_lambdas(a(l_reverse->clone(), l_empty->clone()), depth)
                ->normalize();
        auto l_expected_empty = wrap_lambdas(scott_nil(depth), depth);
        assert(l_result_empty.m_expr->equals(l_expected_empty));

        // Test case 2: reverse([v(10)]) = [v(10)]
        std::vector<std::unique_ptr<lambda::expr>> elems_single_in;
        elems_single_in.push_back(v(depth + 10));
        auto l_single = build_list(std::move(elems_single_in));
        auto l_result_single =
            wrap_lambdas(a(l_reverse->clone(), l_single->clone()), depth)
                ->normalize();
        std::vector<std::unique_ptr<lambda::expr>> elems_single_out;
        elems_single_out.push_back(v(depth + 10));
        auto l_expected_single_raw = build_list(std::move(elems_single_out));
        auto l_expected_single =
            wrap_lambdas(std::move(l_expected_single_raw), depth)->normalize();
        assert(l_result_single.m_expr->equals(l_expected_single.m_expr));

        // Test case 3: reverse([v(10), v(11)]) = [v(11), v(10)]
        std::vector<std::unique_ptr<lambda::expr>> elems_two_in;
        elems_two_in.push_back(v(depth + 10));
        elems_two_in.push_back(v(depth + 11));
        auto l_two = build_list(std::move(elems_two_in));
        auto l_result_two =
            wrap_lambdas(a(l_reverse->clone(), l_two->clone()), depth)
                ->normalize();
        std::vector<std::unique_ptr<lambda::expr>> elems_two_out;
        elems_two_out.push_back(v(depth + 11));
        elems_two_out.push_back(v(depth + 10));
        auto l_expected_two_raw = build_list(std::move(elems_two_out));
        auto l_expected_two =
            wrap_lambdas(std::move(l_expected_two_raw), depth)->normalize();
        assert(l_result_two.m_expr->equals(l_expected_two.m_expr));

        // Test case 4: reverse([v(10), v(11), v(12)]) = [v(12), v(11), v(10)]
        std::vector<std::unique_ptr<lambda::expr>> elems_three_in;
        elems_three_in.push_back(v(depth + 10));
        elems_three_in.push_back(v(depth + 11));
        elems_three_in.push_back(v(depth + 12));
        auto l_three = build_list(std::move(elems_three_in));
        auto l_result_three =
            wrap_lambdas(a(l_reverse->clone(), l_three->clone()), depth)
                ->normalize();
        std::vector<std::unique_ptr<lambda::expr>> elems_three_out;
        elems_three_out.push_back(v(depth + 12));
        elems_three_out.push_back(v(depth + 11));
        elems_three_out.push_back(v(depth + 10));
        auto l_expected_three_raw = build_list(std::move(elems_three_out));
        auto l_expected_three =
            wrap_lambdas(std::move(l_expected_three_raw), depth)->normalize();
        assert(l_result_three.m_expr->equals(l_expected_three.m_expr));

        // Test case 5: reverse([v(10), v(11), v(12), v(13)]) = [v(13), v(12),
        // v(11), v(10)]
        std::vector<std::unique_ptr<lambda::expr>> elems_four_in;
        elems_four_in.push_back(v(depth + 10));
        elems_four_in.push_back(v(depth + 11));
        elems_four_in.push_back(v(depth + 12));
        elems_four_in.push_back(v(depth + 13));
        auto l_four = build_list(std::move(elems_four_in));
        auto l_result_four =
            wrap_lambdas(a(l_reverse->clone(), l_four->clone()), depth)
                ->normalize();
        std::vector<std::unique_ptr<lambda::expr>> elems_four_out;
        elems_four_out.push_back(v(depth + 13));
        elems_four_out.push_back(v(depth + 12));
        elems_four_out.push_back(v(depth + 11));
        elems_four_out.push_back(v(depth + 10));
        auto l_expected_four_raw = build_list(std::move(elems_four_out));
        auto l_expected_four =
            wrap_lambdas(std::move(l_expected_four_raw), depth)->normalize();
        assert(l_result_four.m_expr->equals(l_expected_four.m_expr));

        // Test case 6: reverse(reverse([])) = []
        auto l_reverse_reverse_empty =
            wrap_lambdas(
                a(l_reverse->clone(), a(l_reverse->clone(), l_empty->clone())),
                depth)
                ->normalize();
        assert(l_reverse_reverse_empty.m_expr->equals(l_expected_empty));

        // Test case 7: reverse(reverse([v(10)])) = [v(10)]
        auto l_reverse_reverse_single =
            wrap_lambdas(
                a(l_reverse->clone(), a(l_reverse->clone(), l_single->clone())),
                depth)
                ->normalize();
        assert(
            l_reverse_reverse_single.m_expr->equals(l_expected_single.m_expr));

        // Test case 8: reverse(reverse([v(10), v(11)])) = [v(10), v(11)]
        std::vector<std::unique_ptr<lambda::expr>> elems_two_rr_in;
        elems_two_rr_in.push_back(v(depth + 10));
        elems_two_rr_in.push_back(v(depth + 11));
        auto l_two_rr = build_list(std::move(elems_two_rr_in));
        auto l_reverse_reverse_two =
            wrap_lambdas(a(l_reverse->clone(),
                           a(l_reverse->clone(), std::move(l_two_rr))),
                         depth)
                ->normalize();
        std::vector<std::unique_ptr<lambda::expr>> elems_two_rr_expected;
        elems_two_rr_expected.push_back(v(depth + 10));
        elems_two_rr_expected.push_back(v(depth + 11));
        auto l_expected_two_rr_raw =
            build_list(std::move(elems_two_rr_expected));
        auto l_expected_two_rr =
            wrap_lambdas(std::move(l_expected_two_rr_raw), depth)->normalize();
        assert(l_reverse_reverse_two.m_expr->equals(l_expected_two_rr.m_expr));

        // Test case 9: reverse(reverse([v(10), v(11), v(12)])) = [v(10), v(11),
        // v(12)]
        std::vector<std::unique_ptr<lambda::expr>> elems_three_rr_in;
        elems_three_rr_in.push_back(v(depth + 10));
        elems_three_rr_in.push_back(v(depth + 11));
        elems_three_rr_in.push_back(v(depth + 12));
        auto l_three_rr = build_list(std::move(elems_three_rr_in));
        auto l_reverse_reverse_three =
            wrap_lambdas(a(l_reverse->clone(),
                           a(l_reverse->clone(), std::move(l_three_rr))),
                         depth)
                ->normalize();
        std::vector<std::unique_ptr<lambda::expr>> elems_three_rr_expected;
        elems_three_rr_expected.push_back(v(depth + 10));
        elems_three_rr_expected.push_back(v(depth + 11));
        elems_three_rr_expected.push_back(v(depth + 12));
        auto l_expected_three_rr_raw =
            build_list(std::move(elems_three_rr_expected));
        auto l_expected_three_rr =
            wrap_lambdas(std::move(l_expected_three_rr_raw), depth)
                ->normalize();
        assert(
            l_reverse_reverse_three.m_expr->equals(l_expected_three_rr.m_expr));

        // Test case 10: reverse([TRUE, FALSE]) = [FALSE, TRUE]
        std::vector<std::unique_ptr<lambda::expr>> elems_bool_in;
        elems_bool_in.push_back(church_true(depth));
        elems_bool_in.push_back(church_false(depth));
        auto l_bool_list = build_list(std::move(elems_bool_in));
        auto l_result_bool =
            wrap_lambdas(a(l_reverse->clone(), l_bool_list->clone()), depth)
                ->normalize();
        std::vector<std::unique_ptr<lambda::expr>> elems_bool_out;
        elems_bool_out.push_back(church_false(depth));
        elems_bool_out.push_back(church_true(depth));
        auto l_expected_bool_raw = build_list(std::move(elems_bool_out));
        auto l_expected_bool =
            wrap_lambdas(std::move(l_expected_bool_raw), depth)->normalize();
        assert(l_result_bool.m_expr->equals(l_expected_bool.m_expr));

        // Test case 11: reverse([v(20)]) = [v(20)]
        std::vector<std::unique_ptr<lambda::expr>> elems_v20_in;
        elems_v20_in.push_back(v(depth + 20));
        auto l_single_v20 = build_list(std::move(elems_v20_in));
        auto l_result_single_v20 =
            wrap_lambdas(a(l_reverse->clone(), l_single_v20->clone()), depth)
                ->normalize();
        std::vector<std::unique_ptr<lambda::expr>> elems_v20_out;
        elems_v20_out.push_back(v(depth + 20));
        auto l_expected_single_v20_raw = build_list(std::move(elems_v20_out));
        auto l_expected_single_v20 =
            wrap_lambdas(std::move(l_expected_single_v20_raw), depth)
                ->normalize();
        assert(
            l_result_single_v20.m_expr->equals(l_expected_single_v20.m_expr));

        // Test case 12: reverse([v(5), v(6), v(7), v(8), v(9)]) = [v(9), v(8),
        // v(7), v(6), v(5)]
        std::vector<std::unique_ptr<lambda::expr>> elems_five_in;
        elems_five_in.push_back(v(depth + 5));
        elems_five_in.push_back(v(depth + 6));
        elems_five_in.push_back(v(depth + 7));
        elems_five_in.push_back(v(depth + 8));
        elems_five_in.push_back(v(depth + 9));
        auto l_five = build_list(std::move(elems_five_in));
        auto l_result_five =
            wrap_lambdas(a(l_reverse->clone(), l_five->clone()), depth)
                ->normalize();
        std::vector<std::unique_ptr<lambda::expr>> elems_five_out;
        elems_five_out.push_back(v(depth + 9));
        elems_five_out.push_back(v(depth + 8));
        elems_five_out.push_back(v(depth + 7));
        elems_five_out.push_back(v(depth + 6));
        elems_five_out.push_back(v(depth + 5));
        auto l_expected_five_raw = build_list(std::move(elems_five_out));
        auto l_expected_five =
            wrap_lambdas(std::move(l_expected_five_raw), depth)->normalize();
        assert(l_result_five.m_expr->equals(l_expected_five.m_expr));
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

        auto l_expected_five = wrap_lambdas(
            f(f(a(a(v(depth + 1), church_true(depth + 2)),
                  f(f(a(a(v(depth + 3), church_false(depth + 4)),
                        f(f(a(a(v(depth + 5), church_true(depth + 6)),
                              scott_nil(depth + 6)))))))))),
            depth);

        auto l_expected_twenty = wrap_lambdas(
            f(f(a(
                a(v(depth + 1), church_false(depth + 2)),
                f(f(a(
                    a(v(depth + 3), church_false(depth + 4)),
                    f(f(a(a(v(depth + 5), church_true(depth + 6)),
                          f(f(a(a(v(depth + 7), church_false(depth + 8)),
                                f(f(a(a(v(depth + 9), church_true(depth + 10)),
                                      scott_nil(depth + 10)))))))))))))))),
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

        // Test case 11: [1, 0, 1] -> [1, 0, 1] (five, already canonical - non-
        // trailing zero preserved)
        auto l_five =
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_false(depth)),
                a(a(scott_cons(depth), church_true(depth)), scott_nil(depth))));
        auto l_result_five =
            wrap_lambdas(a(l_canonicalize->clone(), l_five->clone()), depth)
                ->normalize();
        assert(l_result_five.m_expr->equals(l_expected_five));

        // Test case 12: [1, 0, 1, 0] -> [1, 0, 1] (five with one trailing zero)
        auto l_five_trailing =
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_false(depth)),
                a(a(scott_cons(depth), church_true(depth)),
                  a(a(scott_cons(depth), church_false(depth)),
                    scott_nil(depth)))));
        auto l_result_five_trailing =
            wrap_lambdas(a(l_canonicalize->clone(), l_five_trailing->clone()),
                         depth)
                ->normalize();
        assert(l_result_five_trailing.m_expr->equals(l_expected_five));

        // Test case 13: [1, 0, 1, 0, 0] -> [1, 0, 1] (five with two trailing
        // zeros)
        auto l_five_two_trailing =
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_false(depth)),
                a(a(scott_cons(depth), church_true(depth)),
                  a(a(scott_cons(depth), church_false(depth)),
                    a(a(scott_cons(depth), church_false(depth)),
                      scott_nil(depth))))));
        auto l_result_five_two_trailing =
            wrap_lambdas(
                a(l_canonicalize->clone(), l_five_two_trailing->clone()), depth)
                ->normalize();
        assert(l_result_five_two_trailing.m_expr->equals(l_expected_five));

        // Test case 14: [0, 0, 1, 0, 1] -> [0, 0, 1, 0, 1] (twenty, already
        // canonical)
        auto l_twenty = a(a(scott_cons(depth), church_false(depth)),
                          a(a(scott_cons(depth), church_false(depth)),
                            a(a(scott_cons(depth), church_true(depth)),
                              a(a(scott_cons(depth), church_false(depth)),
                                a(a(scott_cons(depth), church_true(depth)),
                                  scott_nil(depth))))));
        auto l_result_twenty =
            wrap_lambdas(a(l_canonicalize->clone(), l_twenty->clone()), depth)
                ->normalize();
        assert(l_result_twenty.m_expr->equals(l_expected_twenty));

        // Test case 15: [0, 0, 1, 0, 1, 0] -> [0, 0, 1, 0, 1] (twenty with
        // trailing zero)
        auto l_twenty_trailing =
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_false(depth)),
                a(a(scott_cons(depth), church_true(depth)),
                  a(a(scott_cons(depth), church_false(depth)),
                    a(a(scott_cons(depth), church_true(depth)),
                      a(a(scott_cons(depth), church_false(depth)),
                        scott_nil(depth)))))));
        auto l_result_twenty_trailing =
            wrap_lambdas(a(l_canonicalize->clone(), l_twenty_trailing->clone()),
                         depth)
                ->normalize();
        assert(l_result_twenty_trailing.m_expr->equals(l_expected_twenty));

        // Test case 16: [0, 0, 0] -> [] (all zeros trim to empty)
        auto l_all_zeros = a(
            a(scott_cons(depth), church_false(depth)),
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_false(depth)), scott_nil(depth))));
        auto l_result_all_zeros =
            wrap_lambdas(a(l_canonicalize->clone(), l_all_zeros->clone()),
                         depth)
                ->normalize();
        assert(l_result_all_zeros.m_expr->equals(l_expected_empty));

        // Test case 17: [0, 0, 0, 0, 0] -> [] (many zeros trim to empty)
        auto l_many_zeros = a(a(scott_cons(depth), church_false(depth)),
                              a(a(scott_cons(depth), church_false(depth)),
                                a(a(scott_cons(depth), church_false(depth)),
                                  a(a(scott_cons(depth), church_false(depth)),
                                    a(a(scott_cons(depth), church_false(depth)),
                                      scott_nil(depth))))));
        auto l_result_many_zeros =
            wrap_lambdas(a(l_canonicalize->clone(), l_many_zeros->clone()),
                         depth)
                ->normalize();
        assert(l_result_many_zeros.m_expr->equals(l_expected_empty));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        test_at_depth(depth);
    }
}

void test_binary_compare()
{
    using namespace dml::predef;
    auto test_at_depth = [](size_t depth)
    {
        auto l_compare = binary_compare(depth);

        // Helper to build a binary number from boolean vector (LSB first)
        auto build_binary = [depth](const std::vector<bool>& bits)
        {
            auto result = scott_nil(depth);
            for(int i = bits.size() - 1; i >= 0; --i)
            {
                result = a(a(scott_cons(depth), bits[i] ? church_true(depth)
                                                        : church_false(depth)),
                           std::move(result));
            }
            return result;
        };

        // Expected case markers (use distinct variables)
        auto v_lt = v(depth + 100); // Marker for less-than case
        auto v_eq = v(depth + 200); // Marker for equal case
        auto v_gt = v(depth + 300); // Marker for greater-than case

        // Test case 1: 0 == 0 (both empty)
        auto l_zero_a = scott_nil(depth);
        auto l_zero_b = scott_nil(depth);
        auto l_result_0_0 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_zero_a->clone()),
                                 l_zero_b->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_0_0 = wrap_lambdas(v_eq->clone(), depth);
        assert(l_result_0_0.m_expr->equals(l_expected_0_0));

        // Test case 2: 0 < 1
        auto l_zero = scott_nil(depth);
        auto l_one = build_binary({true}); // 1 = 0b1
        auto l_result_0_1 =
            wrap_lambdas(
                a(a(a(a(a(l_compare->clone(), l_zero->clone()), l_one->clone()),
                      v_lt->clone()),
                    v_eq->clone()),
                  v_gt->clone()),
                depth)
                ->normalize();
        auto l_expected_0_1 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_0_1.m_expr->equals(l_expected_0_1));

        // Test case 3: 1 > 0
        auto l_result_1_0 =
            wrap_lambdas(
                a(a(a(a(a(l_compare->clone(), l_one->clone()), l_zero->clone()),
                      v_lt->clone()),
                    v_eq->clone()),
                  v_gt->clone()),
                depth)
                ->normalize();
        auto l_expected_1_0 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_1_0.m_expr->equals(l_expected_1_0));

        // Test case 4: 1 == 1
        auto l_one_b = build_binary({true});
        auto l_result_1_1 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_one->clone()),
                                 l_one_b->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_1_1 = wrap_lambdas(v_eq->clone(), depth);
        assert(l_result_1_1.m_expr->equals(l_expected_1_1));

        // Test case 5: 1 < 2
        auto l_two = build_binary({false, true}); // 2 = 0b10
        auto l_result_1_2 =
            wrap_lambdas(
                a(a(a(a(a(l_compare->clone(), l_one->clone()), l_two->clone()),
                      v_lt->clone()),
                    v_eq->clone()),
                  v_gt->clone()),
                depth)
                ->normalize();
        auto l_expected_1_2 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_1_2.m_expr->equals(l_expected_1_2));

        // Test case 6: 2 > 1
        auto l_result_2_1 =
            wrap_lambdas(
                a(a(a(a(a(l_compare->clone(), l_two->clone()), l_one->clone()),
                      v_lt->clone()),
                    v_eq->clone()),
                  v_gt->clone()),
                depth)
                ->normalize();
        auto l_expected_2_1 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_2_1.m_expr->equals(l_expected_2_1));

        // Test case 7: 2 == 2
        auto l_two_b = build_binary({false, true});
        auto l_result_2_2 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_two->clone()),
                                 l_two_b->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_2_2 = wrap_lambdas(v_eq->clone(), depth);
        assert(l_result_2_2.m_expr->equals(l_expected_2_2));

        // Test case 8: 2 < 3
        auto l_three = build_binary({true, true}); // 3 = 0b11
        auto l_result_2_3 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_two->clone()),
                                 l_three->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_2_3 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_2_3.m_expr->equals(l_expected_2_3));

        // Test case 9: 3 > 2
        auto l_result_3_2 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_three->clone()),
                                 l_two->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_3_2 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_3_2.m_expr->equals(l_expected_3_2));

        // Test case 10: 3 == 3
        auto l_three_b = build_binary({true, true});
        auto l_result_3_3 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_three->clone()),
                                 l_three_b->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_3_3 = wrap_lambdas(v_eq->clone(), depth);
        assert(l_result_3_3.m_expr->equals(l_expected_3_3));

        // Test case 11: 3 < 4 (previously failing case - different lengths!)
        auto l_four = build_binary({false, false, true}); // 4 = 0b100
        auto l_result_3_4 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_three->clone()),
                                 l_four->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_3_4 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_3_4.m_expr->equals(l_expected_3_4));

        // Test case 12: 4 > 3
        auto l_result_4_3 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_four->clone()),
                                 l_three->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_4_3 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_4_3.m_expr->equals(l_expected_4_3));

        // Test case 13: 5 > 3
        auto l_five = build_binary({true, false, true}); // 5 = 0b101
        auto l_result_5_3 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_five->clone()),
                                 l_three->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_5_3 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_5_3.m_expr->equals(l_expected_5_3));

        // Test case 14: 3 < 5
        auto l_result_3_5 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_three->clone()),
                                 l_five->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_3_5 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_3_5.m_expr->equals(l_expected_3_5));

        // Test case 15: 7 < 8 (different lengths)
        auto l_seven = build_binary({true, true, true});          // 7 = 0b111
        auto l_eight = build_binary({false, false, false, true}); // 8 = 0b1000
        auto l_result_7_8 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_seven->clone()),
                                 l_eight->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_7_8 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_7_8.m_expr->equals(l_expected_7_8));

        // Test case 16: 8 > 7
        auto l_result_8_7 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_eight->clone()),
                                 l_seven->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_8_7 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_8_7.m_expr->equals(l_expected_8_7));

        // Test case 17: 10 < 15 (same length, different values)
        auto l_ten = build_binary({false, true, false, true});   // 10 = 0b1010
        auto l_fifteen = build_binary({true, true, true, true}); // 15 = 0b1111
        auto l_result_10_15 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_ten->clone()),
                                 l_fifteen->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_10_15 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_10_15.m_expr->equals(l_expected_10_15));

        // Test case 18: 15 > 10
        auto l_result_15_10 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_fifteen->clone()),
                                 l_ten->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_15_10 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_15_10.m_expr->equals(l_expected_15_10));

        // Test case 19: 15 < 16 (different lengths, boundary)
        auto l_sixteen =
            build_binary({false, false, false, false, true}); // 16 = 0b10000
        auto l_result_15_16 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_fifteen->clone()),
                                 l_sixteen->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_15_16 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_15_16.m_expr->equals(l_expected_15_16));

        // Test case 20: 31 < 32 (max for 5 bits vs min for 6 bits)
        auto l_thirtyone =
            build_binary({true, true, true, true, true}); // 31 = 0b11111
        auto l_thirtytwo = build_binary(
            {false, false, false, false, false, true}); // 32 = 0b100000
        auto l_result_31_32 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_thirtyone->clone()),
                                 l_thirtytwo->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_31_32 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_31_32.m_expr->equals(l_expected_31_32));

        // Test case 21: 20 == 20 (equal multi-bit numbers)
        auto l_twenty_a =
            build_binary({false, false, true, false, true}); // 20 = 0b10100
        auto l_twenty_b =
            build_binary({false, false, true, false, true}); // 20 = 0b10100
        auto l_result_20_20 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_twenty_a->clone()),
                                 l_twenty_b->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_20_20 = wrap_lambdas(v_eq->clone(), depth);
        assert(l_result_20_20.m_expr->equals(l_expected_20_20));

        // Test case 22: 19 < 20 (adjacent numbers)
        auto l_nineteen =
            build_binary({true, true, false, false, true}); // 19 = 0b10011
        auto l_result_19_20 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_nineteen->clone()),
                                 l_twenty_a->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_19_20 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_19_20.m_expr->equals(l_expected_19_20));

        // Test case 23: 20 > 19
        auto l_result_20_19 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_twenty_a->clone()),
                                 l_nineteen->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_20_19 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_20_19.m_expr->equals(l_expected_20_19));

        // Test case 24: 5 == 5 (equal with non-contiguous bits)
        auto l_five_b = build_binary({true, false, true}); // 5 = 0b101
        auto l_result_5_5 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_five->clone()),
                                 l_five_b->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_5_5 = wrap_lambdas(v_eq->clone(), depth);
        assert(l_result_5_5.m_expr->equals(l_expected_5_5));

        // Test case 25: 1 < 100 (very far apart)
        auto l_hundred = build_binary(
            {false, false, true, false, false, true, true}); // 100 = 0b1100100
        auto l_result_1_100 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_one->clone()),
                                 l_hundred->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_1_100 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_1_100.m_expr->equals(l_expected_1_100));

        // Test case 26: 100 > 1
        auto l_result_100_1 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_hundred->clone()),
                                 l_one->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_100_1 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_100_1.m_expr->equals(l_expected_100_1));

        // Test case 27: 0 < 63 (empty vs full 6-bit number)
        auto l_sixtythree =
            build_binary({true, true, true, true, true, true}); // 63 = 0b111111
        auto l_result_0_63 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_zero->clone()),
                                 l_sixtythree->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_0_63 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_0_63.m_expr->equals(l_expected_0_63));

        // Test case 28: 63 > 0
        auto l_result_63_0 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_sixtythree->clone()),
                                 l_zero->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_63_0 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_63_0.m_expr->equals(l_expected_63_0));

        // Test case 29: 7 < 64 (different bit lengths, powers of 2)
        auto l_sixtyfour = build_binary(
            {false, false, false, false, false, false, true}); // 64 = 0b1000000
        auto l_result_7_64 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_seven->clone()),
                                 l_sixtyfour->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_7_64 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_7_64.m_expr->equals(l_expected_7_64));

        // Test case 30: 64 > 7
        auto l_result_64_7 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_sixtyfour->clone()),
                                 l_seven->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_64_7 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_64_7.m_expr->equals(l_expected_64_7));

        // Test case 31: 10 < 127 (large gap)
        auto l_onetwentyseven = build_binary(
            {true, true, true, true, true, true, true}); // 127 = 0b1111111
        auto l_result_10_127 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_ten->clone()),
                                 l_onetwentyseven->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_10_127 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_10_127.m_expr->equals(l_expected_10_127));

        // Test case 32: 127 > 10
        auto l_result_127_10 =
            wrap_lambdas(
                a(a(a(a(a(l_compare->clone(), l_onetwentyseven->clone()),
                        l_ten->clone()),
                      v_lt->clone()),
                    v_eq->clone()),
                  v_gt->clone()),
                depth)
                ->normalize();
        auto l_expected_127_10 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_127_10.m_expr->equals(l_expected_127_10));

        // Test case 33: 1 < 128 (smallest vs power of 2)
        auto l_onetwentyeight =
            build_binary({false, false, false, false, false, false, false,
                          true}); // 128 = 0b10000000
        auto l_result_1_128 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_one->clone()),
                                 l_onetwentyeight->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_1_128 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_1_128.m_expr->equals(l_expected_1_128));

        // Test case 34: 128 > 1
        auto l_result_128_1 =
            wrap_lambdas(
                a(a(a(a(a(l_compare->clone(), l_onetwentyeight->clone()),
                        l_one->clone()),
                      v_lt->clone()),
                    v_eq->clone()),
                  v_gt->clone()),
                depth)
                ->normalize();
        auto l_expected_128_1 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_128_1.m_expr->equals(l_expected_128_1));

        // Test case 35: 31 < 100 (both multi-bit, far apart)
        auto l_result_31_100 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_thirtyone->clone()),
                                 l_hundred->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_31_100 = wrap_lambdas(v_lt->clone(), depth);
        assert(l_result_31_100.m_expr->equals(l_expected_31_100));

        // Test case 36: 100 > 31
        auto l_result_100_31 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_hundred->clone()),
                                 l_thirtyone->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_100_31 = wrap_lambdas(v_gt->clone(), depth);
        assert(l_result_100_31.m_expr->equals(l_expected_100_31));

        // Test case 37: 100 == 100 (equal large numbers)
        auto l_hundred_b = build_binary(
            {false, false, true, false, false, true, true}); // 100 = 0b1100100
        auto l_result_100_100 =
            wrap_lambdas(a(a(a(a(a(l_compare->clone(), l_hundred->clone()),
                                 l_hundred_b->clone()),
                               v_lt->clone()),
                             v_eq->clone()),
                           v_gt->clone()),
                         depth)
                ->normalize();
        auto l_expected_100_100 = wrap_lambdas(v_eq->clone(), depth);
        assert(l_result_100_100.m_expr->equals(l_expected_100_100));
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

        // Expected canonical forms (pred now canonicalizes output)
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

        auto l_expected_seven = wrap_lambdas(
            f(f(a(a(v(depth + 1), church_true(depth + 2)),
                  f(f(a(a(v(depth + 3), church_true(depth + 4)),
                        f(f(a(a(v(depth + 5), church_true(depth + 6)),
                              scott_nil(depth + 6)))))))))),
            depth);

        // Test 1: pred(0) = 0 (saturating, 0 stays 0)
        auto l_zero = binary_zero(depth);
        auto l_pred_zero =
            wrap_lambdas(a(l_pred->clone(), l_zero->clone()), depth)
                ->normalize();
        assert(l_pred_zero.m_expr->equals(l_expected_empty));

        // Test 2: pred(1) = 0 (canonicalized to empty list)
        // This is a key test: pred([1]) produces [0] which canonicalizes to []
        auto l_one =
            a(a(scott_cons(depth), church_true(depth)), scott_nil(depth));
        auto l_pred_one =
            wrap_lambdas(a(l_pred->clone(), l_one->clone()), depth)
                ->normalize();
        assert(l_pred_one.m_expr->equals(l_expected_empty));

        // Test 3: pred(2) = 1 (canonicalized)
        // pred([0,1]) produces [1,0] which canonicalizes to [1]
        auto l_two =
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_true(depth)), scott_nil(depth)));
        auto l_pred_two =
            wrap_lambdas(a(l_pred->clone(), l_two->clone()), depth)
                ->normalize();
        assert(l_pred_two.m_expr->equals(l_expected_one));

        // Test 4: pred(3) = 2 (already canonical)
        auto l_three =
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_true(depth)), scott_nil(depth)));
        auto l_pred_three =
            wrap_lambdas(a(l_pred->clone(), l_three->clone()), depth)
                ->normalize(
                    std::numeric_limits<size_t>::max(),
                    std::numeric_limits<size_t>::max(),
                    [](const std::unique_ptr<lambda::expr>&
                           a_expr) { /*std::cout << *a_expr << std::endl;*/ });
        // std::cout << *l_pred_three.m_expr << std::endl;
        assert(l_pred_three.m_expr->equals(l_expected_two));

        // Test 5: pred(4) = 3 (canonicalized)
        // pred([0,0,1]) produces [1,1,0] which canonicalizes to [1,1]
        auto l_four =
            a(a(scott_cons(depth), church_false(depth)),
              a(a(scott_cons(depth), church_false(depth)),
                a(a(scott_cons(depth), church_true(depth)), scott_nil(depth))));
        auto l_pred_four =
            wrap_lambdas(a(l_pred->clone(), l_four->clone()), depth)
                ->normalize();
        assert(l_pred_four.m_expr->equals(l_expected_three));

        // Test 6: pred(5) = 4 (already canonical)
        // [1,0,1] -> [0,0,1]
        auto l_five =
            a(a(scott_cons(depth), church_true(depth)),
              a(a(scott_cons(depth), church_false(depth)),
                a(a(scott_cons(depth), church_true(depth)), scott_nil(depth))));
        auto l_pred_five =
            wrap_lambdas(a(l_pred->clone(), l_five->clone()), depth)
                ->normalize();
        assert(l_pred_five.m_expr->equals(l_expected_four));

        // Test 7: pred(8) = 7 (canonicalized)
        // [0,0,0,1] -> pred produces [1,1,1,0] which canonicalizes to [1,1,1]
        auto l_eight = a(a(scott_cons(depth), church_false(depth)),
                         a(a(scott_cons(depth), church_false(depth)),
                           a(a(scott_cons(depth), church_false(depth)),
                             a(a(scott_cons(depth), church_true(depth)),
                               scott_nil(depth)))));
        auto l_pred_eight =
            wrap_lambdas(a(l_pred->clone(), l_eight->clone()), depth)
                ->normalize();
        assert(l_pred_eight.m_expr->equals(l_expected_seven));
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

void test_binary_subtract()
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

        // compute 0-0
        auto l_zero_minus_zero =
            wrap_lambdas(a_twr(binary_subtract(a_depth), l_zero->clone(),
                               l_zero->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(std::numeric_limits<size_t>::max(),
                            std::numeric_limits<size_t>::max(),
                            [](const std::unique_ptr<lambda::expr>& a_expr)
                            { std::cout << *a_expr << std::endl; })
                .m_expr;
        auto l_zero_minus_zero_expected =
            wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
        std::cout << *l_zero_minus_zero_expected << std::endl;
        assert(l_zero_minus_zero->equals(l_zero_minus_zero_expected));

        // compute 0-1
        auto l_zero_minus_one =
            wrap_lambdas(a_twr(binary_subtract(a_depth), l_zero->clone(),
                               l_one->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(std::numeric_limits<size_t>::max(),
                            std::numeric_limits<size_t>::max(),
                            [](const std::unique_ptr<lambda::expr>& a_expr)
                            { std::cout << *a_expr << std::endl; })
                .m_expr;
        auto l_zero_minus_one_expected =
            wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
        std::cout << *l_zero_minus_zero_expected << std::endl;
        assert(l_zero_minus_one->equals(l_zero_minus_one_expected));

        // compute 0-2
        auto l_zero_minus_two =
            wrap_lambdas(a_twr(binary_subtract(a_depth), l_zero->clone(),
                               l_two->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(std::numeric_limits<size_t>::max(),
                            std::numeric_limits<size_t>::max(),
                            [](const std::unique_ptr<lambda::expr>& a_expr)
                            { std::cout << *a_expr << std::endl; })
                .m_expr;
        auto l_zero_minus_two_expected =
            wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
        std::cout << *l_zero_minus_zero_expected << std::endl;
        assert(l_zero_minus_two->equals(l_zero_minus_two_expected));

        // compute 0-3
        auto l_zero_minus_three =
            wrap_lambdas(a_twr(binary_subtract(a_depth), l_zero->clone(),
                               l_three->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(std::numeric_limits<size_t>::max(),
                            std::numeric_limits<size_t>::max(),
                            [](const std::unique_ptr<lambda::expr>& a_expr)
                            { std::cout << *a_expr << std::endl; })
                .m_expr;
        auto l_zero_minus_three_expected =
            wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
        std::cout << *l_zero_minus_zero_expected << std::endl;
        assert(l_zero_minus_three->equals(l_zero_minus_three_expected));

        // compute 1-0
        auto l_one_minus_zero =
            wrap_lambdas(a_twr(binary_subtract(a_depth), l_one->clone(),
                               l_zero->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(std::numeric_limits<size_t>::max(),
                            std::numeric_limits<size_t>::max(),
                            [](const std::unique_ptr<lambda::expr>& a_expr)
                            { std::cout << *a_expr << std::endl; })
                .m_expr;
        auto l_one_minus_zero_expected =
            wrap_lambdas(l_one->clone(), a_depth)->normalize().m_expr;
        std::cout << *l_one_minus_zero_expected << std::endl;
        assert(l_one_minus_zero->equals(l_one_minus_zero_expected));

        // compute 1-1
        auto l_one_minus_one =
            wrap_lambdas(a_twr(binary_subtract(a_depth), l_one->clone(),
                               l_one->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(std::numeric_limits<size_t>::max(),
                            std::numeric_limits<size_t>::max(),
                            [](const std::unique_ptr<lambda::expr>& a_expr)
                            { std::cout << *a_expr << std::endl; })
                .m_expr;
        auto l_one_minus_one_expected =
            wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
        std::cout << *l_one_minus_one_expected << std::endl;
        assert(l_one_minus_one->equals(l_one_minus_one_expected));

        // compute 1-2
        auto l_one_minus_two =
            wrap_lambdas(a_twr(binary_subtract(a_depth), l_one->clone(),
                               l_two->clone(), church_false(a_depth)),
                         a_depth)
                ->normalize(std::numeric_limits<size_t>::max(),
                            std::numeric_limits<size_t>::max(),
                            [](const std::unique_ptr<lambda::expr>& a_expr)
                            { std::cout << *a_expr << std::endl; })
                .m_expr;
        auto l_one_minus_two_expected =
            wrap_lambdas(l_zero->clone(), a_depth)->normalize().m_expr;
        std::cout << *l_one_minus_two_expected << std::endl;
        assert(l_one_minus_two->equals(l_one_minus_two_expected));
    };

    for(size_t depth = 0; depth <= 5; ++depth)
    {
        l_test_at_depth(depth);
    }
}

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
    TEST(test_scott_compare_lengths);
    TEST(test_scott_reverse);
    TEST(test_binary_zero);
    TEST(test_binary_is_zero);
    TEST(test_binary_succ);
    TEST(test_binary_canonicalize);
    TEST(test_binary_compare);
    TEST(test_binary_pred);
    TEST(test_binary_add);
    // TEST(test_binary_subtract);
}

#endif // UNIT_TEST
