#include "../include/model.hpp"
#include <cassert>

model::model(bool a_homogenous_value)
    : m_homogenous_value(a_homogenous_value), m_func(nullptr),
      m_negative_child(nullptr), m_positive_child(nullptr)
{
}
model::model(std::unique_ptr<lambda::expr>&& a_func,
             std::unique_ptr<model>&& a_negative_child,
             std::unique_ptr<model>&& a_positive_child)
    : m_homogenous_value(false), m_func(std::move(a_func)),
      m_negative_child(std::move(a_negative_child)),
      m_positive_child(std::move(a_positive_child))
{
}

// builds an application tower of the binning function and params
lambda::expr::normalize_result
call_function(const std::unique_ptr<lambda::expr>& a_function,
              const std::unique_ptr<lambda::expr>* a_params,
              size_t a_param_count, size_t a_step_limit, size_t a_size_limit)
{
    // construct the application of the binning function to the parameters
    auto l_app = a_function->clone();
    for(size_t i = 0; i < a_param_count; ++i)
        l_app = a(std::move(l_app), a_params[i]->clone());

    // normalize the application
    return l_app->normalize(a_step_limit, a_size_limit);
}

bool boolify(const std::unique_ptr<lambda::expr>& a_expr)
{
    using namespace lambda;

    // TODO: implement this
    // returns the inverse parity of the first variable in the expression

    // start at the root of the expression
    const std::unique_ptr<lambda::expr>* l_expr = &a_expr;

    // traverse until we find a variable
    while(true)
    {
        if(const var* l_var = dynamic_cast<const var*>(l_expr->get()))
            return l_var->index() % 2 == 0;
        else if(const app* l_app = dynamic_cast<const app*>(l_expr->get()))
            l_expr = &l_app->lhs();
        else if(const func* l_func = dynamic_cast<const func*>(l_expr->get()))
            l_expr = &l_func->body();
        else
            throw std::runtime_error("Error: invalid expression in boolify.");
    }
}

std::optional<bool> model::eval(const std::unique_ptr<lambda::expr>* a_params,
                                size_t a_param_count, size_t a_step_limit,
                                size_t a_size_limit)
{
    using namespace lambda;

    // if the model is homogenous, then return the homogenous value
    if(m_func == nullptr)
        return m_homogenous_value;

    // evaluate the binning function
    auto l_normalize_result = call_function(m_func, a_params, a_param_count,
                                            a_step_limit, a_size_limit);

    // if the evaluation is too complex, return std::nullopt
    if(l_normalize_result.m_step_excess || l_normalize_result.m_size_excess)
        return std::nullopt;

    // boolify the result
    bool l_binning_result = boolify(l_normalize_result.m_expr);

    // get the appropriate child
    const auto& l_child =
        l_binning_result ? m_positive_child : m_negative_child;

    // evaluate the child
    return l_child->eval(a_params, a_param_count, a_step_limit, a_size_limit);
}

std::ostream& operator<<(std::ostream& a_ostream, const model& a_model)
{
    if(a_model.m_func == nullptr)
        return a_ostream << (a_model.m_homogenous_value ? "true" : "false");

    return a_ostream << "[" << *a_model.m_func << "] ? {"
                     << *a_model.m_positive_child << "} : {"
                     << *a_model.m_negative_child << "}";
}

////////////////////////////////////////////////////
//////////////// COMPARISON OPERATORS //////////////
////////////////////////////////////////////////////

bool operator<(const place_var_node& a_lhs, const place_var_node& a_rhs)
{
    return a_lhs.m_index < a_rhs.m_index;
}
bool operator<(const place_func_node&, const place_func_node&)
{
    return false;
}
bool operator<(const place_app_node&, const place_app_node&)
{
    return false;
}
bool operator<(const add_helper&, const add_helper&)
{
    return false;
}
bool operator<(const terminate&, const terminate&)
{
    return false;
}

////////////////////////////////////////////////////
//////////////// FUNCTION GENERATION ///////////////
////////////////////////////////////////////////////

