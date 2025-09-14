#include "../include/reduce.hpp"
#include "../include/model.hpp"
#include "../include/program.hpp"
#include "../include/scope.hpp"
#include "../mcts/include/mcts.hpp"
#include <iostream>
#include <random>
#include <sstream>

////////////////////////////////////////////////////
//////////////// COMPARISON OPERATORS //////////////
////////////////////////////////////////////////////

bool operator<(const place_functor_node& a_lhs, const place_functor_node& a_rhs)
{
    return a_lhs.m_func < a_rhs.m_func;
}
bool operator<(const place_param_node& a_lhs, const place_param_node& a_rhs)
{
    return false;
}
bool operator<(const terminate&, const terminate&)
{
    return false;
}
bool operator<(const make_function&, const make_function&)
{
    return false;
}

////////////////////////////////////////////////////
//////////////// FUNCTION GENERATION ///////////////
////////////////////////////////////////////////////

func_body
build_function(program& a_program, scope& a_scope,
               std::multimap<std::type_index, size_t>& a_param_types,
               std::stringstream& a_repr_stream,
               const std::type_index& a_return_type,
               const bool& a_allow_adding_params,
               monte_carlo::simulation<choice, std::mt19937>& a_simulation,
               const size_t& a_recursion_limit)
{
    ////////////////////////////////////////////////////
    /////////////// GET THE SCOPE RANGES ///////////////
    ////////////////////////////////////////////////////
    const auto& l_nullary_range =
        a_scope.m_nullaries.equal_range(a_return_type);
    const auto& l_non_nullary_range =
        a_scope.m_non_nullaries.equal_range(a_return_type);

    ////////////////////////////////////////////////////
    //////////////// GET THE PARAM RANGE ///////////////
    ////////////////////////////////////////////////////
    const auto& l_param_range = a_param_types.equal_range(a_return_type);

    ////////////////////////////////////////////////////
    ////////////// POPULATE CHOICE VECTOR //////////////
    ////////////////////////////////////////////////////
    std::vector<choice> l_node_choices;

    // allow choosing any nullary of this type
    std::transform(l_nullary_range.first, l_nullary_range.second,
                   std::back_inserter(l_node_choices), [](auto a_nullary)
                   { return place_functor_node{a_nullary.second}; });

    // if the recursion limit has not been reached,
    // push non-nullary function types into list
    if(a_recursion_limit > 0)
    {
        std::transform(l_non_nullary_range.first, l_non_nullary_range.second,
                       std::back_inserter(l_node_choices),
                       [](auto a_non_nullary)
                       { return place_functor_node{a_non_nullary.second}; });
    }

    // allow choosing any known param of this type
    std::transform(l_param_range.first, l_param_range.second,
                   std::back_inserter(l_node_choices), [](auto a_entry)
                   { return place_param_node{a_entry.second}; });

    // allow choosing the next param of this type
    if(a_allow_adding_params)
        l_node_choices.push_back(place_param_node{a_param_types.size()});

    ////////////////////////////////////////////////////
    ////////////// CHOOSE A NODE TO PLACE //////////////
    ////////////////////////////////////////////////////
    choice l_node_choice = a_simulation.choose(l_node_choices);

    // if the choice is a place_param_node
    if(const auto& l_place_param_node =
           std::get_if<place_param_node>(&l_node_choice))
    {
        // if the choice is a new param
        if(l_place_param_node->m_index == a_param_types.size())
        {
            // add the param to the param types
            a_param_types.insert({a_return_type, l_place_param_node->m_index});
        }

        // add the param's representation to the stream
        a_repr_stream << "?" << l_place_param_node->m_index;

        // regardless, return the param node
        return func_body{
            .m_data = param{l_place_param_node->m_index},
            .m_children = {},
        };
    }

    // extract the func
    auto l_node_func = std::get<place_functor_node>(l_node_choice).m_func;

    // add the node's representation to the stream
    a_repr_stream << l_node_func->m_repr << "(";

    ////////////////////////////////////////////////////
    //////////////// CONSTRUCT CHILDREN ////////////////
    ////////////////////////////////////////////////////
    size_t l_node_arity = l_node_func->m_param_types.size();

    // pre-allocate the args vector
    std::vector<func_body> l_node_children(l_node_func->m_param_types.size());

    // loop through with iterator, construct args in place
    for(auto l_param_type_it = l_node_func->m_param_types.begin();
        l_param_type_it != l_node_func->m_param_types.end(); ++l_param_type_it)
    {
        l_node_children[l_param_type_it->second] =
            build_function(a_program, a_scope, a_param_types, a_repr_stream,
                           l_param_type_it->first, a_allow_adding_params,
                           a_simulation, a_recursion_limit - 1);

        // if this is not the last param, add a comma
        if(std::next(l_param_type_it) != l_node_func->m_param_types.end())
            a_repr_stream << ",";
    }

    a_repr_stream << ")";

    // create an invoking lambda
    auto l_functor =
        [l_node_func](const std::any* a_params, size_t a_param_count)
    { return l_node_func->m_body.eval(a_params, a_param_count); };

    ////////////////////////////////////////////////////
    //////////// CONSTRUCT THE FUNC_NODE_T /////////////
    ////////////////////////////////////////////////////
    return func_body{
        .m_data = l_functor,
        .m_children = l_node_children,
    };
}

