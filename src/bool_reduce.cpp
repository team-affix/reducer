#include "../include/bool_reduce.hpp"
#include "../mcts/include/mcts.hpp"
#include <iostream>
#include <iterator>
#include <map>
#include <random>

// using replacement_rule = std::pair<bool_node, bool_node>;

////////////////////////////////////////////////////
////////////////// NODE DATA TYPES /////////////////
////////////////////////////////////////////////////

// equality comparisons
bool operator==(const zero_t&, const zero_t&)
{
    return true;
}
bool operator==(const one_t&, const one_t&)
{
    return true;
}
bool operator==(const var_t& a_lhs, const var_t& a_rhs)
{
    return a_lhs.m_index == a_rhs.m_index;
}
bool operator==(const invert_t&, const invert_t&)
{
    return true;
}
bool operator==(const disjoin_t&, const disjoin_t&)
{
    return true;
}
bool operator==(const conjoin_t&, const conjoin_t&)
{
    return true;
}
bool operator==(const helper_t& a_lhs,
                const helper_t& a_rhs)
{
    return a_lhs.m_index == a_rhs.m_index;
}

// less than comparisons
bool operator<(const zero_t&, const zero_t&)
{
    return false;
}
bool operator<(const one_t&, const one_t&)
{
    return false;
}
bool operator<(const var_t& a_lhs, const var_t& a_rhs)
{
    return a_lhs.m_index < a_rhs.m_index;
}
bool operator<(const invert_t&, const invert_t&)
{
    return false;
}
bool operator<(const disjoin_t&, const disjoin_t&)
{
    return false;
}
bool operator<(const conjoin_t&, const conjoin_t&)
{
    return false;
}
bool operator<(const helper_t& a_lhs, const helper_t& a_rhs)
{
    return a_lhs.m_index < a_rhs.m_index;
}

////////////////////////////////////////////////////
////////////////// MAKER FUNCTIONS /////////////////
////////////////////////////////////////////////////

bool_node zero()
{
    return bool_node{zero_t{}, {}};
}
bool_node one()
{
    return bool_node{one_t{}, {}};
}
bool_node var(size_t a_index)
{
    return bool_node{var_t{a_index}, {}};
}
bool_node invert(const bool_node& a_x)
{
    return bool_node{invert_t{}, {a_x}};
}
bool_node disjoin(const bool_node& a_x,
                  const bool_node& a_y)
{
    return bool_node{disjoin_t{}, {a_x, a_y}};
}
bool_node conjoin(const bool_node& a_x,
                  const bool_node& a_y)
{
    return bool_node{conjoin_t{}, {a_x, a_y}};
}
bool_node helper(size_t a_index,
                 const std::vector<bool_node>& a_children)
{
    return bool_node{helper_t{a_index}, a_children};
}

////////////////////////////////////////////////////
////////////////// OSTREAM INSERTER ////////////////
////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& a_ostream,
                         const bool_node& a_node)
{
    if(std::get_if<zero_t>(&a_node.m_data))
        return a_ostream << "low";

    if(std::get_if<one_t>(&a_node.m_data))
        return a_ostream << "high";

    if(const var_t* l_data =
           std::get_if<var_t>(&a_node.m_data))
        return a_ostream << l_data->m_index;

    if(std::get_if<invert_t>(&a_node.m_data))
        return a_ostream << "~{" << a_node.m_children[0]
                         << "}";

    if(std::get_if<disjoin_t>(&a_node.m_data))
        return a_ostream << "[" << a_node.m_children[0]
                         << "]|[" << a_node.m_children[1]
                         << "]";

    if(std::get_if<conjoin_t>(&a_node.m_data))
        return a_ostream << "(" << a_node.m_children[0]
                         << ")&(" << a_node.m_children[1]
                         << ")";

    if(const helper_t* l_data =
           std::get_if<helper_t>(&a_node.m_data))
    {
        a_ostream << "h" << l_data->m_index << "<";

        if(a_node.m_children.size() > 0)
            a_ostream << a_node.m_children[0];

        for(int i = 1; i < a_node.m_children.size(); ++i)
            a_ostream << "," << a_node.m_children[i];

        return a_ostream << ">";
    }

    throw std::runtime_error(
        "Error, invalid node data type.");
}