std::unique_ptr<lambda::expr>
build_function_body(const size_t a_binder_depth,
                    monte_carlo::simulation<choice, std::mt19937>& a_simulation,
                    const size_t& a_recursion_limit)
{
    using namespace lambda;

    // declare the choice list
    std::vector<choice> l_choices;

    // add var choices
    for(size_t i = 0; i < a_binder_depth; ++i)
        l_choices.push_back(place_var_node{i});

    if(a_recursion_limit > 0)
    {
        // add func choices
        l_choices.push_back(place_func_node{});

        // add app choices
        l_choices.push_back(place_app_node{});
    }

    // make choice
    choice l_choice = a_simulation.choose(l_choices);

    // handle the choice
    if(const auto* l_var_node = std::get_if<place_var_node>(&l_choice))
    {
        return v(l_var_node->m_index);
    }
    else if(std::holds_alternative<place_func_node>(l_choice))
    {
        // build the function
        return f(build_function_body(a_binder_depth + 1, a_simulation,
                                     a_recursion_limit - 1));
    }
    else if(std::holds_alternative<place_app_node>(l_choice))
    {
        // MUST be an application here

        // build the application
        return a(build_function_body(a_binder_depth, a_simulation,
                                     a_recursion_limit - 1),
                 build_function_body(a_binder_depth, a_simulation,
                                     a_recursion_limit - 1));
    }

    throw std::runtime_error("Error: invalid choice in build_function_body.");
}

std::unique_ptr<lambda::expr>
build_function(const size_t a_binder_depth, const size_t a_arity,
               monte_carlo::simulation<choice, std::mt19937>& a_simulation,
               const size_t& a_recursion_limit)
{
    using namespace lambda;

    // build the function body
    std::unique_ptr<lambda::expr> l_function = build_function_body(
        a_binder_depth + a_arity, a_simulation, a_recursion_limit);

    // add the lambda abstractions
    for(size_t i = 0; i < a_arity; ++i)
        l_function = f(std::move(l_function));

    // return the final expression
    return l_function;
}

std::unique_ptr<model>
build_model(const std::vector<const data_point*>& a_data,
            const std::list<std::unique_ptr<lambda::expr>>& a_helpers,
            const size_t& a_step_limit, const size_t& a_size_limit,
            const size_t& a_arity,
            monte_carlo::simulation<choice, std::mt19937>& a_simulation,
            const size_t& a_recursion_limit)
{
    using namespace lambda;

    ////////////////////////////////////////////////////
    //////////////// CHECK FOR TRIVIALITY //////////////
    ////////////////////////////////////////////////////
    if(a_data.empty())
        throw std::runtime_error("Error: no data points to build model from.");

    ////////////////////////////////////////////////////
    //////////////// CHECK FOR HOMOGENEITY /////////////
    ////////////////////////////////////////////////////

    // get the first label
    bool l_homogenous_value = (*a_data.begin())->m_output;

    // loop through the data points, check for homogeneity
    bool l_data_is_homogenous =
        std::all_of(a_data.begin(), a_data.end(),
                    [l_homogenous_value](const data_point* a_data_point)
                    { return a_data_point->m_output == l_homogenous_value; });

    // if the data is homogenous, return the appropriate
    // constant
    if(l_data_is_homogenous)
        return std::make_unique<model>(l_homogenous_value);

    ////////////////////////////////////////////////////
    /////////////// CREATE BINNING FUNCTION ////////////
    ////////////////////////////////////////////////////

    // construct the negative bin
    std::vector<const data_point*> l_negative_bin;

    // construct the positive bin
    std::vector<const data_point*> l_positive_bin;

    // declare the binning function body
    std::unique_ptr<lambda::expr> l_binning_function;

    // track if the normalization is complex (too many steps or size)
    bool l_normalization_complex = true;

    // loop until we have a contingent, terminating binning function
    while(l_normalization_complex || l_negative_bin.empty() ||
          l_positive_bin.empty())
    {
        // clear BOTH bins in case one contains items
        l_negative_bin.clear();
        l_positive_bin.clear();
        l_normalization_complex = true;

        // construct the binning function
        // [create a binning function that will bin (evaluate
        // on) each data point]
        l_binning_function = build_function(a_helpers.size(), a_arity,
                                            a_simulation, a_recursion_limit);

        // replace binning function with program given binning function as main
        l_binning_function = construct_program(
            a_helpers.begin(), a_helpers.end(), l_binning_function);

        ////////////////////////////////////////////////////
        ////////////// EVALUATE BINNING FUNCTION ///////////
        ////////////////////////////////////////////////////

        // evaluate the binning function on all of the
        // data points
        for(const auto* l_data_point : a_data)
        {
            // evaluate the binning function
            auto l_normalize_result = call_function(
                l_binning_function, l_data_point->m_inputs.data(),
                l_data_point->m_inputs.size(), a_step_limit, a_size_limit);

            // if the evaluation is too complex, set normalization complex flag
            // then break to prevent evaluating any more data points.
            if(l_normalize_result.m_step_excess ||
               l_normalize_result.m_size_excess)
            {
                l_normalization_complex = true;
                break;
            }

            // boolify the result
            bool l_binning_result = boolify(l_normalize_result.m_expr);

            // store in the appropriate bin
            if(l_binning_result)
                l_positive_bin.emplace_back(l_data_point);
            else
                l_negative_bin.emplace_back(l_data_point);
        }
    }

    ////////////////////////////////////////////////////
    //////////////////////// RECUR /////////////////////
    ////////////////////////////////////////////////////

    // construct the negative child
    std::unique_ptr<model> l_negative_child =
        build_model(l_negative_bin, a_helpers, a_step_limit, a_size_limit,
                    a_arity, a_simulation, a_recursion_limit);

    // construct the positive child
    std::unique_ptr<model> l_positive_child =
        build_model(l_positive_bin, a_helpers, a_step_limit, a_size_limit,
                    a_arity, a_simulation, a_recursion_limit);

    // construct the final node
    return std::make_unique<model>(std::move(l_binning_function),
                                   std::move(l_negative_child),
                                   std::move(l_positive_child));
}

