#include <iostream>
#include <random>
#include <sstream>

////////////////////////////////////////////////////
////////////////////// TESTING /////////////////////
////////////////////////////////////////////////////
#ifdef UNIT_TEST

#include "test_utils.hpp"
#include <random>
#include <sstream>

// void test_zero_construct_and_equality_check()
// {
//     assert(zero() == zero());
//     bool_node l_node = zero();
//     // check data field
//     assert(std::get_if<zero_t>(&l_node.m_data));
// }

// void test_one_construct_and_equality_check()
// {
//     assert(one() == one());
//     bool_node l_node = one();
//     // check data field
//     assert(std::get_if<one_t>(&l_node.m_data));
// }

// void test_var_construct_and_equality_check()
// {
//     assert(var(0) == var(0));
//     assert(var(0) != var(1));
//     bool_node l_node = var(1);
//     // check data field
//     var_t* l_data;
//     assert(l_data = std::get_if<var_t>(&l_node.m_data));
//     assert(l_data->m_index == 1);
// }

// void test_invert_construct_and_equality_check()
// {
//     assert(invert(var(0)) == invert(var(0)));
//     assert(invert(var(0)) != invert(var(1)));
//     bool_node l_node = invert(var(1));
//     // check data field
//     assert(std::get_if<invert_t>(&l_node.m_data));
// }

// void test_disjoin_construct_and_equality_check()
// {
//     assert(disjoin(var(0), var(1)) ==
//            disjoin(var(0), var(1)));
//     assert(disjoin(var(0), var(1)) !=
//            disjoin(var(1), var(1)));
//     assert(disjoin(var(0), var(1)) !=
//            disjoin(var(0), var(2)));
//     bool_node l_node = disjoin(var(0), var(1));
//     // check data field
//     assert(std::get_if<disjoin_t>(&l_node.m_data));
// }

// void test_conjoin_construct_and_equality_check()
// {
//     assert(conjoin(var(0), var(1)) ==
//            conjoin(var(0), var(1)));
//     assert(conjoin(var(0), var(1)) !=
//            conjoin(var(1), var(1)));
//     assert(conjoin(var(0), var(1)) !=
//            conjoin(var(0), var(2)));
//     bool_node l_node = conjoin(var(0), var(1));
//     // check data field
//     assert(std::get_if<conjoin_t>(&l_node.m_data));
// }

// void test_helper_construct_and_equality_check()
// {
//     assert(helper(0, {var(0), var(1)}) ==
//            helper(0, {var(0), var(1)}));
//     assert(helper(0, {var(0), var(1)}) !=
//            helper(1, {var(0), var(1)}));
//     assert(helper(0, {var(0), var(1)}) !=
//            helper(0, {var(1), var(1)}));
//     assert(helper(0, {var(0), var(1)}) !=
//            helper(0, {var(0), var(0)}));
//     assert(helper(0, {var(0), var(1)}) !=
//            helper(0, {var(0), var(0), var(1)}));
//     bool_node l_node = helper(2, {var(0), var(1)});
//     // check data field
//     helper_t* l_data;
//     assert(l_data =
//     std::get_if<helper_t>(&l_node.m_data));
//     assert(l_data->m_index == 2);
// }

// void test_bool_node_ostream_inserter()
// {
//     {
//         bool_node l_node = zero();
//         std::stringstream l_ss;
//         l_ss << l_node;
//         assert(l_ss.str() == "low");
//     }

//     {
//         bool_node l_node = one();
//         std::stringstream l_ss;
//         l_ss << l_node;
//         assert(l_ss.str() == "high");
//     }

//     {
//         bool_node l_node = var(3);
//         std::stringstream l_ss;
//         l_ss << l_node;
//         assert(l_ss.str() == "3");
//     }

//     {
//         bool_node l_node = invert(var(4));
//         std::stringstream l_ss;
//         l_ss << l_node;
//         assert(l_ss.str() == "~{4}");
//     }

//     {
//         bool_node l_node = disjoin(var(5), var(6));
//         std::stringstream l_ss;
//         l_ss << l_node;
//         assert(l_ss.str() == "[5]|[6]");
//     }

//     {
//         bool_node l_node = conjoin(var(5), var(6));
//         std::stringstream l_ss;
//         l_ss << l_node;
//         assert(l_ss.str() == "(5)&(6)");
//     }
// }

