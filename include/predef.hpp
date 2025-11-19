#ifndef PREDEF_HPP
#define PREDEF_HPP

#include "lambda.hpp"

namespace dml
{
namespace predef
{

// church true
std::unique_ptr<lambda::expr> church_true(size_t a_binder_depth);

// church false
std::unique_ptr<lambda::expr> church_false(size_t a_binder_depth);

// church not
std::unique_ptr<lambda::expr> church_not(size_t a_binder_depth);

// church and
std::unique_ptr<lambda::expr> church_and(size_t a_binder_depth);

// church or
std::unique_ptr<lambda::expr> church_or(size_t a_binder_depth);

// church xor
std::unique_ptr<lambda::expr> church_xor(size_t a_binder_depth);

// church zero
std::unique_ptr<lambda::expr> church_zero(size_t a_binder_depth);

// church succ
std::unique_ptr<lambda::expr> church_succ(size_t a_binder_depth);

// church is_zero
std::unique_ptr<lambda::expr> church_is_zero(size_t a_binder_depth);

// church pair
std::unique_ptr<lambda::expr> church_pair(size_t a_binder_depth);

// church fst
std::unique_ptr<lambda::expr> church_fst(size_t a_binder_depth);

// church snd
std::unique_ptr<lambda::expr> church_snd(size_t a_binder_depth);

// church pred (predecessor)
std::unique_ptr<lambda::expr> church_pred(size_t a_binder_depth);

// church sub (subtraction)
std::unique_ptr<lambda::expr> church_sub(size_t a_binder_depth);

// church less_than
std::unique_ptr<lambda::expr> church_less_than(size_t a_binder_depth);

} // namespace predef
} // namespace dml

#endif // CONSTANTS_HPP