// template <typename... Params>
// model learn_model(
//     program& a_program, scope& a_scope,
//     const std::vector<std::pair<std::vector<std::any>, bool>>& a_data,
//     const size_t& a_iterations, const size_t& a_recursion_limit,
//     const double& a_exploration_constant)
// {
//     std::mt19937 l_rnd_gen(27);
//     monte_carlo::tree_node<choice> l_root;

//     // get the parameter types
//     std::vector<std::type_index> l_param_types_list = {typeid(Params)...};

//     // convert the parameter types to a multimap
//     std::multimap<std::type_index, size_t> l_param_types;
//     for(size_t i = 0; i < l_param_types_list.size(); ++i)
//         l_param_types.emplace(l_param_types_list[i], i);

//     // initialize the best reward to the lowest possible
//     // value
//     double l_best_reward = -std::numeric_limits<double>::infinity();
//     model l_best_model;

//     // save the original program and scope
//     program l_original_program = a_program;
//     scope l_original_scope = a_scope;

//     for(int i = 0; i < a_iterations; ++i)
//     {
//         // construct the simulation
//         monte_carlo::simulation<choice, std::mt19937> l_sim(
//             l_root, a_exploration_constant, l_rnd_gen);

//         // restore the original program and scope
//         program l_program = l_original_program;
//         scope l_scope = l_original_scope;

//         // construct the model
//         model l_model = build_model(l_program, l_scope, l_param_types,
//         a_data,
//                                     l_sim, a_recursion_limit);

//         // compute the number of nodes in the whole program
//         size_t l_program_node_count =
//             std::accumulate(l_program.m_funcs.begin(),
//             l_program.m_funcs.end(),
//                             size_t{0}, [](size_t a_acc, const auto& a_func)
//                             { return a_acc + a_func->m_body.node_count(); });

//         // compute the number of nodes in the model
//         size_t l_model_node_count = l_model.node_count();

//         // compute the reward (negative number of nodes)
//         double l_reward =
//             -static_cast<double>(l_program_node_count + l_model_node_count);

//         // save best model
//         if(l_reward > l_best_reward)
//         {
//             l_best_reward = l_reward;
//             a_program = l_program;
//             a_scope = l_scope;
//             l_best_model = l_model;

//             std::cout << l_program_node_count << " " << l_reward <<
//             std::endl;

//             std::cout << "program: " << std::endl;
//             for(const auto& l_func : l_program.m_funcs)
//                 std::cout << "    " << l_func->m_repr << std::endl;

//             std::cout << "model: " << l_model.repr() << std::endl;
//             std::cout << std::endl;
//         }

//         // terminate the simulation
//         l_sim.terminate(l_reward);
//     }

//     return l_best_model;
// }

#ifdef UNIT_TEST
#include "test_utils.hpp"

// void test_model_eval()
// {
//     // immediately homogenous (falsy) model
//     {
//         model l_model(false);

//         // evaluate the model
//         bool l_result = l_model.eval(nullptr, 0, 1000, 1000).value();

//         // check the result
//         assert(l_result == false);
//     }

//     // immediately homogenous (truthy) model
//     {
//         model l_model{.m_homogenous_value = true};

//         // evaluate the model
//         bool l_result = l_model.eval(nullptr, 0);

//         // check the result
//         assert(l_result == true);
//     }

//     // model with 1 binning function
//     {
//         // construct program
//         program l_program;

//         // add a primitive binning function
//         auto l_func_0 = l_program.add_primitive(
//             "bin0", std::function([](int a_x) { return a_x > 0; }));

//         // construct model
//         model l_model{.m_func = l_func_0};

//         // construct left child
//         l_model.m_negative_child =
//             std::make_unique<model>(model{.m_homogenous_value = false});

//         // construct right child
//         l_model.m_positive_child =
//             std::make_unique<model>(model{.m_homogenous_value = true});

//         // construct input
//         std::vector<std::any> l_input;