// // void test_build_function()
// // {

// //     struct test_data
// //     {
// //         uint32_t m_rnd_gen_seed;
// //         std::vector<size_t> m_helper_arities;
// //         size_t m_desired_func_arity;
// //         bool_node m_desired_func;
// //     };

// //     std::list<test_data> l_examples{
// //         test_data{
// //             17,
// //             {},
// //             0,
// //             one(),
// //         },
// //         test_data{
// //             18,
// //             {},
// //             0,
// //             disjoin(zero(),
// //                     invert(invert(conjoin(one(),
// //                     zero())))),
// //         },
// //         test_data{
// //             20,
// //             {},
// //             4,
// //             var(3),
// //         },
// //         test_data{
// //             23,
// //             {
// //                 1,
// //             },
// //             0,
// //             disjoin(conjoin(helper(0,
// //             {one()}),conjoin(conjoin(one(),
// //             disjoin(zero(), zero()))))),
// //         },
// //         test_data{
// //             24,
// //             {
// //                 1,
// //                 2,
// //                 3,
// //                 4,
// //             },
// //             4,
// //             helper(
// //                 3,
// //                 {var(0), helper(0, {zero()}),
// //                  helper(
// //                      3,
// //                      {
// //                          var(1),
// //                          var(0),
// //                          helper(
// //                              3,
// //                              {
// //                                  var(2),
// //                                  invert(
// //                                      helper(0,
// //                                      {var(2)})),
// //                                  helper(3,
// //                                         {
// // invert(one()),
// // conjoin(zero(),
// // one()),
// //                                             var(2),
// // disjoin(one(),
// // zero()),
// //                                         }),
// //                                  var(2),
// //                              }),
// //                          conjoin(
// //                              helper(0, {var(3)}),
// //                              helper(2,
// //                                     {
// //                                         helper(2,
// //                                                {
// // var(1),
// // var(3),
// // one(),
// //                                                }),
// //                                         var(1),
// // conjoin(var(1),
// // var(1)),
// //                                     })),
// //                      }),
// //                  invert(invert(
// //                      helper(3,
// //                             {
// //                                 helper(2, {var(1),
// //                                 var(3),
// //                                            var(0)}),
// //                                 var(2),
// //                                 invert(var(2)),
// //                                 invert(var(2)),
// //                             })))}),
// //         },
// //     };

// //     for(const test_data& l_example : l_examples)
// //     {
// //         std::mt19937
// l_rnd_gen(l_example.m_rnd_gen_seed);
// //         monte_carlo::tree_node<choice_t> l_root;
// //         monte_carlo::simulation<choice_t,
// std::mt19937>
// //             l_sim(l_root, 5, l_rnd_gen);
// //         bool_node l_func = build_function(
// //             l_example.m_desired_func_arity,
// //             l_example.m_helper_arities, l_sim, 5);
// //         std::cout << l_func << std::endl;
// //         assert(l_func == l_example.m_desired_func);
// //     }
// // }

// void test_build_model()
// {
//     std::mt19937 l_rnd_gen(5);
//     monte_carlo::tree_node<choice_t> l_root;
//     monte_carlo::simulation<choice_t, std::mt19937>
//     l_sim(
//         l_root, 5, l_rnd_gen);

//     // while(true)
//     // {

//     //     std::vector<size_t>    l_helper_arities;
//     //     std::vector<bool_node> l_helpers;

//     //     bool_node l_model = build_model(10,
//     //     l_helper_arities, l_helpers, l_sim, 5);

//     //     for (const auto& l_fn : l_helpers)
//     //         std::cout << l_fn << std::endl;

//     //     std::cout << l_model << std::endl <<
//     std::endl;

//     // }
// }

// void test_evaluate()
// {
//     struct test_data
//     {
//         std::vector<function> m_helpers;
//         bool_node m_model;
//         std::vector<bool> m_x;
//         bool m_y;
//     };

