#ifndef PREDEF_HPP
#define PREDEF_HPP

#include "lambda.hpp"

namespace dml
{
namespace predef
{

// church true
std::unique_ptr<lambda::expr> church_true();

// church false
std::unique_ptr<lambda::expr> church_false();

// church zero
std::unique_ptr<lambda::expr> church_zero();

// church succ
std::unique_ptr<lambda::expr> church_succ();

// church pair
std::unique_ptr<lambda::expr> church_pair();

} // namespace predef
} // namespace dml

#endif // CONSTANTS_HPP