bool operator<(const terminate_t&, const terminate_t&)
{
    return false;
}
bool operator<(const make_function_t&,
               const make_function_t&)
{
    return false;
}

bool evaluate(const bool_node& a_expr,
              const std::vector<function>& a_helpers,
              const std::vector<bool>& a_x)
{
    if(std::get_if<zero_t>(&a_expr.m_data))
        return false;
    if(std::get_if<one_t>(&a_expr.m_data))
        return true;
    if(const var_t* l_var =
           std::get_if<var_t>(&a_expr.m_data))
        return a_x[l_var->m_index];
    if(std::get_if<invert_t>(&a_expr.m_data))
        return !evaluate(a_expr.m_children[0], a_helpers,
                         a_x);
    if(std::get_if<disjoin_t>(&a_expr.m_data))
        return evaluate(a_expr.m_children[0], a_helpers,
                        a_x) ||
               evaluate(a_expr.m_children[1], a_helpers,
                        a_x);
    if(std::get_if<conjoin_t>(&a_expr.m_data))
        return evaluate(a_expr.m_children[0], a_helpers,
                        a_x) &&
               evaluate(a_expr.m_children[1], a_helpers,
                        a_x);
    if(const helper_t* l_helper =
           std::get_if<helper_t>(&a_expr.m_data))
    {
        // get the helper function
        const function& l_helper_function =
            a_helpers[l_helper->m_index];

        // initialize the helper x to the input x (in-scope
        // vars are inherited)
        std::vector<bool> l_helper_x = a_x;

        // evaluate the arguments
        for(const auto& l_argument : a_expr.m_children)
            l_helper_x.push_back(
                evaluate(l_argument, a_helpers, a_x));

        return evaluate(l_helper_function.m_definition,
                        l_helper_function.m_helpers,
                        l_helper_x);
    }

    throw std::runtime_error(
        "Error: invalid bool_op_data type in evaluate().");
}

////////////////////////////////////////////////////
//////////////// FUNCTION GENERATION ///////////////
////////////////////////////////////////////////////

void build_function(
    size_t& a_arity, bool_node& a_definition,
    std::vector<function>& a_helpers,
    const size_t& a_in_scope_var_count,
    const bool& a_allow_non_nullary,
    monte_carlo::simulation<choice_t, std::mt19937>&
        a_simulation,
    const size_t& a_recursion_limit)
{
    ////////////////////////////////////////////////////
    ////////////// POPULATE CHOICE VECTOR //////////////
    ////////////////////////////////////////////////////
    std::vector<choice_t> l_choices;

    l_choices.push_back(zero_t{});
    l_choices.push_back(one_t{});

    // allow choosing any variable
    for(int i = 0; i < a_in_scope_var_count; ++i)
        l_choices.push_back(var_t{(size_t)i});

    // compute the new parameter index, if it is to be added
    // to the scope
    const size_t l_new_param_index =
        a_in_scope_var_count + a_arity;

    if(a_allow_non_nullary)
        l_choices.push_back(var_t{l_new_param_index});

    if(a_recursion_limit > 0)
    {
        // if the recursion limit has not been reached, push
        // non-nullary function types into list

        l_choices.push_back(invert_t{});
        l_choices.push_back(disjoin_t{});
        l_choices.push_back(conjoin_t{});

        // allow choosing any helper
        for(int i = 0; i < a_helpers.size(); ++i)
            l_choices.push_back(helper_t{(size_t)i});
    }

    ////////////////////////////////////////////////////
    ////////////// CHOOSE A NODE TO PLACE //////////////
    ////////////////////////////////////////////////////
    choice_t l_choice = a_simulation.choose(l_choices);

    // extract the op data, it should always be an op data
    // type.
    bool_op_data l_op_data =
        std::get<bool_op_data>(l_choice);

    ////////////////////////////////////////////////////
    //////////////// CONSTRUCT CHILDREN ////////////////
    ////////////////////////////////////////////////////
    size_t l_node_arity = 0;

    if(const var_t* l_var_data =
           std::get_if<var_t>(&l_op_data))
    {
        // allow overshooting by 1. In this case,
        // increment var count.
        if(l_var_data->m_index == l_new_param_index)
            ++a_arity;
    }
    else if(std::get_if<invert_t>(&l_op_data))
        l_node_arity = 1;
    else if(std::get_if<disjoin_t>(&l_op_data) ||
            std::get_if<conjoin_t>(&l_op_data))
        l_node_arity = 2;
    else if(const helper_t* l_helper_data =
                std::get_if<helper_t>(&l_op_data))
        l_node_arity =
            a_helpers[l_helper_data->m_index].m_arity;

    // create the children vector (pre-allocate)
    std::vector<bool_node> l_children(l_node_arity);

    // loop through, construct children in places
    for(int i = 0; i < l_node_arity; ++i)
        build_function(a_arity, l_children[i], a_helpers,
                       a_in_scope_var_count,
                       a_allow_non_nullary, a_simulation,
                       a_recursion_limit - 1);

    ////////////////////////////////////////////////////
    //////////////// CONSTRUCT THE NODE ////////////////
    ////////////////////////////////////////////////////
    a_definition = bool_node{l_op_data, l_children};
}

