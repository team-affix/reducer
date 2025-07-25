#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <assert.h>
#include <list>
#include <map>
#include <functional>
#include "../include/node.hpp"
#include "../mcts/include/mcts.hpp"
#include "../include/bool_reduce.hpp"

#include "test_utils.hpp"

#include <iostream>

extern void bool_reduce_test_main();

void unit_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;
    
    TEST(bool_reduce_test_main);
}

int main()
{
    // unit_test_main();
    
    node n1 = var(10);
    std::cout << n1 << std::endl;
    node n2 = conjoin(
        invert(var(10)),
        disjoin(
            var(11),
            var(12)
        )
    );
    std::cout << n2 << std::endl;
    node n3 = helper(0, {disjoin(var(13), var(12)), var(14)});
    std::cout << n3 << std::endl;
    // node n4 = disjoin(one(), zero());
    // std::cout << n4 << std::endl;
    
    std::cout << (n1 == n2) << std::endl;
    node n4 = helper(101, {disjoin(var(13), var(12)), var(14)});
    std::cout << (n3 == n4) << std::endl;
    // node n5 = op(101, {disjoin(var(13), var(12)), var(15)});
    // std::cout << (n3 == n5) << std::endl;


    // node n6 = invert(invert(var(13)));

    // std::cout << n6 << std::endl;

    // std::cout << dn(n6) << std::endl;

    // std::cout << n6 << std::endl;

    // std::cout << dn(n6) << std::endl;

    // std::cout << n6 << std::endl;

    // node n7 = invert(goal(10));

    // std::cout << n7 << std::endl;

    // node n8 = conjoin(n7, n7);

    // std::cout << n8 << std::endl;

    // std::cout << resolve_goal(n8, 10, var(0)) << std::endl;

    // std::cout << n8 << std::endl;

    // std::cout << std::endl << std::endl;

    // node n9 = conjoin(goal(10), goal(10));

    // std::cout << n9 << std::endl;
    
    // std::cout << resolve_goal(n9, 10, invert(goal(11))) << " " << n9 << std::endl;
    // std::cout << resolve_goal(n9, 11, conjoin(goal(12), goal(13))) << " " << n9 << std::endl;
    // std::cout << resolve_goal(n9, 12, var(5)) << " " << n9 << std::endl;
    // std::cout << resolve_goal(n9, 13, var(6)) << " " << n9 << std::endl;

    return 0;
}