model build_model(
    program& a_program, scope& a_scope,
    std::multimap<std::type_index, size_t>& a_param_types,
    const std::vector<std::pair<std::vector<std::any>, bool>>& a_data,
    monte_carlo::simulation<choice, std::mt19937>& a_simulation,
    const size_t& a_recursion_limit)
{
    ////////////////////////////////////////////////////
    //////////////// CHECK FOR TRIVIALITY //////////////
    ////////////////////////////////////////////////////
    if(a_data.empty())
        throw std::runtime_error("Error: no data points to build model from.");

    ////////////////////////////////////////////////////
    //////////////// CHECK FOR HOMOGENEITY /////////////
    ////////////////////////////////////////////////////

    // get the first label
    bool l_homogenous_value = a_data.begin()->second;

    // loop through the data points, check for homogeneity
    bool l_data_is_homogenous =
        std::all_of(a_data.begin(), a_data.end(),
                    [l_homogenous_value](const auto& a_data_point)
                    { return a_data_point.second == l_homogenous_value; });

    // if the data is homogenous, return the appropriate
    // constant
    if(l_data_is_homogenous)
        return model{.m_homogenous_value = l_homogenous_value};

    ////////////////////////////////////////////////////
    /////////////// CREATE BINNING FUNCTION ////////////
    ////////////////////////////////////////////////////

    // declare return type
    const std::type_index BINNING_RETURN_TYPE = std::type_index(typeid(bool));

    // construct the negative bin
    std::vector<std::pair<std::vector<std::any>, bool>> l_negative_bin;

    // construct the positive bin
    std::vector<std::pair<std::vector<std::any>, bool>> l_positive_bin;

    // declare the binning function body
    func_body l_binning_function_body;

    // construct the repr stream
    std::stringstream l_repr_stream;

    // create a copy of the original program
    program l_original_program = a_program;

    // loop until neither output bin is empty
    // REASON: if one of the bins is empty, the binning
    // function is useless
    while(l_negative_bin.empty() || l_positive_bin.empty())
    {
        // clear BOTH bins in case one contains items
        l_negative_bin.clear();
        l_positive_bin.clear();

        // clear the repr stream
        l_repr_stream.str("");

        // restore the original program
        a_program = l_original_program;

        // construct the binning function body
        // [create a binning function that will bin (evaluate
        // on) each data point]
        l_binning_function_body = build_function(
            a_program, a_scope, a_param_types, l_repr_stream,
            BINNING_RETURN_TYPE, false, a_simulation, a_recursion_limit);

        ////////////////////////////////////////////////////
        ////////////// EVALUATE BINNING FUNCTION ///////////
        ////////////////////////////////////////////////////

        // evaluate the binning function on all of the
        // data points
        for(const auto& [l_x, l_y] : a_data)
        {
            // evaluate the binning function (should return bool)
            bool l_binning_result = std::any_cast<bool>(
                l_binning_function_body.eval(l_x.data(), l_x.size()));

            // store in the appropriate bin
            if(l_binning_result)
                l_positive_bin.emplace_back(l_x, l_y);
            else
                l_negative_bin.emplace_back(l_x, l_y);
        }
    }

    // construct the function definition
    auto l_binning_function =
        std::make_shared<func>(typeid(bool), a_param_types,
                               l_binning_function_body, l_repr_stream.str());

    // add the binning function to the program
    a_program.m_funcs.push_back(l_binning_function);

    ////////////////////////////////////////////////////
    //////////////////////// RECUR /////////////////////
    ////////////////////////////////////////////////////

    // construct the negative child
    model l_negative_child =
        build_model(a_program, a_scope, a_param_types, l_negative_bin,
                    a_simulation, a_recursion_limit);

    // construct the positive child
    model l_positive_child =
        build_model(a_program, a_scope, a_param_types, l_positive_bin,
                    a_simulation, a_recursion_limit);

    // construct the final node
    return model{
        .m_func = l_binning_function.get(),
        .m_negative_child = std::make_shared<model>(l_negative_child),
        .m_positive_child = std::make_shared<model>(l_positive_child),
    };
}