bool_node
build_model(const std::map<std::vector<bool>, bool> a_data,
            const size_t& a_in_scope_var_count,
            std::vector<function>& a_helpers,
            monte_carlo::simulation<choice_t, std::mt19937>&
                a_simulation,
            const size_t& a_recursion_limit)
{
    ////////////////////////////////////////////////////
    //////////////// CHECK FOR TRIVIALITY //////////////
    ////////////////////////////////////////////////////
    if(a_data.empty())
        throw std::runtime_error(
            "Error: no data points to build model from.");

    ////////////////////////////////////////////////////
    //////////////// CHECK FOR HOMOGENEITY /////////////
    ////////////////////////////////////////////////////

    // get the first label
    bool l_homogenous_value = a_data.begin()->second;

    // loop through the data points, check for homogeneity
    bool l_data_is_homogenous = std::all_of(
        a_data.begin(), a_data.end(),
        [l_homogenous_value](const auto& a_data_point)
        {
            return a_data_point.second ==
                   l_homogenous_value;
        });

    // if the data is homogenous, return the appropriate
    // constant
    if(l_data_is_homogenous)
        return l_homogenous_value ? one() : zero();

    ////////////////////////////////////////////////////
    //////////////// CREATE BINNING FUNCTION ///////////
    ////////////////////////////////////////////////////

    // construct the negative bin
    std::map<std::vector<bool>, bool> l_negative_bin;

    // construct the positive bin
    std::map<std::vector<bool>, bool> l_positive_bin;

    // declare the binning function
    bool_node l_binning_function;

    // configure the binning function to be nullary, but
    // to capture the in-scope variables
    bool l_bf_allow_non_nullary = false;
    size_t l_bf_arity = 0;

    // loop until neither output bin is empty
    // REASON: if one of the bins is empty, the binning
    // function is useless
    while(l_negative_bin.empty() || l_positive_bin.empty())
    {
        // clear BOTH bins in case one contains items
        l_negative_bin.clear();
        l_positive_bin.clear();

        // create a binning function that will bin (evaluate
        // on) each data point
        build_function(l_bf_arity, l_binning_function,
                       a_helpers, a_in_scope_var_count,
                       l_bf_allow_non_nullary, a_simulation,
                       a_recursion_limit);

        ////////////////////////////////////////////////////
        ////////////// EVALUATE BINNING FUNCTION ///////////
        ////////////////////////////////////////////////////

        // evaluate the binning function on all of the data
        // points
        for(const auto& [l_x, l_y] : a_data)
        {
            // evaluate the binning function
            bool l_binning_result = evaluate(
                l_binning_function, a_helpers, l_x);
            // store in the appropriate bin
            if(l_binning_result)
                l_positive_bin[l_x] = l_y;
            else
                l_negative_bin[l_x] = l_y;
        }
    }

    ////////////////////////////////////////////////////
    //////////////// COMMIT HELPER FUNCTION ////////////
    ////////////////////////////////////////////////////

    // create the helper function
    const function l_binning_function_helper{
        l_bf_arity, l_binning_function, a_helpers};

    // add the helper function to the helpers vector
    a_helpers.push_back(l_binning_function_helper);

    // construct an invocation of the helper function
    bool_node l_binning_function_invocation =
        helper(a_helpers.size() - 1, {});

    ////////////////////////////////////////////////////
    //////////////////////// RECUR /////////////////////
    ////////////////////////////////////////////////////

    // construct the left child
    bool_node l_left_child = build_model(
        l_negative_bin, a_in_scope_var_count, a_helpers,
        a_simulation, a_recursion_limit);

    // construct the right child
    bool_node l_right_child = build_model(
        l_positive_bin, a_in_scope_var_count, a_helpers,
        a_simulation, a_recursion_limit);

    // construct the final node
    return disjoin(
        conjoin(invert(l_binning_function_invocation),
                l_left_child),
        conjoin(l_binning_function_invocation,
                l_right_child));
}