//     std::list<test_data> l_examples{
//         {
//             {},
//             zero(),
//             {0, 0, 0},
//             0,
//         },
//         {
//             {},
//             one(),
//             {0, 0, 0},
//             1,
//         },
//         {
//             {},
//             var(0),
//             {0, 0, 0},
//             0,
//         },
//         {
//             {},
//             var(0),
//             {1, 0, 0},
//             1,
//         },
//         {
//             {},
//             var(1),
//             {1, 0, 0},
//             0,
//         },
//         {
//             {},
//             var(1),
//             {1, 1, 0},
//             1,
//         },
//         {
//             {},
//             invert(zero()),
//             {1, 0, 0},
//             1,
//         },
//         {
//             {},
//             invert(one()),
//             {1, 0, 0},
//             0,
//         },
//         {
//             {},
//             disjoin(zero(), zero()),
//             {1, 0, 0},
//             0,
//         },
//         {
//             {},
//             disjoin(zero(), one()),
//             {1, 0, 0},
//             1,
//         },
//         {
//             {},
//             disjoin(one(), zero()),
//             {1, 0, 0},
//             1,
//         },
//         {
//             {},
//             disjoin(one(), one()),
//             {1, 0, 0},
//             1,
//         },
//         {
//             {},
//             conjoin(zero(), zero()),
//             {1, 0, 0},
//             0,
//         },
//         {
//             {},
//             conjoin(zero(), one()),
//             {1, 0, 0},
//             0,
//         },
//         {
//             {},
//             conjoin(one(), zero()),
//             {1, 0, 0},
//             0,
//         },
//         {
//             {},
//             conjoin(one(), one()),
//             {1, 0, 0},
//             1,
//         },
//         {
//             // demonstrate helper captures
//             {
//                 function{0, var(2), {}},
//             },
//             helper(0, {}),
//             {1, 0, 0},
//             0,
//         },
//         {
//             // demonstrate helper captures
//             {
//                 function{0, var(2), {}},
//             },
//             helper(0, {}),
//             {1, 0, 1},
//             1,
//         },
//         {
//             // demonstrate helper parameters
//             {
//                 function{1, var(3), {}},
//             },
//             helper(1, {var(1)}),
//             {0, 1, 0},
//             1,
//         },
//         {
//             // demonstrate helper uses captured val over
//             // param
//             {
//                 function{1, var(0), {}},
//                 function{3, var(5), {}},
//             },
//             helper(0, {var(2)}),
//             {0, 1, 1},
//             0,
//         },
//     };

//     for(const auto& l_example : l_examples)
//     {
//         assert(evaluate(l_example.m_model,
//                         l_example.m_helpers,
//                         l_example.m_x) == l_example.m_y);
//     }
// }

// int string_length(const std::string& a_string)
// {
//     return a_string.size();
// }

// void test_learn_model()
// {
//     // learn nested exor
//     {
//         constexpr size_t ITERATIONS = 10000;

//         // nested exor data
//         // 8 rows
//         std::vector<std::pair<std::vector<std::any>, bool>> l_data{
//             {{false, false, false}, false}, {{false, false, true}, true},
//             {{false, true, false}, true},   {{false, true, true}, false},
//             {{true, false, false}, true}, //{{true, false, true}, false},
//             {{true, true, false}, false},   {{true, true, true}, true},
//         };

//         // initialize the program and scope
//         program l_program;
//         scope l_scope;

//         // add some primitive functions
//         std::function l_exor = std::function(
//             [](bool a_x, bool a_y) { return !a_x && a_y || a_x && !a_y; });
//         std::function l_exor_3 =
//             std::function([l_exor](bool a_x, bool a_y, bool a_z)
//                           { return l_exor(l_exor(a_x, a_y), a_z); });

//         // add two-way exor
//         l_scope.add_function(l_program.add_primitive("exor", l_exor));

//         // add three-way exor
//         l_scope.add_function(l_program.add_primitive("exor_3", l_exor_3));

//         // learn a model
//         model l_model = learn_model<bool, bool, bool>(
//             l_program, l_scope, l_data, ITERATIONS, 10, 100);
//     }

//     // learn a&&(b exor c exor d)
//     {
//         constexpr size_t ITERATIONS = 10000;

