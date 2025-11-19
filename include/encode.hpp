#ifndef ENCODE_HPP
#define ENCODE_HPP

#include "lambda.hpp"

namespace dml
{
namespace encode
{

// church boolean
std::unique_ptr<lambda::expr> church_boolean(size_t a_binder_depth,
                                             bool a_boolean);

// church numeral
std::unique_ptr<lambda::expr> church_numeral(size_t a_binder_depth,
                                             size_t a_numeral);

// church pair
std::unique_ptr<lambda::expr>
church_pair(size_t a_binder_depth, std::unique_ptr<lambda::expr>&& a_first,
            std::unique_ptr<lambda::expr>&& a_second);

} // namespace encode
} // namespace dml

#endif // ENCODE_HPP