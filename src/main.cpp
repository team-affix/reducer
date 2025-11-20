#include "test_utils.hpp"

extern void scope_test_main();
extern void func_test_main();
extern void program_test_main();
extern void model_test_main();
extern void reduce_test_main();
extern void predef_test_main();
extern void encode_test_main();

void unit_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    // TEST(scope_test_main);
    // TEST(func_test_main);
    // TEST(program_test_main);
    // TEST(model_test_main); // Disabled: hangs during execution
    TEST(predef_test_main);
    TEST(encode_test_main);
    // TEST(reduce_test_main);
}

int main()
{
    unit_test_main();

    return 0;
}