model learn_model(
    program& a_program, scope& a_scope,
    std::multimap<std::type_index, size_t>& a_param_types,
    const std::vector<std::pair<std::vector<std::any>, bool>>& a_data,
    const size_t& a_iterations, const size_t& a_recursion_limit,
    const double& a_exploration_constant)
{
    std::mt19937 l_rnd_gen(27);
    monte_carlo::tree_node<choice> l_root;

    // initialize the best reward to the lowest possible
    // value
    double l_best_reward = -std::numeric_limits<double>::infinity();
    model l_best_model;

    // save the original program and scope
    program l_original_program = a_program;
    scope l_original_scope = a_scope;

    for(int i = 0; i < a_iterations; ++i)
    {
        // construct the simulation
        monte_carlo::simulation<choice, std::mt19937> l_sim(
            l_root, a_exploration_constant, l_rnd_gen);

        // restore the original program and scope
        program l_program = l_original_program;
        scope l_scope = l_original_scope;

        // construct the model
        model l_model = build_model(l_program, l_scope, a_param_types, a_data,
                                    l_sim, a_recursion_limit);

        // compute the number of nodes in the whole program
        size_t l_program_node_count =
            std::accumulate(l_program.m_funcs.begin(), l_program.m_funcs.end(),
                            size_t{0}, [](size_t a_acc, const auto& a_func)
                            { return a_acc + a_func->m_body.node_count(); });

        // compute the number of nodes in the model
        size_t l_model_node_count = l_model.node_count();

        // compute the reward (negative number of nodes)
        double l_reward =
            -static_cast<double>(l_program_node_count + l_model_node_count);

        // save best model
        if(l_reward > l_best_reward)
        {
            l_best_reward = l_reward;
            a_program = l_program;
            a_scope = l_scope;
            l_best_model = l_model;

            std::cout << l_program_node_count << " " << l_reward << std::endl;

            std::cout << "program: " << std::endl;
            for(const auto& l_func : l_program.m_funcs)
                std::cout << "    " << l_func->m_repr << std::endl;
            std::cout << std::endl;
        }

        // terminate the simulation
        l_sim.terminate(l_reward);
    }

    return l_best_model;
}

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