//         // nested exor data
//         // 16 rows
//         std::vector<std::pair<std::vector<std::any>, bool>> l_data{
//             {{false, false, false, false}, false},
//             {{false, false, false, true}, false},
//             {{false, false, true, false}, false},
//             // {{false, false, true, true}, false},
//             {{false, true, false, false}, false},
//             {{false, true, false, true}, false},
//             {{false, true, true, false}, false},
//             {{false, true, true, true}, false},
//             {{true, false, false, false}, false},
//             {{true, false, false, true}, true},
//             // {{true, false, true, false}, true},
//             {{true, false, true, true}, false},
//             {{true, true, false, false}, true},
//             {{true, true, false, true}, false},
//             // {{true, true, true, false}, false},
//             {{true, true, true, true}, true},
//         };

//         // initialize the program and scope
//         program l_program;
//         scope l_scope;

//         // add some primitive functions
//         std::function l_exor = std::function(
//             [](bool a_x, bool a_y) { return !a_x && a_y || a_x && !a_y; });

//         std::function l_and =
//             std::function([](bool a_x, bool a_y) { return a_x && a_y; });

//         // add two-way exor
//         l_scope.add_function(l_program.add_primitive("exor", l_exor));

//         // add three-way exor
//         l_scope.add_function(l_program.add_primitive("and", l_and));

//         // learn a model
//         model l_model = learn_model<bool, bool, bool, bool>(
//             l_program, l_scope, l_data, ITERATIONS, 10, 100);
//     }

//     // learn x > 0 && x < 3 function
//     {
//         constexpr size_t ITERATIONS = 1000;

//         // x > 0 && x < 3 data
//         // 8 rows
//         std::vector<std::pair<std::vector<std::any>, bool>> l_data{
//             {{-3}, false}, {{-2}, false}, {{-1}, false}, {{0}, false},
//             {{1}, true},   {{2}, true},   {{3}, false},  {{4}, false},
//             {{5}, false},  {{6}, false},
//         };

//         // initialize the program and scope
//         program l_program;
//         scope l_scope;

//         // add primitive for 0
//         l_scope.add_function(
//             l_program.add_primitive("0", std::function([]() { return 0; })));

//         // add primitive for succ(n)
//         l_scope.add_function(l_program.add_primitive(
//             "succ", std::function([](int a_n) { return a_n + 1; })));

//         // add primitive for >
//         l_scope.add_function(l_program.add_primitive(
//             ">", std::function([](int a_x, int a_y) { return a_x > a_y; })));

//         // add primitive for <
//         l_scope.add_function(l_program.add_primitive(
//             "<", std::function([](int a_x, int a_y) { return a_x < a_y; })));

//         // add primitive for &&
//         l_scope.add_function(l_program.add_primitive(
//             "&&", std::function([](int a_x, int a_y) { return a_x && a_y;
//             })));

//         // learn a model
//         model l_model =
//             learn_model<int>(l_program, l_scope, l_data, ITERATIONS, 10,
//             100);
//     }

//     // learn x^2 < y
//     {
//         constexpr size_t ITERATIONS = 1000;

//         // x^2 < y data
//         std::vector<std::pair<std::vector<std::any>, bool>> l_data{
//             {{0, 0}, false},  {{0, 1}, true},  {{0, 2}, true},  {{1, 0},
//             false},
//             {{1, 2}, true},   {{2, 3}, false}, {{2, 4}, false}, {{2, 5},
//             true},
//             {{7, 49}, false}, {{7, 50}, true},
//         };

//         // initialize the program and scope
//         program l_program;
//         scope l_scope;

//         // add primitive for 0
//         l_scope.add_function(
//             l_program.add_primitive("0", std::function([]() { return 0; })));

//         // add primitive for succ(n)
//         l_scope.add_function(l_program.add_primitive(
//             "succ", std::function([](int a_n) { return a_n + 1; })));

//         // add primitive for square(n)
//         l_scope.add_function(l_program.add_primitive(
//             "square", std::function([](int a_n) { return a_n * a_n; })));

//         // add primitive for >
//         l_scope.add_function(l_program.add_primitive(
//             ">", std::function([](int a_x, int a_y) { return a_x > a_y; })));

//         // add primitive for <
//         l_scope.add_function(l_program.add_primitive(
//             "<", std::function([](int a_x, int a_y) { return a_x < a_y; })));

//         // add primitive for &&
//         l_scope.add_function(l_program.add_primitive(
//             "&&", std::function([](int a_x, int a_y) { return a_x && a_y;
//             })));