// bool_node build_model(
//     const size_t&           a_arity,
//     std::vector<size_t>&    a_helper_arities,
//     std::vector<bool_node>& a_helpers,
//     monte_carlo::simulation<choice_t, std::mt19937>&
//     a_simulation, const size_t& a_recursion_limit)
// {
//     ////////////////////////////////////////////////////
//     //////////////// CONSTRUCT HELPERS /////////////////
//     ////////////////////////////////////////////////////
//     std::vector<choice_t> l_termination_choices
//     {terminate_t{}, make_function_t{}};

//     choice_t l_choice;

//     while(
//         (l_choice =
//         a_simulation.choose(l_termination_choices)),
//         std::get_if<make_function_t>(&l_choice))
//     {
//         size_t    l_helper_arity = 0;
//         bool_node l_helper =
//         build_function(l_helper_arity, a_helper_arities,
//         a_simulation, a_recursion_limit);

//         a_helper_arities.push_back(l_helper_arity);
//         a_helpers.push_back(l_helper);
//     }

// }

size_t node_count(const bool_node& a_expr)
{
    size_t l_result = 1;

    for(const auto& l_child : a_expr.m_children)
        l_result += node_count(l_child);

    return l_result;
}

void learn_model(
    bool_node& a_model, std::vector<function>& a_helpers,
    const size_t& a_in_scope_var_count,
    const std::map<std::vector<bool>, bool>& a_data,
    const size_t& a_iterations,
    const size_t& a_recursion_limit,
    const double& a_exploration_constant)
{
    std::mt19937 l_rnd_gen(27);
    monte_carlo::tree_node<choice_t> l_root;

    // initialize the best reward to the lowest possible
    // value
    double l_best_reward =
        -std::numeric_limits<double>::infinity();

    // save the original helpers
    std::vector<function> l_original_helpers = a_helpers;

    for(int i = 0; i < a_iterations; ++i)
    {
        // restore the original helpers
        std::vector<function> l_iteration_helpers =
            l_original_helpers;

        // construct the simulation
        monte_carlo::simulation<choice_t, std::mt19937>
            l_sim(l_root, a_exploration_constant,
                  l_rnd_gen);

        // construct the model
        bool_node l_model = build_model(
            a_data, a_in_scope_var_count,
            l_iteration_helpers, l_sim, a_recursion_limit);

        // compute the number of nodes in the helpers
        size_t l_helper_node_count = std::accumulate(
            l_iteration_helpers.begin(),
            l_iteration_helpers.end(), size_t{0},
            [](size_t a_acc, const function& a_helper)
            {
                return a_acc +
                       node_count(a_helper.m_definition);
            });

        // compute the number of nodes in the model
        size_t l_model_node_count = node_count(l_model);

        // compute the reward (negative number of nodes)
        double l_reward = -static_cast<double>(
            l_helper_node_count + l_model_node_count);

        // save best model
        if(l_reward > l_best_reward)
        {
            l_best_reward = l_reward;
            a_model = l_model;
            a_helpers = l_iteration_helpers;
        }

        std::cout << l_helper_node_count << " "
                  << l_model_node_count << " " << l_reward
                  << std::endl;

        std::cout << "helpers: " << std::endl;
        for(const auto& l_helper : l_iteration_helpers)
            std::cout << "    " << l_helper.m_definition
                      << std::endl;
        std::cout << std::endl;
        std::cout << "model: " << l_model << std::endl;

        // terminate the simulation
        l_sim.terminate(l_reward);
    }
}

