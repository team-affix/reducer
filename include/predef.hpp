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

// church not
std::unique_ptr<lambda::expr> church_not();

// church and
std::unique_ptr<lambda::expr> church_and();

// church or
std::unique_ptr<lambda::expr> church_or();

// church zero
std::unique_ptr<lambda::expr> church_zero();

// church succ
std::unique_ptr<lambda::expr> church_succ();

// church is_zero
std::unique_ptr<lambda::expr> church_is_zero();

// church pair
std::unique_ptr<lambda::expr> church_pair();

} // namespace predef
} // namespace dml

#endif // CONSTANTS_HPP