//         // learn a model
//         model l_model = learn_model<int, int>(l_program, l_scope, l_data,
//                                               ITERATIONS, 10, 100);
//     }

//     // learn xy < y
//     {
//         constexpr size_t ITERATIONS = 10000;

//         // xy < y data
//         std::vector<std::pair<std::vector<std::any>, bool>> l_data{
//             {{0, 0}, false}, {{0, 1}, true},   {{0, 2}, true},  {{1, 1},
//             false},
//             {{1, 2}, false}, {{1, 3}, false},  {{2, 1}, false}, {{2, 2},
//             false},
//             {{-1, 1}, true}, {{-10, 1}, true},
//         };

//         // initialize the program and scope
//         program l_program;
//         scope l_scope;

//         // add primitive for 0
//         l_scope.add_function(
//             l_program.add_primitive("0", std::function([]() { return 0; })));

//         // add primitive for succ(n)
//         l_scope.add_function(l_program.add_primitive(
//             "succ", std::function([](int a_n) { return a_n + 1; })));

//         // add primitive for square(n)
//         l_scope.add_function(l_program.add_primitive(
//             "square", std::function([](int a_n) { return a_n * a_n; })));

//         // add primitive for mul(n, m)
//         l_scope.add_function(l_program.add_primitive(
//             "*", std::function([](int a_n, int a_m) { return a_n * a_m; })));

//         // add primitive for >
//         l_scope.add_function(l_program.add_primitive(
//             ">", std::function([](int a_x, int a_y) { return a_x > a_y; })));

//         // add primitive for <
//         l_scope.add_function(l_program.add_primitive(
//             "<", std::function([](int a_x, int a_y) { return a_x < a_y; })));

//         // add primitive for &&
//         l_scope.add_function(l_program.add_primitive(
//             "&&", std::function([](int a_x, int a_y) { return a_x && a_y;
//             })));

//         // learn a model
//         model l_model = learn_model<int, int>(l_program, l_scope, l_data,
//                                               ITERATIONS, 10, 1000);
//     }

//     // learn string length < 5
//     {
//         constexpr size_t ITERATIONS = 1000;

//         // string length < 5 data
//         std::vector<std::pair<std::string, bool>> l_og_data{
//             {{""}, true},        {{"a"}, true},        {{"ab"}, true},
//             {{"abc"}, true},     {{"abcd"}, true},     {{"abcde"}, false},
//             {{"abcdef"}, false}, {{"abcdefg"}, false},
//         };

//         // convert the data to a vector of pairs of vectors of any and bool
//         std::vector<std::pair<std::vector<std::any>, bool>> l_data;
//         for(const auto& l_example : l_og_data)
//             l_data.emplace_back(std::vector<std::any>{l_example.first},
//                                 l_example.second);

//         // initialize the program and scope
//         program l_program;
//         scope l_scope;

//         // add primitive for 0
//         l_scope.add_function(
//             l_program.add_primitive("0", std::function([]() { return 0; })));

//         // add primitive for succ(n)
//         l_scope.add_function(l_program.add_primitive(
//             "succ", std::function([](int a_n) { return a_n + 1; })));

//         // add primitive for >
//         l_scope.add_function(l_program.add_primitive(
//             ">", std::function([](int a_x, int a_y) { return a_x > a_y; })));

//         // add primitive for <
//         l_scope.add_function(l_program.add_primitive(
//             "<", std::function([](int a_x, int a_y) { return a_x < a_y; })));

//         // add primitive for string length
//         l_scope.add_function(l_program.add_primitive(
//             "string_length", std::function(string_length)));

//         // learn a model
//         model l_model = learn_model<std::string>(l_program, l_scope, l_data,
//                                                  ITERATIONS, 10, 1000);
//     }

//     // learn 2 < string length < 5
//     {
//         constexpr size_t ITERATIONS = 1000;

//         // 2 < string length < 5 data
//         std::vector<std::pair<std::string, bool>> l_og_data{
//             {{""}, false},       {{"a"}, false},       {{"ab"}, false},
//             {{"abc"}, true},     {{"abcd"}, true},     {{"abcde"}, false},
//             {{"abcdef"}, false}, {{"abcdefg"}, false},
//         };

