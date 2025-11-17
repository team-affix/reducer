#include "../include/model.hpp"
#include <cassert>

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

std::optional<bool>
eval_binning_program(const std::unique_ptr<lambda::expr>& a_binning_program,
                     const std::unique_ptr<lambda::expr>* a_params,
                     size_t a_param_count, size_t a_step_limit,
                     size_t a_size_limit)
{
    using namespace lambda;

    auto l_norm_operand = a_binning_program->clone();

    // build the application tower
    for(size_t i = 0; i < a_param_count; ++i)
        l_norm_operand = a(std::move(l_norm_operand), a_params[i]->clone());

    // normalize the application
    auto l_normalize_result =
        l_norm_operand->normalize(a_step_limit, a_size_limit);

    // if the evaluation is too complex, return std::nullopt
    if(l_normalize_result.m_step_excess || l_normalize_result.m_size_excess)
        return std::nullopt;

    // boolify the result
    bool l_binning_result = boolify(l_normalize_result.m_expr);

    // return the result
    return l_binning_result;
}

model::model(bool a_homogenous_value)
    : m_homogenous_value(a_homogenous_value), m_func(nullptr),
      m_negative_child(nullptr), m_positive_child(nullptr)
{
}

model::model(std::unique_ptr<lambda::expr>&& a_func,
             std::unique_ptr<model>&& a_positive_child,
             std::unique_ptr<model>&& a_negative_child)
    : m_homogenous_value(false), m_func(std::move(a_func)),
      m_positive_child(std::move(a_positive_child)),
      m_negative_child(std::move(a_negative_child))
{
}

std::optional<bool>
model::eval(const std::list<std::unique_ptr<lambda::expr>>& a_helpers,
            const std::unique_ptr<lambda::expr>* a_params, size_t a_param_count,
            size_t a_step_limit, size_t a_size_limit)
{
    using namespace lambda;

    // if the model is homogenous, then return the homogenous value
    if(m_func == nullptr)
        return m_homogenous_value;

    // construct the binning program
    auto l_binning_program =
        construct_program(a_helpers.begin(), a_helpers.end(), m_func->clone());

    // evaluate the binning program
    auto l_binning_result = eval_binning_program(
        l_binning_program, a_params, a_param_count, a_step_limit, a_size_limit);

    // if the binning function evaluation is too complex, return std::nullopt
    if(!l_binning_result.has_value())
        return std::nullopt;

    // get the appropriate child
    const auto& l_child =
        (*l_binning_result) ? m_positive_child : m_negative_child;

    // evaluate the child
    return l_child->eval(a_helpers, a_params, a_param_count, a_step_limit,
                         a_size_limit);
}

////////////////////////////////////////////////////
//////////////// FACTORY FUNCTIONS /////////////////
////////////////////////////////////////////////////

std::unique_ptr<model> m(bool a_value)
{
    return std::unique_ptr<model>(new model(a_value));
}