////////////////////////////////////////////////////
////////////////////// TESTING /////////////////////
////////////////////////////////////////////////////
#ifdef UNIT_TEST

#include "test_utils.hpp"
#include <random>
#include <sstream>

void test_zero_construct_and_equality_check()
{
    assert(zero() == zero());
    bool_node l_node = zero();
    // check data field
    assert(std::get_if<zero_t>(&l_node.m_data));
}

void test_one_construct_and_equality_check()
{
    assert(one() == one());
    bool_node l_node = one();
    // check data field
    assert(std::get_if<one_t>(&l_node.m_data));
}

void test_var_construct_and_equality_check()
{
    assert(var(0) == var(0));
    assert(var(0) != var(1));
    bool_node l_node = var(1);
    // check data field
    var_t* l_data;
    assert(l_data = std::get_if<var_t>(&l_node.m_data));
    assert(l_data->m_index == 1);
}

void test_invert_construct_and_equality_check()
{
    assert(invert(var(0)) == invert(var(0)));
    assert(invert(var(0)) != invert(var(1)));
    bool_node l_node = invert(var(1));
    // check data field
    assert(std::get_if<invert_t>(&l_node.m_data));
}

void test_disjoin_construct_and_equality_check()
{
    assert(disjoin(var(0), var(1)) ==
           disjoin(var(0), var(1)));
    assert(disjoin(var(0), var(1)) !=
           disjoin(var(1), var(1)));
    assert(disjoin(var(0), var(1)) !=
           disjoin(var(0), var(2)));
    bool_node l_node = disjoin(var(0), var(1));
    // check data field
    assert(std::get_if<disjoin_t>(&l_node.m_data));
}

void test_conjoin_construct_and_equality_check()
{
    assert(conjoin(var(0), var(1)) ==
           conjoin(var(0), var(1)));
    assert(conjoin(var(0), var(1)) !=
           conjoin(var(1), var(1)));
    assert(conjoin(var(0), var(1)) !=
           conjoin(var(0), var(2)));
    bool_node l_node = conjoin(var(0), var(1));
    // check data field
    assert(std::get_if<conjoin_t>(&l_node.m_data));
}

void test_helper_construct_and_equality_check()
{
    assert(helper(0, {var(0), var(1)}) ==
           helper(0, {var(0), var(1)}));
    assert(helper(0, {var(0), var(1)}) !=
           helper(1, {var(0), var(1)}));
    assert(helper(0, {var(0), var(1)}) !=
           helper(0, {var(1), var(1)}));
    assert(helper(0, {var(0), var(1)}) !=
           helper(0, {var(0), var(0)}));
    assert(helper(0, {var(0), var(1)}) !=
           helper(0, {var(0), var(0), var(1)}));
    bool_node l_node = helper(2, {var(0), var(1)});
    // check data field
    helper_t* l_data;
    assert(l_data = std::get_if<helper_t>(&l_node.m_data));
    assert(l_data->m_index == 2);
}

