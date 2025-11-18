#include "../include/encode.hpp"

using namespace lambda;

namespace dml
{
namespace encode
{

// // church boolean
// std::unique_ptr<lambda::expr> church_boolean(bool a_boolean)
// {
//     if(a_boolean)
//         return f(f(v(0)));
//     else
//         return f(f(v(1)));
// }

// // church numeral
// std::unique_ptr<lambda::expr> church_numeral(size_t a_numeral)
// {
//     auto l_result = v(1);
//     for(size_t i = 0; i < a_numeral; ++i)
//         l_result = a(v(0), std::move(l_result));
//     l_result = f(f(std::move(l_result)));
//     return l_result;
// }

// // church pair
// std::unique_ptr<lambda::expr>
// church_pair(std::unique_ptr<lambda::expr>&& a_first,
//             std::unique_ptr<lambda::expr>&& a_second)
// {
//     return f(v(0));
// }

} // namespace encode
} // namespace dml

#ifdef UNIT_TEST
#include "test_utils.hpp"

// void test_encode_church_boolean()
// {
//     using namespace dml::encode;
//     // encode true
//     {
//         auto l_boolean = church_boolean(true);
//         assert(l_boolean->equals(f(f(v(0)))));
//     }
//     // encode false
//     {
//         auto l_boolean = church_boolean(false);
//         assert(l_boolean->equals(f(f(v(1)))));
//     }
// }

// void test_encode_church_numeral()
// {
//     using namespace dml::encode;
//     // encode 0
//     {
//         auto l_numeral = church_numeral(0);
//         assert(l_numeral->equals(f(f(v(1)))));
//     }
//     // encode 1
//     {
//         auto l_numeral = church_numeral(1);
//         assert(l_numeral->equals(f(f(a(v(0), v(1))))));
//     }
//     // encode 2
//     {
//         auto l_numeral = church_numeral(2);
//         assert(l_numeral->equals(f(f(a(v(0), a(v(0), v(1)))))));
//     }
//     // encode 3
//     {
//         auto l_numeral = church_numeral(3);
//         assert(l_numeral->equals(f(f(a(v(0), a(v(0), a(v(0), v(1))))))));
//     }
//     // encode 4
//     {
//         auto l_numeral = church_numeral(4);
//         assert(
//             l_numeral->equals(f(f(a(v(0), a(v(0), a(v(0), a(v(0),
//             v(1)))))))));
//     }
// }

void encode_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;
    // TEST(test_encode_church_boolean);
    // TEST(test_encode_church_numeral);
}

#endif // UNIT_TEST