std::unique_ptr<model> m(std::unique_ptr<lambda::expr>&& a_func,
                         std::unique_ptr<model>&& a_positive_child,
                         std::unique_ptr<model>&& a_negative_child)
{
    return std::unique_ptr<model>(new model(std::move(a_func),
                                            std::move(a_positive_child),
                                            std::move(a_negative_child)));
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
        return m(l_homogenous_value);

    ////////////////////////////////////////////////////
    /////////////// CREATE BINNING FUNCTION ////////////
    ////////////////////////////////////////////////////

    // construct the positive bin
    std::vector<const data_point*> l_positive_bin;

    // construct the negative bin
    std::vector<const data_point*> l_negative_bin;

    // declare the binning function body
    std::unique_ptr<lambda::expr> l_binning_function;

    // track if the normalization is complex (too many steps or size)
    bool l_normalization_complex = false;

    // loop until we have a contingent, terminating binning function
    while(l_positive_bin.empty() || l_negative_bin.empty() ||
          l_normalization_complex)
    {
        // clear BOTH bins in case one contains items
        l_positive_bin.clear();
        l_negative_bin.clear();
        l_normalization_complex = false;

        // construct the binning function
        // [create a binning function that will bin (evaluate
        // on) each data point]
        l_binning_function = build_function(a_helpers.size(), a_arity,
                                            a_simulation, a_recursion_limit);

        // construct the binning program
        auto l_binning_program = construct_program(
            a_helpers.begin(), a_helpers.end(), l_binning_function->clone());

        ////////////////////////////////////////////////////
        ////////////// EVALUATE BINNING FUNCTION ///////////
        ////////////////////////////////////////////////////

        // evaluate the binning function on all of the
        // data points
        for(const auto* l_data_point : a_data)
        {
            // evaluate the binning function
            auto l_normalize_result = eval_binning_program(
                l_binning_program, l_data_point->m_inputs.data(),
                l_data_point->m_inputs.size(), a_step_limit, a_size_limit);

            // if the evaluation is too complex, set normalization complex flag
            // then break to prevent evaluating any more data points.
            if(!l_normalize_result.has_value())
            {
                l_normalization_complex = true;
                break;
            }

            // store in the appropriate bin
            if(*l_normalize_result)
                l_positive_bin.emplace_back(l_data_point);
            else
                l_negative_bin.emplace_back(l_data_point);
        }
    }

    ////////////////////////////////////////////////////
    //////////////////////// RECUR /////////////////////
    ////////////////////////////////////////////////////

    // construct the positive child
    std::unique_ptr<model> l_positive_child =
        build_model(l_positive_bin, a_helpers, a_step_limit, a_size_limit,
                    a_arity, a_simulation, a_recursion_limit);

    // construct the negative child
    std::unique_ptr<model> l_negative_child =
        build_model(l_negative_bin, a_helpers, a_step_limit, a_size_limit,
                    a_arity, a_simulation, a_recursion_limit);

    // construct the final node
    return m(std::move(l_binning_function), std::move(l_positive_child),
             std::move(l_negative_child));
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
#include <sstream>

void test_boolify()
{
    using namespace lambda;
    assert(boolify(v(0)) == true);
    assert(boolify(v(1)) == false);
    assert(boolify(f(v(0))) == true);
    assert(boolify(f(v(1))) == false);
    assert(boolify(a(v(0), v(1))) == true);
    assert(boolify(a(f(v(0)), v(1))) == true);
    // var 2
    assert(boolify(v(2)) == true);
    assert(boolify(v(3)) == false);
    assert(boolify(f(v(2))) == true);
    assert(boolify(f(v(3))) == false);
    assert(boolify(a(v(2), v(3))) == true);
    assert(boolify(a(f(v(2)), v(3))) == true);
    assert(boolify(a(v(2), f(v(3)))) == true);
    assert(boolify(a(f(v(2)), f(v(3)))) == true);
}

void test_eval_binning_program()
{
    using namespace lambda;

    // var 0, step limit 1000, size limit 1000
    {
        std::unique_ptr<lambda::expr> l_binning_program = v(0);
        assert(eval_binning_program(l_binning_program, nullptr, 0, 1000, 1000)
                   .value() == true);
    }

    // var 1, step limit 1000, size limit 1000
    {
        std::unique_ptr<lambda::expr> l_binning_program = v(1);
        assert(eval_binning_program(l_binning_program, nullptr, 0, 1000, 1000)
                   .value() == false);
    }

    // func 0, step limit 1000, size limit 1000
    {
        std::unique_ptr<lambda::expr> l_binning_program = f(v(0));
        assert(eval_binning_program(l_binning_program, nullptr, 0, 1000, 1000)
                   .value() == true);
    }

    // func 1, step limit 1000, size limit 1000
    {
        std::unique_ptr<lambda::expr> l_binning_program = f(v(1));
        assert(eval_binning_program(l_binning_program, nullptr, 0, 1000, 1000)
                   .value() == false);
    }

    // omega (omega combinator), step limit 100, size limit 100, should be too
    // complex to evaluate
    {
        std::unique_ptr<lambda::expr> l_binning_program =
            a(f(a(v(0), v(0))), f(a(v(0), v(0))));
        assert(eval_binning_program(l_binning_program, nullptr, 0, 100, 100) ==
               std::nullopt);
    }

    // church bools, (make sure true is truthy and false is falsy)
    // step limit 100, size limit 100, should be easy to evaluate
    {
        std::list<std::unique_ptr<lambda::expr>> l_helpers;
        // l() and g() pattern
        auto l = [&l_helpers](size_t a_index)
        { return v(a_index + l_helpers.size()); };
        auto g = [](size_t a_index) { return v(a_index); };

        // construct the helpers
        const auto TRUE = g(l_helpers.size());
        l_helpers.push_back(f(f(l(0))));
        const auto FALSE = g(l_helpers.size());
        l_helpers.push_back(f(f(l(1))));

        // construct the binning program
        std::unique_ptr<lambda::expr> l_true_binning_program =
            construct_program(l_helpers.begin(), l_helpers.end(),
                              TRUE->clone());

        // construct the binning program
        std::unique_ptr<lambda::expr> l_false_binning_program =
            construct_program(l_helpers.begin(), l_helpers.end(),
                              FALSE->clone());

        // evaluate the binning program
        assert(
            eval_binning_program(l_true_binning_program, nullptr, 0, 100, 100)
                .value() == true);

        // evaluate the binning program
        assert(
            eval_binning_program(l_false_binning_program, nullptr, 0, 100, 100)
                .value() == false);
    }

    // succ of zero, step limit 100, size limit 100, should be easy to eval
    {
        std::list<std::unique_ptr<lambda::expr>> l_helpers;

        // l() and g() pattern
        auto l = [&l_helpers](size_t a_index)
        { return v(a_index + l_helpers.size()); };
        auto g = [](size_t a_index) { return v(a_index); };

        // construct the helpers
        // 0
        const auto ZERO = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(1))));

        // succ
        const auto SUCC = g(l_helpers.size());
        l_helpers.emplace_back(f(f(f(a(l(1), a(a(l(0), l(1)), l(2)))))));

        // construct the binning program (zero)
        std::unique_ptr<lambda::expr> l_zero_binning_program =
            construct_program(l_helpers.begin(), l_helpers.end(),
                              ZERO->clone());

        // construct the binning program (succ of zero)
        std::unique_ptr<lambda::expr> l_one_binning_program =
            construct_program(l_helpers.begin(), l_helpers.end(),
                              a(SUCC->clone(), ZERO->clone()));

        // evaluate the binning program
        assert(
            eval_binning_program(l_zero_binning_program, nullptr, 0, 100, 100)
                .value() == false);

        // evaluate the binning program
        assert(eval_binning_program(l_one_binning_program, nullptr, 0, 100, 100)
                   .value() == true);
    }
}