void test_bool_node_ostream_inserter()
{
    {
        bool_node l_node = zero();
        std::stringstream l_ss;
        l_ss << l_node;
        assert(l_ss.str() == "low");
    }

    {
        bool_node l_node = one();
        std::stringstream l_ss;
        l_ss << l_node;
        assert(l_ss.str() == "high");
    }

    {
        bool_node l_node = var(3);
        std::stringstream l_ss;
        l_ss << l_node;
        assert(l_ss.str() == "3");
    }

    {
        bool_node l_node = invert(var(4));
        std::stringstream l_ss;
        l_ss << l_node;
        assert(l_ss.str() == "~{4}");
    }

    {
        bool_node l_node = disjoin(var(5), var(6));
        std::stringstream l_ss;
        l_ss << l_node;
        assert(l_ss.str() == "[5]|[6]");
    }

    {
        bool_node l_node = conjoin(var(5), var(6));
        std::stringstream l_ss;
        l_ss << l_node;
        assert(l_ss.str() == "(5)&(6)");
    }
}

// void test_build_function()
// {

//     struct test_data
//     {
//         uint32_t m_rnd_gen_seed;
//         std::vector<size_t> m_helper_arities;
//         size_t m_desired_func_arity;
//         bool_node m_desired_func;
//     };

//     std::list<test_data> l_examples{
//         test_data{
//             17,
//             {},
//             0,
//             one(),
//         },
//         test_data{
//             18,
//             {},
//             0,
//             disjoin(zero(),
//                     invert(invert(conjoin(one(),
//                     zero())))),
//         },
//         test_data{
//             20,
//             {},
//             4,
//             var(3),
//         },
//         test_data{
//             23,
//             {
//                 1,
//             },
//             0,
//             disjoin(conjoin(helper(0,
//             {one()}),conjoin(conjoin(one(),
//             disjoin(zero(), zero()))))),
//         },
//         test_data{
//             24,
//             {
//                 1,
//                 2,
//                 3,
//                 4,
//             },
//             4,
//             helper(
//                 3,
//                 {var(0), helper(0, {zero()}),
//                  helper(
//                      3,
//                      {
//                          var(1),
//                          var(0),
//                          helper(
//                              3,
//                              {
//                                  var(2),
//                                  invert(
//                                      helper(0,
//                                      {var(2)})),
//                                  helper(3,
//                                         {
//                                             invert(one()),
//                                             conjoin(zero(),
//                                                     one()),
//                                             var(2),
//                                             disjoin(one(),
//                                                     zero()),
//                                         }),
//                                  var(2),
//                              }),
//                          conjoin(
//                              helper(0, {var(3)}),
//                              helper(2,
//                                     {
//                                         helper(2,
//                                                {
//                                                    var(1),
//                                                    var(3),
//                                                    one(),
//                                                }),
//                                         var(1),
//                                         conjoin(var(1),
//                                                 var(1)),
//                                     })),
//                      }),
//                  invert(invert(
//                      helper(3,
//                             {
//                                 helper(2, {var(1),
//                                 var(3),
//                                            var(0)}),
//                                 var(2),
//                                 invert(var(2)),
//                                 invert(var(2)),
//                             })))}),
//         },
//     };

//     for(const test_data& l_example : l_examples)
//     {
//         std::mt19937 l_rnd_gen(l_example.m_rnd_gen_seed);
//         monte_carlo::tree_node<choice_t> l_root;
//         monte_carlo::simulation<choice_t, std::mt19937>
//             l_sim(l_root, 5, l_rnd_gen);
//         bool_node l_func = build_function(
//             l_example.m_desired_func_arity,
//             l_example.m_helper_arities, l_sim, 5);
//         std::cout << l_func << std::endl;
//         assert(l_func == l_example.m_desired_func);
//     }
// }

void test_build_model()
{
    std::mt19937 l_rnd_gen(5);
    monte_carlo::tree_node<choice_t> l_root;
    monte_carlo::simulation<choice_t, std::mt19937> l_sim(
        l_root, 5, l_rnd_gen);

    // while(true)
    // {

    //     std::vector<size_t>    l_helper_arities;
    //     std::vector<bool_node> l_helpers;

    //     bool_node l_model = build_model(10,
    //     l_helper_arities, l_helpers, l_sim, 5);

    //     for (const auto& l_fn : l_helpers)
    //         std::cout << l_fn << std::endl;

    //     std::cout << l_model << std::endl << std::endl;

    // }
}