void test_learn_model()
{
    // learn nested exor
    {
        constexpr size_t ITERATIONS = 10000;

        // nested exor data
        // 8 rows
        std::vector<std::pair<std::vector<std::any>, bool>> l_data{
            {{false, false, false}, false}, {{false, false, true}, true},
            {{false, true, false}, true},   {{false, true, true}, false},
            {{true, false, false}, true}, //{{true, false, true}, false},
            {{true, true, false}, false},   {{true, true, true}, true},
        };

        // set the input type
        std::multimap<std::type_index, size_t> l_input_types{
            {typeid(bool), 0},
            {typeid(bool), 1},
            {typeid(bool), 2},
        };

        // initialize the program and scope
        program l_program;
        scope l_scope;

        // add some primitive functions
        std::function l_exor = std::function(
            [](bool a_x, bool a_y) { return !a_x && a_y || a_x && !a_y; });
        std::function l_exor_3 =
            std::function([l_exor](bool a_x, bool a_y, bool a_z)
                          { return l_exor(l_exor(a_x, a_y), a_z); });

        // add two-way exor
        l_scope.add_function(l_program.add_primitive("exor", l_exor));

        // add three-way exor
        l_scope.add_function(l_program.add_primitive("exor_3", l_exor_3));

        // learn a model
        model l_model = learn_model(l_program, l_scope, l_input_types, l_data,
                                    ITERATIONS, 10, 100);
    }

    // learn a&&(b exor c exor d)
    {
        constexpr size_t ITERATIONS = 10000;

        // nested exor data
        // 16 rows
        std::vector<std::pair<std::vector<std::any>, bool>> l_data{
            {{false, false, false, false}, false},
            {{false, false, false, true}, false},
            {{false, false, true, false}, false},
            // {{false, false, true, true}, false},
            {{false, true, false, false}, false},
            {{false, true, false, true}, false},
            {{false, true, true, false}, false},
            {{false, true, true, true}, false},
            {{true, false, false, false}, false},
            {{true, false, false, true}, true},
            // {{true, false, true, false}, true},
            {{true, false, true, true}, false},
            {{true, true, false, false}, true},
            {{true, true, false, true}, false},
            // {{true, true, true, false}, false},
            {{true, true, true, true}, true},
        };

        // set the input type
        std::multimap<std::type_index, size_t> l_input_types{
            {typeid(bool), 0},
            {typeid(bool), 1},
            {typeid(bool), 2},
            {typeid(bool), 3},
        };

        // initialize the program and scope
        program l_program;
        scope l_scope;

        // add some primitive functions
        std::function l_exor = std::function(
            [](bool a_x, bool a_y) { return !a_x && a_y || a_x && !a_y; });

        std::function l_and =
            std::function([](bool a_x, bool a_y) { return a_x && a_y; });

        // add two-way exor
        l_scope.add_function(l_program.add_primitive("exor", l_exor));

        // add three-way exor
        l_scope.add_function(l_program.add_primitive("and", l_and));

        // learn a model
        model l_model = learn_model(l_program, l_scope, l_input_types, l_data,
                                    ITERATIONS, 10, 100);
    }

    // learn x > 0 && x < 3 function
    {
        constexpr size_t ITERATIONS = 1000;

        // x > 0 && x < 3 data
        // 8 rows
        std::vector<std::pair<std::vector<std::any>, bool>> l_data{
            {{-3}, false}, {{-2}, false}, {{-1}, false}, {{0}, false},
            {{1}, true},   {{2}, true},   {{3}, false},  {{4}, false},
            {{5}, false},  {{6}, false},
        };

        // set the input type
        std::multimap<std::type_index, size_t> l_input_types{
            {typeid(int), 0},
        };

        // initialize the program and scope
        program l_program;
        scope l_scope;

        // add primitive for 0
        l_scope.add_function(
            l_program.add_primitive("0", std::function([]() { return 0; })));

        // add primitive for succ(n)
        l_scope.add_function(l_program.add_primitive(
            "succ", std::function([](int a_n) { return a_n + 1; })));

        // add primitive for >
        l_scope.add_function(l_program.add_primitive(
            ">", std::function([](int a_x, int a_y) { return a_x > a_y; })));

        // add primitive for <
        l_scope.add_function(l_program.add_primitive(
            "<", std::function([](int a_x, int a_y) { return a_x < a_y; })));

        // add primitive for &&
        l_scope.add_function(l_program.add_primitive(
            "&&", std::function([](int a_x, int a_y) { return a_x && a_y; })));

        // learn a model
        model l_model = learn_model(l_program, l_scope, l_input_types, l_data,
                                    ITERATIONS, 10, 100);
    }

    // learn x^2 < y
    {
        constexpr size_t ITERATIONS = 1000;

        // x^2 < y data
        std::vector<std::pair<std::vector<std::any>, bool>> l_data{
            {{0, 0}, false},  {{0, 1}, true},  {{0, 2}, true},  {{1, 0}, false},
            {{1, 2}, true},   {{2, 3}, false}, {{2, 4}, false}, {{2, 5}, true},
            {{7, 49}, false}, {{7, 50}, true},
        };

        // set the input type
        std::multimap<std::type_index, size_t> l_input_types{
            {typeid(int), 0},
            {typeid(int), 1},
        };

        // initialize the program and scope
        program l_program;
        scope l_scope;

        // add primitive for 0
        l_scope.add_function(
            l_program.add_primitive("0", std::function([]() { return 0; })));

        // add primitive for succ(n)
        l_scope.add_function(l_program.add_primitive(
            "succ", std::function([](int a_n) { return a_n + 1; })));

        // add primitive for square(n)
        l_scope.add_function(l_program.add_primitive(
            "square", std::function([](int a_n) { return a_n * a_n; })));

        // add primitive for >
        l_scope.add_function(l_program.add_primitive(
            ">", std::function([](int a_x, int a_y) { return a_x > a_y; })));

        // add primitive for <
        l_scope.add_function(l_program.add_primitive(
            "<", std::function([](int a_x, int a_y) { return a_x < a_y; })));

        // add primitive for &&
        l_scope.add_function(l_program.add_primitive(
            "&&", std::function([](int a_x, int a_y) { return a_x && a_y; })));

        // learn a model
        model l_model = learn_model(l_program, l_scope, l_input_types, l_data,
                                    ITERATIONS, 10, 100);
    }

    // learn xy < y
    {
        constexpr size_t ITERATIONS = 10000;

        // xy < y data
        std::vector<std::pair<std::vector<std::any>, bool>> l_data{
            {{0, 0}, false}, {{0, 1}, true},   {{0, 2}, true},  {{1, 1}, false},
            {{1, 2}, false}, {{1, 3}, false},  {{2, 1}, false}, {{2, 2}, false},
            {{-1, 1}, true}, {{-10, 1}, true},
        };

        // set the input type
        std::multimap<std::type_index, size_t> l_input_types{
            {typeid(int), 0},
            {typeid(int), 1},
        };

        // initialize the program and scope
        program l_program;
        scope l_scope;

        // add primitive for 0
        l_scope.add_function(
            l_program.add_primitive("0", std::function([]() { return 0; })));

        // add primitive for succ(n)
        l_scope.add_function(l_program.add_primitive(
            "succ", std::function([](int a_n) { return a_n + 1; })));

        // add primitive for square(n)
        l_scope.add_function(l_program.add_primitive(
            "square", std::function([](int a_n) { return a_n * a_n; })));

        // add primitive for mul(n, m)
        l_scope.add_function(l_program.add_primitive(
            "*", std::function([](int a_n, int a_m) { return a_n * a_m; })));

        // add primitive for >
        l_scope.add_function(l_program.add_primitive(
            ">", std::function([](int a_x, int a_y) { return a_x > a_y; })));

        // add primitive for <
        l_scope.add_function(l_program.add_primitive(
            "<", std::function([](int a_x, int a_y) { return a_x < a_y; })));

        // add primitive for &&
        l_scope.add_function(l_program.add_primitive(
            "&&", std::function([](int a_x, int a_y) { return a_x && a_y; })));

        // learn a model
        model l_model = learn_model(l_program, l_scope, l_input_types, l_data,
                                    ITERATIONS, 10, 1000);
    }
}

void reduce_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    // TEST(test_zero_construct_and_equality_check);
    // TEST(test_one_construct_and_equality_check);
    // TEST(test_var_construct_and_equality_check);
    // TEST(test_invert_construct_and_equality_check);
    // TEST(test_disjoin_construct_and_equality_check);
    // TEST(test_conjoin_construct_and_equality_check);
    // TEST(test_helper_construct_and_equality_check);
    // TEST(test_bool_node_ostream_inserter);
    // TEST(test_build_function);
    // TEST(test_build_model);
    // TEST(test_evaluate);
    TEST(test_learn_model);
}

#endif