//         // test truthy input
//         l_input = {10};
//         assert(l_model.eval(l_input.data(), l_input.size()) == true);

//         // test falsy inputs
//         l_input = {-10};
//         assert(l_model.eval(l_input.data(), l_input.size()) == false);
//     }

//     // model with 1 binning function and a left child
//     {
//         // construct program
//         program l_program;

//         // add a primitive binning function
//         auto l_func_0 = l_program.add_primitive(
//             "bin0", std::function([](int a_x) { return a_x > 0; })); //
//             positive

//         // add another primitive binning function
//         auto l_func_1 = l_program.add_primitive(
//             "bin1",
//             std::function([](int a_x) { return a_x % 2 == 0; })); // even

//         // construct model
//         model l_model{.m_func = l_func_0};

//         // construct left child
//         l_model.m_negative_child =
//             std::make_unique<model>(model{.m_func = l_func_1});

//         // construct left-left child and left-right child
//         l_model.m_negative_child->m_negative_child =
//             std::make_unique<model>(model{.m_homogenous_value = false});
//         l_model.m_negative_child->m_positive_child =
//             std::make_unique<model>(model{.m_homogenous_value = true});

//         // construct right child
//         l_model.m_positive_child =
//             std::make_unique<model>(model{.m_homogenous_value = true});

//         // construct input
//         std::vector<std::any> l_input;

//         // test input 10 (positive and even)
//         l_input = {10};
//         assert(l_model.eval(l_input.data(), l_input.size()) == true);

//         // test input 7 (positive and odd)
//         l_input = {7};
//         assert(l_model.eval(l_input.data(), l_input.size()) == true);

//         // test input -10 (negative and even)
//         l_input = {-10};
//         assert(l_model.eval(l_input.data(), l_input.size()) == true);

//         // test input -7 (negative and odd)
//         l_input = {-7};
//         assert(l_model.eval(l_input.data(), l_input.size()) == false);
//     }

//     // model with 1 binning function and a left, and right child
//     {
//         // construct program
//         program l_program;

//         // add a primitive binning function
//         auto l_func_0 = l_program.add_primitive(
//             "bin0", std::function([](int a_x) { return a_x > 0; })); //
//             positive

//         // add another primitive binning function
//         auto l_func_1 = l_program.add_primitive(
//             "bin1",
//             std::function([](int a_x) { return a_x % 2 == 0; })); // even

//         // add another primitive binning function
//         auto l_func_2 = l_program.add_primitive(
//             "bin2", std::function([](int a_x)
//                                   { return a_x % 3 == 0; })); // divisible by
//                                   3

//         // construct model
//         model l_model{.m_func = l_func_0};

//         // construct left child
//         l_model.m_negative_child =
//             std::make_unique<model>(model{.m_func = l_func_1});

//         // construct left-left child and left-right child
//         l_model.m_negative_child->m_negative_child =
//             std::make_unique<model>(model{.m_homogenous_value = false});
//         l_model.m_negative_child->m_positive_child =
//             std::make_unique<model>(model{.m_homogenous_value = true});

//         // construct right child
//         l_model.m_positive_child =
//             std::make_unique<model>(model{.m_func = l_func_2});

//         // construct right-left child and right-right child
//         l_model.m_positive_child->m_negative_child =
//             std::make_unique<model>(model{.m_homogenous_value = false});
//         l_model.m_positive_child->m_positive_child =
//             std::make_unique<model>(model{.m_homogenous_value = true});

//         // construct input
//         std::vector<std::any> l_input;

//         // positive, even, divisible by 3
//         l_input = {6};
//         assert(l_model.eval(l_input.data(), l_input.size()) == true);

//         // positive, even, not divisible by 3
//         l_input = {4};
//         assert(l_model.eval(l_input.data(), l_input.size()) == false);

//         // positive, odd, divisible by 3
//         l_input = {9};
//         assert(l_model.eval(l_input.data(), l_input.size()) == true);

//         // positive, odd, not divisible by 3
//         l_input = {7};
//         assert(l_model.eval(l_input.data(), l_input.size()) == false);

//         // negative, even, divisible by 3
//         l_input = {-6};
//         assert(l_model.eval(l_input.data(), l_input.size()) == true);

//         // negative, even, not divisible by 3
//         l_input = {-4};
//         assert(l_model.eval(l_input.data(), l_input.size()) == true);

//         // negative, odd, divisible by 3
//         l_input = {-9};
//         assert(l_model.eval(l_input.data(), l_input.size()) == false);

//         // negative, odd, not divisible by 3
//         l_input = {-7};
//         assert(l_model.eval(l_input.data(), l_input.size()) == false);
//     }
// }

void model_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    // TEST(test_model_eval);
}

#endif // UNIT_TEST