//         // convert the data to a vector of pairs of vectors of any and bool
//         std::vector<std::pair<std::vector<std::any>, bool>> l_data;
//         for(const auto& l_example : l_og_data)
//             l_data.emplace_back(std::vector<std::any>{l_example.first},
//                                 l_example.second);

//         // initialize the program and scope
//         program l_program;
//         scope l_scope;

//         // add primitive for 0
//         l_scope.add_function(
//             l_program.add_primitive("0", std::function([]() { return 0; })));

//         // add primitive for succ(n)
//         l_scope.add_function(l_program.add_primitive(
//             "succ", std::function([](int a_n) { return a_n + 1; })));

//         // add primitive for >
//         l_scope.add_function(l_program.add_primitive(
//             ">", std::function([](int a_x, int a_y) { return a_x > a_y; })));

//         // add primitive for <
//         l_scope.add_function(l_program.add_primitive(
//             "<", std::function([](int a_x, int a_y) { return a_x < a_y; })));

//         // add primitive for string length
//         l_scope.add_function(l_program.add_primitive(
//             "string_length", std::function(string_length)));

//         // learn a model
//         model l_model = learn_model<std::string>(l_program, l_scope, l_data,
//                                                  ITERATIONS, 10, 1000);
//     }

//     // // learn v[4] == param
//     // {
//     //     constexpr size_t ITERATIONS = 10000;

//     //     // 2 < string length < 5 data
//     //     std::vector<std::pair<std::tuple<std::vector<int>, int>, bool>>
//     //         l_og_data{
//     //             {{{1, 1, 12, 13, 1}, 1}, true},
//     //             {{{2, 5, 1, 9, 4}, 5}, false},
//     //             {{{4, 2, 2, 5, 4}, 2}, false},
//     //             {{{6, 8, 20, 2, 7, 8, 9}, 7}, true},
//     //             {{{7, 1, 7, 31, 7, 8, 9}, 3}, false},
//     //         };

//     //     // convert the data to a vector of pairs of vectors of any and
//     bool
//     //     std::vector<std::pair<std::vector<std::any>, bool>> l_data;
//     //     for(const auto& l_example : l_og_data)
//     //         l_data.emplace_back(
//     //             std::vector<std::any>{std::get<0>(l_example.first),
//     //                                   std::get<1>(l_example.first)},
//     //             l_example.second);

//     //     // initialize the program and scope
//     //     program l_program;
//     //     scope l_scope;

//     //     // add primitive for 0
//     //     l_scope.add_function(
//     //         l_program.add_primitive("0", std::function([]() { return 0;
//     })));

//     //     // add primitive for succ(n)
//     //     l_scope.add_function(l_program.add_primitive(
//     //         "succ", std::function([](int a_n) { return a_n + 1; })));

//     //     // add primitive for v[i]
//     //     l_scope.add_function(l_program.add_primitive(
//     //         "index", std::function(
//     //                      [](std::vector<int> a_v, int a_i)
//     //                      {
//     //                          // compute the index by modulus
//     //                          int l_index = a_i % a_v.size();
//     //                          return a_v[l_index];
//     //                      })));

//     //     // add primitive for ==
//     //     l_scope.add_function(l_program.add_primitive(
//     //         "<", std::function([](int a_x, int a_y) { return a_x < a_y;
//     })));

//     //     // learn a model
//     //     model l_model = learn_model<std::vector<int>, int>(
//     //         l_program, l_scope, l_data, ITERATIONS, 10, 1000);
//     // }
// }

// void reduce_test_main()
// {
//     constexpr bool ENABLE_DEBUG_LOGS = true;

//     // TEST(test_zero_construct_and_equality_check);
//     // TEST(test_one_construct_and_equality_check);
//     // TEST(test_var_construct_and_equality_check);
//     // TEST(test_invert_construct_and_equality_check);
//     // TEST(test_disjoin_construct_and_equality_check);
//     // TEST(test_conjoin_construct_and_equality_check);
//     // TEST(test_helper_construct_and_equality_check);
//     // TEST(test_bool_node_ostream_inserter);
//     // TEST(test_build_function);
//     // TEST(test_build_model);
//     // TEST(test_evaluate);
//     TEST(test_learn_model);
// }

#endif