void test_evaluate()
{
    struct test_data
    {
        std::vector<function> m_helpers;
        bool_node m_model;
        std::vector<bool> m_x;
        bool m_y;
    };

    std::list<test_data> l_examples{
        {
            {},
            zero(),
            {0, 0, 0},
            0,
        },
        {
            {},
            one(),
            {0, 0, 0},
            1,
        },
        {
            {},
            var(0),
            {0, 0, 0},
            0,
        },
        {
            {},
            var(0),
            {1, 0, 0},
            1,
        },
        {
            {},
            var(1),
            {1, 0, 0},
            0,
        },
        {
            {},
            var(1),
            {1, 1, 0},
            1,
        },
        {
            {},
            invert(zero()),
            {1, 0, 0},
            1,
        },
        {
            {},
            invert(one()),
            {1, 0, 0},
            0,
        },
        {
            {},
            disjoin(zero(), zero()),
            {1, 0, 0},
            0,
        },
        {
            {},
            disjoin(zero(), one()),
            {1, 0, 0},
            1,
        },
        {
            {},
            disjoin(one(), zero()),
            {1, 0, 0},
            1,
        },
        {
            {},
            disjoin(one(), one()),
            {1, 0, 0},
            1,
        },
        {
            {},
            conjoin(zero(), zero()),
            {1, 0, 0},
            0,
        },
        {
            {},
            conjoin(zero(), one()),
            {1, 0, 0},
            0,
        },
        {
            {},
            conjoin(one(), zero()),
            {1, 0, 0},
            0,
        },
        {
            {},
            conjoin(one(), one()),
            {1, 0, 0},
            1,
        },
        {
            // demonstrate helper captures
            {
                function{0, var(2), {}},
            },
            helper(0, {}),
            {1, 0, 0},
            0,
        },
        {
            // demonstrate helper captures
            {
                function{0, var(2), {}},
            },
            helper(0, {}),
            {1, 0, 1},
            1,
        },
        {
            // demonstrate helper parameters
            {
                function{1, var(3), {}},
            },
            helper(1, {var(1)}),
            {0, 1, 0},
            1,
        },
        {
            // demonstrate helper uses captured val over
            // param
            {
                function{1, var(0), {}},
                function{3, var(5), {}},
            },
            helper(0, {var(2)}),
            {0, 1, 1},
            0,
        },
    };

    for(const auto& l_example : l_examples)
    {
        assert(evaluate(l_example.m_model,
                        l_example.m_helpers,
                        l_example.m_x) == l_example.m_y);
    }
}

void test_learn_model()
{
    constexpr size_t IN_SCOPE_VAR_COUNT = 4;
    constexpr size_t ITERATIONS = 1000000;

    std::vector<function> l_helpers;

    std::map<std::vector<bool>, bool> l_data{
        {{0, 0, 0, 1}, 0},
        {{0, 1, 0, 1}, 1},
        {{1, 0, 0, 1}, 1},
        {{1, 1, 0, 1}, 1},
        {{1, 1, 0, 0}, 0}};

    bool_node l_model;

    learn_model(l_model, l_helpers, IN_SCOPE_VAR_COUNT,
                l_data, ITERATIONS, 3, 10);

    // for(const auto& l_helper : l_helpers)
    //     std::cout << l_helper << std::endl;

    std::cout << l_model << std::endl;
}

void bool_reduce_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_zero_construct_and_equality_check);
    TEST(test_one_construct_and_equality_check);
    TEST(test_var_construct_and_equality_check);
    TEST(test_invert_construct_and_equality_check);
    TEST(test_disjoin_construct_and_equality_check);
    TEST(test_conjoin_construct_and_equality_check);
    TEST(test_helper_construct_and_equality_check);
    TEST(test_bool_node_ostream_inserter);
    // TEST(test_build_function);
    TEST(test_build_model);
    TEST(test_evaluate);
    TEST(test_learn_model);
}

#endif