void test_model_construct_and_print()
{
    using namespace lambda;

    // falsy model
    {
        std::stringstream l_ss;
        // falsy model
        std::unique_ptr<model> l_model = m(false);
        l_ss << *l_model;
        assert(l_ss.str() == "false");
    }

    // truthy model
    {
        std::stringstream l_ss;
        // truthy model
        std::unique_ptr<model> l_model = m(true);
        l_ss << *l_model;
        assert(l_ss.str() == "true");
    }

    // var binning function with homogenous children
    {
        std::stringstream l_ss;
        std::stringstream l_expected_ss;
        std::unique_ptr<lambda::expr> l_binning_function = v(0);
        // non-homogenous model with homogenous children
        std::unique_ptr<model> l_model =
            m(l_binning_function->clone(), m(true), m(false));
        l_ss << *l_model;
        std::cout << l_ss.str() << std::endl;
        l_expected_ss << "[" << *l_binning_function << "] ? {true} : {false}";
        assert(l_ss.str() == l_expected_ss.str());
    }

    // func binning function with homogenous children
    {
        std::stringstream l_ss;
        std::stringstream l_expected_ss;
        std::unique_ptr<lambda::expr> l_binning_function = f(v(0));
        // non-homogenous model with homogenous children
        std::unique_ptr<model> l_model =
            m(l_binning_function->clone(), m(true), m(false));
        l_ss << *l_model;
        std::cout << l_ss.str() << std::endl;
        l_expected_ss << "[" << *l_binning_function << "] ? {true} : {false}";
        assert(l_ss.str() == l_expected_ss.str());
    }

    // app binning function with homogenous children
    {
        std::stringstream l_ss;
        std::stringstream l_expected_ss;
        std::unique_ptr<lambda::expr> l_binning_function = a(v(0), v(1));
        std::unique_ptr<model> l_model =
            m(l_binning_function->clone(), m(true), m(false));
        l_ss << *l_model;
        std::cout << l_ss.str() << std::endl;
        l_expected_ss << "[" << *l_binning_function << "] ? {true} : {false}";
        assert(l_ss.str() == l_expected_ss.str());
    }

    // non-homogenous model with non-homogenous children
    {
        std::stringstream l_ss;
        std::stringstream l_expected_ss;
        std::unique_ptr<lambda::expr> l_binning_function = f(v(0));
        std::unique_ptr<lambda::expr> l_positive_child_function = v(1);
        std::unique_ptr<lambda::expr> l_negative_child_function = v(2);
        std::unique_ptr<model> l_model =
            m(l_binning_function->clone(),
              m(l_positive_child_function->clone(), m(true), m(false)),
              m(l_negative_child_function->clone(), m(false), m(true)));
        l_ss << *l_model;
        std::cout << l_ss.str() << std::endl;
        l_expected_ss << "[" << *l_binning_function << "] ? {["
                      << *l_positive_child_function
                      << "] ? {true} : {false}} : {["
                      << *l_negative_child_function << "] ? {false} : {true}}";
        assert(l_ss.str() == l_expected_ss.str());
    }
}

void test_model_eval()
{
    // immediately homogenous (falsy) model
    {
        std::unique_ptr<model> l_model = m(false);

        // evaluate the model
        bool l_result = l_model->eval({}, nullptr, 0, 1000, 1000).value();

        // check the result
        assert(l_result == false);
    }

    // immediately homogenous (truthy) model
    {
        std::unique_ptr<model> l_model = m(true);

        // evaluate the model
        bool l_result = l_model->eval({}, nullptr, 0, 1000, 1000).value();

        // check the result
        assert(l_result == true);
    }
}

void model_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_boolify);
    TEST(test_eval_binning_program);
    TEST(test_model_construct_and_print);
    TEST(test_model_eval);
}

#endif // UNIT_TEST
