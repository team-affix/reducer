#include "../include/model.hpp"
#include "../mcts/include/mcts.hpp"
#include "lambda.hpp"
#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

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
            return l_var->m_index % 2 == 0;
        else if(const app* l_app = dynamic_cast<const app*>(l_expr->get()))
            l_expr = &l_app->m_lhs;
        else if(const func* l_func = dynamic_cast<const func*>(l_expr->get()))
            l_expr = &l_func->m_body;
        else
            throw std::runtime_error("Error: invalid expression in boolify.");
    }
}

std::optional<bool> eval_binning_program(
    const std::unique_ptr<lambda::expr>& a_binning_program,
    const std::unique_ptr<lambda::expr>* a_args, size_t a_arity,
    const std::chrono::milliseconds& a_time_limit, size_t a_size_limit)
{
    using namespace lambda;

    auto l_norm_operand = a_binning_program->clone();

    // THEN build the application tower
    for(size_t i = 0; i < a_arity; ++i)
        l_norm_operand = a(std::move(l_norm_operand), a_args[i]->clone());

    // normalize the application with step and size limits
    bool l_time_excess = false;
    bool l_size_excess = false;

    // start the timer
    auto l_start_time = std::chrono::high_resolution_clock::now();

    while(reduce_one_step(l_norm_operand))
    {
        // check if the time limit has been exceeded
        if(std::chrono::high_resolution_clock::now() - l_start_time >
           a_time_limit)
        {
            l_time_excess = true;
            break;
        }

        // check if the size limit has been exceeded
        if(l_norm_operand->m_size > a_size_limit)
        {
            l_size_excess = true;
            break;
        }
    }

    // if the evaluation is too complex, return std::nullopt
    if(l_time_excess || l_size_excess)
        return std::nullopt;

    // boolify the result
    bool l_binning_result = boolify(l_norm_operand);

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
            const std::unique_ptr<lambda::expr>* a_args, size_t a_arity,
            const std::chrono::milliseconds& a_time_limit, size_t a_size_limit)
{
    using namespace lambda;

    // if the model is homogenous, then return the homogenous value
    if(m_func == nullptr)
        return m_homogenous_value;

    // construct the program
    auto l_program =
        construct_program(a_helpers.begin(), a_helpers.end(), m_func->clone());

    // evaluate the binning program
    auto l_binning_result = eval_binning_program(l_program, a_args, a_arity,
                                                 a_time_limit, a_size_limit);

    // if the binning program evaluation is too complex, return std::nullopt
    if(!l_binning_result.has_value())
        return std::nullopt;

    // get the appropriate child
    const auto& l_child =
        (*l_binning_result) ? m_positive_child : m_negative_child;

    // evaluate the child
    return l_child->eval(a_helpers, a_args, a_arity, a_time_limit,
                         a_size_limit);
}

size_t model::size() const
{
    if(m_func == nullptr)
        // this is a leaf node, still counts
        return 1;

    return 1 + m_func->m_size + m_positive_child->size() +
           m_negative_child->size();
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
    return false;
}
bool operator<(const place_func_node&, const place_func_node&)
{
    return false;
}
bool operator<(const place_app_node&, const place_app_node&)
{
    return false;
}
bool operator<(const select_var_index& a_lhs, const select_var_index& a_rhs)
{
    return a_lhs.m_index < a_rhs.m_index;
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
////////////////////// DATA POINT //////////////////
////////////////////////////////////////////////////

data_point::data_point(std::vector<std::unique_ptr<lambda::expr>>&& a_inputs,
                       bool a_output)
    : m_inputs(std::move(a_inputs)), m_output(a_output)
{
}

// gets pointers to the data points
std::vector<const data_point*>
data_pointers(const std::vector<data_point>& a_data)
{
    std::vector<const data_point*> l_pointers;
    std::transform(a_data.begin(), a_data.end(), std::back_inserter(l_pointers),
                   [](const data_point& a_data_point)
                   { return &a_data_point; });
    return l_pointers;
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

    // add var choice always (assume we have available var always)
    l_choices.push_back(place_var_node{});

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
        // get the specific var it wants to choose
        l_choices.clear();
        for(size_t i = 0; i < a_binder_depth; ++i)
            l_choices.push_back(select_var_index{i});
        l_choice = a_simulation.choose(l_choices);

        // cast it since we know it's a select_var_index
        select_var_index l_select_var_index =
            std::get<select_var_index>(l_choice);

        // return the variable
        return v(l_select_var_index.m_index);
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
build_model(const std::list<std::unique_ptr<lambda::expr>>& a_helpers,
            const std::vector<const data_point*>& a_data,
            const std::chrono::milliseconds& a_time_limit,
            const size_t& a_size_limit, const size_t& a_arity,
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
        // if(l_normalization_complex)
        // {
        //     std::cout << "normalization complex" << std::endl;
        // }
        // if(l_positive_bin.empty() || l_negative_bin.empty())
        // {
        //     std::cout << "noncontingent" << std::endl;
        // }

        // clear BOTH bins in case one contains items
        l_positive_bin.clear();
        l_negative_bin.clear();
        l_normalization_complex = false;

        // construct the binning function
        // [create a binning function that will bin (evaluate
        // on) each data point]
        l_binning_function = build_function(a_helpers.size(), a_arity,
                                            a_simulation, a_recursion_limit);

        // construct the program
        auto l_program = construct_program(a_helpers.begin(), a_helpers.end(),
                                           l_binning_function->clone());

        ////////////////////////////////////////////////////
        ////////////// EVALUATE BINNING PROGRAM ////////////
        ////////////////////////////////////////////////////

        // evaluate the binning program on all of the
        // data points
        for(const auto* l_data_point : a_data)
        {
            // evaluate the binning program
            auto l_normalize_result =
                eval_binning_program(l_program, l_data_point->m_inputs.data(),
                                     a_arity, a_time_limit, a_size_limit);

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
        build_model(a_helpers, l_positive_bin, a_time_limit, a_size_limit,
                    a_arity, a_simulation, a_recursion_limit);

    // construct the negative child
    std::unique_ptr<model> l_negative_child =
        build_model(a_helpers, l_negative_bin, a_time_limit, a_size_limit,
                    a_arity, a_simulation, a_recursion_limit);

    // construct the final node
    return m(std::move(l_binning_function), std::move(l_positive_child),
             std::move(l_negative_child));
}

std::unique_ptr<model>
learn_model(const std::list<std::unique_ptr<lambda::expr>>& a_helpers,
            const std::vector<const data_point*>& a_data,
            const std::chrono::milliseconds& a_time_limit,
            const size_t& a_size_limit, const size_t& a_arity,
            const size_t& a_iterations, const size_t& a_recursion_limit,
            const double& a_exploration_constant)
{
    std::mt19937 l_rnd_gen(27);
    monte_carlo::tree_node<choice> l_root;

    // initialize the best reward to the lowest possible
    // value
    double l_best_reward = -std::numeric_limits<double>::infinity();
    std::unique_ptr<model> l_best_model;

    for(int i = 0; i < a_iterations; ++i)
    {
        // construct the simulation
        monte_carlo::simulation<choice, std::mt19937> l_sim(
            l_root, a_exploration_constant, l_rnd_gen);

        // construct the model
        std::unique_ptr<model> l_model =
            build_model(a_helpers, a_data, a_time_limit, a_size_limit, a_arity,
                        l_sim, a_recursion_limit);

        // // compute the reward (negative descriptive length)
        // double l_reward = -static_cast<double>(l_sim.length());

        // compute the reward (negative descriptive length)
        double l_reward = -static_cast<double>(l_model->size());

        // std::cout << "model: " << *l_model << std::endl;
        // std::cout << "reward: " << l_reward << std::endl;

        std::cout << "new model!" << std::endl;

        // save best model
        if(l_reward > l_best_reward)
        {
            l_best_reward = l_reward;
            l_best_model = std::move(l_model);

            std::cout << "best model: " << *l_best_model << std::endl;
            std::cout << "best reward: " << l_best_reward << std::endl;
            std::cout << std::endl;
        }

        // terminate the simulation
        l_sim.terminate(l_reward);
    }

    return l_best_model;
}

#ifdef UNIT_TEST
#include "../include/encode.hpp"
#include "../include/predef.hpp"
#include "test_utils.hpp"
#include <sstream>
#include <functional>

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

    // church bools
    assert(boolify(f(f(v(0)))) == true);
    assert(boolify(f(f(v(1)))) == false);

    // zero and one are falsy and truthy respectively
    assert(boolify(f(f(v(1)))) == false);
    assert(boolify(f(f(a(v(0), v(1))))) == true);
    // two is truthy
    assert(boolify(f(f(a(v(0), a(v(0), v(1)))))) == true);
}

void test_eval_binning_program()
{
    using namespace lambda;

    // var 0, step limit 1000, size limit 1000
    {
        std::unique_ptr<lambda::expr> l_binning_program = v(0);
        assert(eval_binning_program(l_binning_program, nullptr, 0,
                                    std::chrono::milliseconds(1000), 1000)
                   .value() == true);
    }

    // var 1, step limit 1000, size limit 1000
    {
        std::unique_ptr<lambda::expr> l_binning_program = v(1);
        assert(eval_binning_program(l_binning_program, nullptr, 0,
                                    std::chrono::milliseconds(1000), 1000)
                   .value() == false);
    }

    // func 0, step limit 1000, size limit 1000
    {
        std::unique_ptr<lambda::expr> l_binning_program = f(v(0));
        assert(eval_binning_program(l_binning_program, nullptr, 0,
                                    std::chrono::milliseconds(1000), 1000)
                   .value() == true);
    }

    // func 1, step limit 1000, size limit 1000
    {
        std::unique_ptr<lambda::expr> l_binning_program = f(v(1));
        assert(eval_binning_program(l_binning_program, nullptr, 0,
                                    std::chrono::milliseconds(1000), 1000)
                   .value() == false);
    }

    // omega (omega combinator), step limit 100, size limit 100, should be too
    // complex to evaluate
    {
        std::unique_ptr<lambda::expr> l_binning_program =
            a(f(a(v(0), v(0))), f(a(v(0), v(0))));
        assert(eval_binning_program(l_binning_program, nullptr, 0,
                                    std::chrono::milliseconds(1000),
                                    100) == std::nullopt);
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

        // construct the program
        auto l_true_program = construct_program(l_helpers.begin(),
                                                l_helpers.end(), TRUE->clone());

        // construct the program
        auto l_false_program = construct_program(
            l_helpers.begin(), l_helpers.end(), FALSE->clone());

        // evaluate the binning function
        assert(eval_binning_program(l_true_program, nullptr, 0,
                                    std::chrono::milliseconds(1000), 100)
                   .value() == true);

        // evaluate the binning function
        assert(eval_binning_program(l_false_program, nullptr, 0,
                                    std::chrono::milliseconds(1000), 100)
                   .value() == false);
    }

    // zero and one are falsy and truthy respectively
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

        // construct the program
        auto l_zero_program = construct_program(l_helpers.begin(),
                                                l_helpers.end(), ZERO->clone());
        auto l_one_program =
            construct_program(l_helpers.begin(), l_helpers.end(),
                              a(SUCC->clone(), ZERO->clone()));

        // evaluate the binning program
        assert(eval_binning_program(l_zero_program, nullptr, 0,
                                    std::chrono::milliseconds(1000), 100)
                   .value() == false);

        // evaluate the binning program
        assert(eval_binning_program(l_one_program, nullptr, 0,
                                    std::chrono::milliseconds(1000), 100)
                   .value() == true);
    }

    // succ of zero, step limit 4, size limit 100, should be too complex to eval
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

        // construct the program
        auto l_succ_zero_program =
            construct_program(l_helpers.begin(), l_helpers.end(),
                              a(SUCC->clone(), ZERO->clone()));

        // evaluate the binning program
        assert(eval_binning_program(l_succ_zero_program, nullptr, 0,
                                    std::chrono::milliseconds(0),
                                    100) == std::nullopt);
    }

    // succ of zero, step limit 100, size limit 10, should be too complex to
    // eval
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

        // construct the program
        auto l_succ_zero_program =
            construct_program(l_helpers.begin(), l_helpers.end(),
                              a(SUCC->clone(), ZERO->clone()));

        // evaluate the binning program
        assert(eval_binning_program(l_succ_zero_program, nullptr, 0,
                                    std::chrono::milliseconds(100),
                                    10) == std::nullopt);
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
    using namespace lambda;

    // immediately homogenous (falsy) model
    {
        std::unique_ptr<model> l_model = m(false);

        // evaluate the model
        bool l_result =
            l_model->eval({}, nullptr, 0, std::chrono::milliseconds(1000), 1000)
                .value();

        // check the result
        assert(l_result == false);
    }

    // immediately homogenous (truthy) model
    {
        std::unique_ptr<model> l_model = m(true);

        // evaluate the model
        bool l_result =
            l_model->eval({}, nullptr, 0, std::chrono::milliseconds(1000), 1000)
                .value();

        // check the result
        assert(l_result == true);
    }

    // unary model (constant fn)
    {
        auto l_model = m(f(v(0)), m(true), m(false));
        // construct arg
        auto l_truthy_arg = v(54);
        auto l_falsy_arg = v(55);
        // evaluate the model
        bool l_result = l_model
                            ->eval({}, &l_truthy_arg, 1,
                                   std::chrono::milliseconds(1000), 1000)
                            .value();
        // check the result
        assert(l_result == true);
        // evaluate the model
        l_result = l_model
                       ->eval({}, &l_falsy_arg, 1,
                              std::chrono::milliseconds(1000), 1000)
                       .value();
        // check the result
        assert(l_result == false);
    }

    // unary model (constant fn) (inverted bins)
    {
        auto l_model = m(f(v(0)), m(false), m(true));
        // construct arg
        auto l_truthy_arg = v(54);
        auto l_falsy_arg = v(55);
        // evaluate the model
        bool l_result = l_model
                            ->eval({}, &l_truthy_arg, 1,
                                   std::chrono::milliseconds(1000), 1000)
                            .value();
        // check the result
        assert(l_result == false);
        // evaluate the model
        l_result = l_model
                       ->eval({}, &l_falsy_arg, 1,
                              std::chrono::milliseconds(1000), 1000)
                       .value();
        // check the result
        assert(l_result == true);
    }

    // ternary model (selector btw two values)
    {
        // takes in condition, branch 0, branch 1
        auto l_model = m(f(f(f(a(a(v(0), v(1)), v(2))))), m(true), m(false));
        // define TRUE
        auto TRUE = f(f(v(0)));
        // define FALSE
        auto FALSE = f(f(v(1)));
        // construct arg
        std::vector<std::unique_ptr<lambda::expr>> l_truthy_args;
        l_truthy_args.push_back(TRUE->clone());
        l_truthy_args.push_back(v(54));
        l_truthy_args.push_back(v(55));
        // construct arg
        std::vector<std::unique_ptr<lambda::expr>> l_falsy_args;
        l_falsy_args.push_back(FALSE->clone());
        l_falsy_args.push_back(v(54));
        l_falsy_args.push_back(v(55));
        // evaluate the model
        bool l_result =
            l_model
                ->eval({}, l_truthy_args.data(), l_truthy_args.size(),
                       std::chrono::milliseconds(1000), 1000)
                .value();
        // check the result
        assert(l_result == true);
        // evaluate the model
        l_result = l_model
                       ->eval({}, l_falsy_args.data(), l_falsy_args.size(),
                              std::chrono::milliseconds(1000), 1000)
                       .value();
        // check the result
        assert(l_result == false);
    }

    // ternary model (simple boolean function)
    {
        // takes in condition, branch 0, branch 1
        auto l_root_bf = f(f(f(v(0))));
        auto l_left_bf = f(f(f(v(1))));
        auto l_right_bf = f(f(f(v(2))));
        // define model
        auto l_model =
            m(l_root_bf->clone(), m(l_left_bf->clone(), m(true), m(false)),
              m(l_right_bf->clone(), m(false), m(true)));
        // define TRUE
        auto TRUE = f(f(v(0)));
        // define FALSE
        auto FALSE = f(f(v(1)));
        // construct 8 truth rows
        std::vector<std::unique_ptr<lambda::expr>> l_row_0;
        l_row_0.push_back(FALSE->clone());
        l_row_0.push_back(FALSE->clone());
        l_row_0.push_back(FALSE->clone());

        std::vector<std::unique_ptr<lambda::expr>> l_row_1;
        l_row_1.push_back(FALSE->clone());
        l_row_1.push_back(FALSE->clone());
        l_row_1.push_back(TRUE->clone());

        std::vector<std::unique_ptr<lambda::expr>> l_row_2;
        l_row_2.push_back(FALSE->clone());
        l_row_2.push_back(TRUE->clone());
        l_row_2.push_back(FALSE->clone());

        std::vector<std::unique_ptr<lambda::expr>> l_row_3;
        l_row_3.push_back(FALSE->clone());
        l_row_3.push_back(TRUE->clone());
        l_row_3.push_back(TRUE->clone());

        std::vector<std::unique_ptr<lambda::expr>> l_row_4;
        l_row_4.push_back(TRUE->clone());
        l_row_4.push_back(FALSE->clone());
        l_row_4.push_back(FALSE->clone());

        std::vector<std::unique_ptr<lambda::expr>> l_row_5;
        l_row_5.push_back(TRUE->clone());
        l_row_5.push_back(FALSE->clone());
        l_row_5.push_back(TRUE->clone());

        std::vector<std::unique_ptr<lambda::expr>> l_row_6;
        l_row_6.push_back(TRUE->clone());
        l_row_6.push_back(TRUE->clone());
        l_row_6.push_back(FALSE->clone());

        std::vector<std::unique_ptr<lambda::expr>> l_row_7;
        l_row_7.push_back(TRUE->clone());
        l_row_7.push_back(TRUE->clone());
        l_row_7.push_back(TRUE->clone());

        // evaluate the model
        auto l_row_result = [&l_model](auto& a_args) -> std::optional<bool>
        {
            return l_model->eval({}, a_args.data(), a_args.size(),
                                 std::chrono::milliseconds(1000), 1000);
        };

        // evaluate the rows
        assert(l_row_result(l_row_0) == true);
        assert(l_row_result(l_row_1) == false);
        assert(l_row_result(l_row_2) == true);
        assert(l_row_result(l_row_3) == false);
        assert(l_row_result(l_row_4) == false);
        assert(l_row_result(l_row_5) == false);
        assert(l_row_result(l_row_6) == true);
        assert(l_row_result(l_row_7) == true);
    }

    // helper function to make a church numeral
    const auto l_numeral = [](size_t a_binder_depth,
                              size_t a_numeral) -> std::unique_ptr<lambda::expr>
    {
        auto l_result = v(a_binder_depth + 1);
        for(size_t i = 0; i < a_numeral; ++i)
            l_result = a(v(a_binder_depth + 0), std::move(l_result));
        return f(f(std::move(l_result)));
    };

    // unary model (single inequality)
    {
        // define helpers
        std::list<std::unique_ptr<lambda::expr>> l_helpers;

        // l() and g() pattern
        auto l = [&l_helpers](size_t a_index)
        { return v(a_index + l_helpers.size()); };
        auto g = [](size_t a_index) { return v(a_index); };

        // define TRUE
        const auto TRUE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(0))));

        // define FALSE
        const auto FALSE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(1))));

        // define NOT
        const auto NOT = g(l_helpers.size());
        l_helpers.emplace_back(f(a(a(l(0), FALSE->clone()), TRUE->clone())));

        // define zero
        const auto ZERO = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(1))));

        // define succ
        const auto SUCC = g(l_helpers.size());
        l_helpers.emplace_back(f(f(f(a(l(1), a(a(l(0), l(1)), l(2)))))));

        // define isZero
        const auto IS_ZERO = g(l_helpers.size());
        l_helpers.emplace_back(f(a(a(l(0), f(FALSE->clone())), TRUE->clone())));

        // define pair
        const auto PAIR = g(l_helpers.size());
        l_helpers.emplace_back(f(f(f(a(a(l(2), l(0)), l(1))))));

        // define fst
        const auto FST = g(l_helpers.size());
        l_helpers.emplace_back(f(a(l(0), TRUE->clone())));

        // define snd
        const auto SND = g(l_helpers.size());
        l_helpers.emplace_back(f(a(l(0), FALSE->clone())));

        // define pred
        const auto PRED = g(l_helpers.size());
        l_helpers.emplace_back(
            f(a(FST->clone(),
                a(a(l(0), // n
                    f(a(a(PAIR->clone(), a(SND->clone(),
                                           l(1) // p
                                           )),
                        a(SUCC->clone(), a(SND->clone(),
                                           l(1) // p
                                           ))))),
                  a(a(PAIR->clone(), ZERO->clone()), ZERO->clone())))));

        // define sub
        const auto SUB = g(l_helpers.size());
        l_helpers.emplace_back(f(f(a(a(l(1), // n
                                       PRED->clone()),
                                     l(0) // m
                                     ))));

        // define lessThan
        const auto LESS_THAN = g(l_helpers.size());
        l_helpers.emplace_back(
            f(f(a(NOT->clone(), a(IS_ZERO->clone(), a(a(SUB->clone(),
                                                        l(1) // n
                                                        ),
                                                      l(0) // m
                                                      ))))));

        // define the root binning function
        auto l_root_bf = f(a(a(LESS_THAN->clone(), l(0)),
                             l_numeral(l_helpers.size() + 1, 24)));

        // define the model
        auto l_model = m(l_root_bf->clone(), m(true), m(false));

        // construct sample inputs
        std::unique_ptr<lambda::expr> l_input_0 = l_numeral(0, 7);
        std::unique_ptr<lambda::expr> l_input_1 = l_numeral(0, 16);
        std::unique_ptr<lambda::expr> l_input_2 = l_numeral(0, 42);
        std::unique_ptr<lambda::expr> l_input_3 = l_numeral(0, 2);
        std::unique_ptr<lambda::expr> l_input_4 = l_numeral(0, 13);
        std::unique_ptr<lambda::expr> l_input_5 = l_numeral(0, 99);
        std::unique_ptr<lambda::expr> l_input_6 = l_numeral(0, 64);
        std::unique_ptr<lambda::expr> l_input_7 = l_numeral(0, 0);
        std::unique_ptr<lambda::expr> l_input_8 = l_numeral(0, 27);
        std::unique_ptr<lambda::expr> l_input_9 = l_numeral(0, 8);
        std::unique_ptr<lambda::expr> l_input_10 = l_numeral(0, 51);
        std::unique_ptr<lambda::expr> l_input_11 = l_numeral(0, 31);

        std::cout << "input 0: " << *l_input_0 << std::endl;
        std::cout << "input 1: " << *l_input_1 << std::endl;
        std::cout << "input 2: " << *l_input_2 << std::endl;
        std::cout << "input 3: " << *l_input_3 << std::endl;
        std::cout << "input 4: " << *l_input_4 << std::endl;
        std::cout << "input 5: " << *l_input_5 << std::endl;
        std::cout << "input 6: " << *l_input_6 << std::endl;
        std::cout << "input 7: " << *l_input_7 << std::endl;
        std::cout << "input 8: " << *l_input_8 << std::endl;
        std::cout << "input 9: " << *l_input_9 << std::endl;
        std::cout << "input 10: " << *l_input_10 << std::endl;
        std::cout << "input 11: " << *l_input_11 << std::endl;

        // evaluate the model
        auto l_row_result = [&l_helpers,
                             &l_model](const auto& a_arg) -> std::optional<bool>
        {
            return l_model->eval(l_helpers, &a_arg, 1,
                                 std::chrono::milliseconds(10000), 10000);
        };

        // evaluate the rows (is input less than 24?)
        assert(l_row_result(l_input_0) == true);   // 7 < 24
        assert(l_row_result(l_input_1) == true);   // 16 < 24
        assert(l_row_result(l_input_2) == false);  // 42 >= 24
        assert(l_row_result(l_input_3) == true);   // 2 < 24
        assert(l_row_result(l_input_4) == true);   // 13 < 24
        assert(l_row_result(l_input_5) == false);  // 99 >= 24
        assert(l_row_result(l_input_6) == false);  // 64 >= 24
        assert(l_row_result(l_input_7) == true);   // 0 < 24
        assert(l_row_result(l_input_8) == false);  // 27 >= 24
        assert(l_row_result(l_input_9) == true);   // 8 < 24
        assert(l_row_result(l_input_10) == false); // 51 >= 24
        assert(l_row_result(l_input_11) == false); // 31 >= 24
    }

    // unary model (cascading inqeualities)
    {
        // define helpers
        std::list<std::unique_ptr<lambda::expr>> l_helpers;

        // l() and g() pattern
        auto l = [&l_helpers](size_t a_index)
        { return v(a_index + l_helpers.size()); };
        auto g = [](size_t a_index) { return v(a_index); };

        // define TRUE
        const auto TRUE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(0))));

        // define FALSE
        const auto FALSE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(1))));

        // define NOT
        const auto NOT = g(l_helpers.size());
        l_helpers.emplace_back(f(a(a(l(0), FALSE->clone()), TRUE->clone())));

        // define zero
        const auto ZERO = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(1))));

        // define succ
        const auto SUCC = g(l_helpers.size());
        l_helpers.emplace_back(f(f(f(a(l(1), a(a(l(0), l(1)), l(2)))))));

        // define isZero
        const auto IS_ZERO = g(l_helpers.size());
        l_helpers.emplace_back(f(a(a(l(0), f(FALSE->clone())), TRUE->clone())));

        // define pair
        const auto PAIR = g(l_helpers.size());
        l_helpers.emplace_back(f(f(f(a(a(l(2), l(0)), l(1))))));

        // define fst
        const auto FST = g(l_helpers.size());
        l_helpers.emplace_back(f(a(l(0), TRUE->clone())));

        // define snd
        const auto SND = g(l_helpers.size());
        l_helpers.emplace_back(f(a(l(0), FALSE->clone())));

        // define pred
        const auto PRED = g(l_helpers.size());
        l_helpers.emplace_back(
            f(a(FST->clone(),
                a(a(l(0), // n
                    f(a(a(PAIR->clone(), a(SND->clone(),
                                           l(1) // p
                                           )),
                        a(SUCC->clone(), a(SND->clone(),
                                           l(1) // p
                                           ))))),
                  a(a(PAIR->clone(), ZERO->clone()), ZERO->clone())))));

        // define sub
        const auto SUB = g(l_helpers.size());
        l_helpers.emplace_back(f(f(a(a(l(1), // n
                                       PRED->clone()),
                                     l(0) // m
                                     ))));

        // define lessThan
        const auto LESS_THAN = g(l_helpers.size());
        l_helpers.emplace_back(
            f(f(a(NOT->clone(), a(IS_ZERO->clone(), a(a(SUB->clone(),
                                                        l(1) // n
                                                        ),
                                                      l(0) // m
                                                      ))))));

        // define the root binning function
        auto l_root_bf = f(a(a(LESS_THAN->clone(), l(0)),
                             l_numeral(l_helpers.size() + 1, 24)));

        // define the left binning function
        auto l_left_bf = f(a(
            a(LESS_THAN->clone(), l_numeral(l_helpers.size() + 1, 18)), l(0)));

        // define the right binning function
        auto l_right_bf = f(a(
            a(LESS_THAN->clone(), l_numeral(l_helpers.size() + 1, 28)), l(0)));

        // define the model
        auto l_model =
            m(l_root_bf->clone(), m(l_left_bf->clone(), m(true), m(false)),
              m(l_right_bf->clone(), m(true), m(false)));

        // construct sample inputs
        std::unique_ptr<lambda::expr> l_input_0 =
            l_numeral(0, 20); // satisfies (18,24)
        std::unique_ptr<lambda::expr> l_input_1 =
            l_numeral(0, 22); // satisfies (18,24)
        std::unique_ptr<lambda::expr> l_input_2 =
            l_numeral(0, 29); // satisfies (28 < input)
        std::unique_ptr<lambda::expr> l_input_3 =
            l_numeral(0, 31); // satisfies (28 < input)
        std::unique_ptr<lambda::expr> l_input_4 =
            l_numeral(0, 19); // satisfies (18,24)
        std::unique_ptr<lambda::expr> l_input_5 =
            l_numeral(0, 21); // satisfies (18,24)
        std::unique_ptr<lambda::expr> l_input_6 =
            l_numeral(0, 30); // satisfies (28 < input)
        std::unique_ptr<lambda::expr> l_input_7 =
            l_numeral(0, 23); // satisfies (18,24)
        std::unique_ptr<lambda::expr> l_input_8 =
            l_numeral(0, 24); // does NOT satisfy (edge)
        std::unique_ptr<lambda::expr> l_input_9 =
            l_numeral(0, 18); // does NOT satisfy (edge)
        std::unique_ptr<lambda::expr> l_input_10 =
            l_numeral(0, 28); // does NOT satisfy (edge)
        std::unique_ptr<lambda::expr> l_input_11 =
            l_numeral(0, 25); // does NOT satisfy (in gap [24,28])

        // evaluate the model
        auto l_row_result = [&l_helpers,
                             &l_model](const auto& a_arg) -> std::optional<bool>
        {
            return l_model->eval(l_helpers, &a_arg, 1,
                                 std::chrono::milliseconds(10000), 10000);
        };

        // evaluate the rows ((18 < input < 24) || (28 < input))
        assert(l_row_result(l_input_0) == true);   // 20 ∈ (18,24)
        assert(l_row_result(l_input_1) == true);   // 22 ∈ (18,24)
        assert(l_row_result(l_input_2) == true);   // 29 > 28
        assert(l_row_result(l_input_3) == true);   // 31 > 28
        assert(l_row_result(l_input_4) == true);   // 19 ∈ (18,24)
        assert(l_row_result(l_input_5) == true);   // 21 ∈ (18,24)
        assert(l_row_result(l_input_6) == true);   // 30 > 28
        assert(l_row_result(l_input_7) == true);   // 23 ∈ (18,24)
        assert(l_row_result(l_input_8) == false);  // 24 ∉ ranges (edge)
        assert(l_row_result(l_input_9) == false);  // 18 ∉ ranges (edge)
        assert(l_row_result(l_input_10) == false); // 28 ∉ ranges (edge)
        assert(l_row_result(l_input_11) == false); // 25 ∈ [24,28] (gap)
    }
}

void test_build_function_body()
{
    std::mt19937 l_rnd_gen(11);
    monte_carlo::tree_node<choice> l_root;
    monte_carlo::simulation<choice, std::mt19937> l_sim(l_root, 5, l_rnd_gen);
    auto l_function = build_function_body(1, l_sim, 5);
    auto l_function_2 = build_function_body(1, l_sim, 5);
    auto l_function_3 = build_function_body(1, l_sim, 5);
    auto l_function_4 = build_function_body(1, l_sim, 5);
    std::cout << "binder depth 1: " << *l_function << std::endl;
    std::cout << "binder depth 1: " << *l_function_2 << std::endl;
    std::cout << "binder depth 1: " << *l_function_3 << std::endl;
    std::cout << "binder depth 1: " << *l_function_4 << std::endl;
    auto l_function_5 = build_function_body(2, l_sim, 15);
    auto l_function_6 = build_function_body(3, l_sim, 15);
    auto l_function_7 = build_function_body(4, l_sim, 15);
    auto l_function_8 = build_function_body(5, l_sim, 15);
    std::cout << "binder depth 2: " << *l_function_5 << std::endl;
    std::cout << "binder depth 3: " << *l_function_6 << std::endl;
    std::cout << "binder depth 4: " << *l_function_7 << std::endl;
    std::cout << "binder depth 5: " << *l_function_8 << std::endl;
    auto l_function_9 = build_function_body(2, l_sim, 15);
    auto l_function_10 = build_function_body(3, l_sim, 15);
    auto l_function_11 = build_function_body(4, l_sim, 15);
    auto l_function_12 = build_function_body(5, l_sim, 15);
    std::cout << "binder depth 2: " << *l_function_9 << std::endl;
    std::cout << "binder depth 3: " << *l_function_10 << std::endl;
    std::cout << "binder depth 4: " << *l_function_11 << std::endl;
    std::cout << "binder depth 5: " << *l_function_12 << std::endl;
}

void test_build_function()
{
    std::mt19937 l_rnd_gen(12);
    monte_carlo::tree_node<choice> l_root;
    monte_carlo::simulation<choice, std::mt19937> l_sim(l_root, 5, l_rnd_gen);
    auto l_function = build_function(0, 1, l_sim, 5);
    std::cout << "binder depth 0, arity 1: " << *l_function << std::endl;
    auto l_function_2 = build_function(1, 1, l_sim, 5);
    std::cout << "binder depth 1, arity 1: " << *l_function_2 << std::endl;
    auto l_function_3 = build_function(2, 1, l_sim, 5);
    std::cout << "binder depth 2, arity 1: " << *l_function_3 << std::endl;
    auto l_function_4 = build_function(3, 1, l_sim, 5);
    std::cout << "binder depth 3, arity 1: " << *l_function_4 << std::endl;
    auto l_function_5 = build_function(4, 1, l_sim, 5);
    std::cout << "binder depth 4, arity 1: " << *l_function_5 << std::endl;
    auto l_function_6 = build_function(0, 2, l_sim, 5);
    std::cout << "binder depth 0, arity 2: " << *l_function_6 << std::endl;
    auto l_function_7 = build_function(1, 2, l_sim, 5);
    std::cout << "binder depth 1, arity 2: " << *l_function_7 << std::endl;
    auto l_function_8 = build_function(2, 2, l_sim, 5);
    std::cout << "binder depth 2, arity 2: " << *l_function_8 << std::endl;
    auto l_function_9 = build_function(3, 2, l_sim, 5);
    std::cout << "binder depth 3, arity 2: " << *l_function_9 << std::endl;
}

void test_build_model()
{
    using namespace lambda;

    std::mt19937 l_rnd_gen(21);
    monte_carlo::tree_node<choice> l_root;
    monte_carlo::simulation<choice, std::mt19937> l_sim(l_root, 5, l_rnd_gen);

    // no helpers in this example, just data

    // define TRUE
    const auto TRUE = f(f(v(0)));

    // define FALSE
    const auto FALSE = f(f(v(1)));

    // define data for binary exor
    std::vector<std::unique_ptr<lambda::expr>> l_inputs_0;
    l_inputs_0.push_back(FALSE->clone());
    l_inputs_0.push_back(FALSE->clone());
    std::vector<std::unique_ptr<lambda::expr>> l_inputs_1;
    l_inputs_1.push_back(FALSE->clone());
    l_inputs_1.push_back(TRUE->clone());
    std::vector<std::unique_ptr<lambda::expr>> l_inputs_2;
    l_inputs_2.push_back(TRUE->clone());
    l_inputs_2.push_back(FALSE->clone());
    std::vector<std::unique_ptr<lambda::expr>> l_inputs_3;
    l_inputs_3.push_back(TRUE->clone());
    l_inputs_3.push_back(TRUE->clone());
    std::vector<data_point> l_data;
    l_data.emplace_back(std::move(l_inputs_0), false);
    l_data.emplace_back(std::move(l_inputs_1), true);
    l_data.emplace_back(std::move(l_inputs_2), true);
    l_data.emplace_back(std::move(l_inputs_3), false);

    // define data pointers
    std::vector<const data_point*> l_data_pointers;
    std::transform(l_data.begin(), l_data.end(),
                   std::back_inserter(l_data_pointers),
                   [](const auto& a_data_point) { return &a_data_point; });

    auto l_model =
        build_model({}, l_data_pointers, std::chrono::milliseconds(1000), 1000,
                    2, l_sim, 5);
    std::cout << "model: " << *l_model << std::endl;
}

void test_learn_model()
{
    using namespace lambda;

    // binary exor
    {
        // no helpers in this example, just data

        // define TRUE
        const auto TRUE = f(f(v(0)));

        // define FALSE
        const auto FALSE = f(f(v(1)));

        // define data for binary exor
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_0;
        l_inputs_0.push_back(FALSE->clone());
        l_inputs_0.push_back(FALSE->clone());
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_1;
        l_inputs_1.push_back(FALSE->clone());
        l_inputs_1.push_back(TRUE->clone());
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_2;
        l_inputs_2.push_back(TRUE->clone());
        l_inputs_2.push_back(FALSE->clone());
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_3;
        l_inputs_3.push_back(TRUE->clone());
        l_inputs_3.push_back(TRUE->clone());
        std::vector<data_point> l_data;
        l_data.emplace_back(std::move(l_inputs_0), false);
        l_data.emplace_back(std::move(l_inputs_1), true);
        l_data.emplace_back(std::move(l_inputs_2), true);
        l_data.emplace_back(std::move(l_inputs_3), false);

        // define data pointers
        std::vector<const data_point*> l_data_pointers;
        std::transform(l_data.begin(), l_data.end(),
                       std::back_inserter(l_data_pointers),
                       [](const auto& a_data_point) { return &a_data_point; });

        auto l_model =
            learn_model({}, l_data_pointers, std::chrono::milliseconds(1), 1000,
                        2, 1000, 5, 5);
        std::cout << "model: " << *l_model << std::endl;
    }

    // ternary nested exor
    {
        // no helpers in this example, just data

        // define TRUE
        const auto TRUE = f(f(v(0)));

        // define FALSE
        const auto FALSE = f(f(v(1)));

        // define data for binary exor
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_0;
        l_inputs_0.push_back(FALSE->clone());
        l_inputs_0.push_back(FALSE->clone());
        l_inputs_0.push_back(FALSE->clone());
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_1;
        l_inputs_1.push_back(FALSE->clone());
        l_inputs_1.push_back(FALSE->clone());
        l_inputs_1.push_back(TRUE->clone());
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_2;
        l_inputs_2.push_back(FALSE->clone());
        l_inputs_2.push_back(TRUE->clone());
        l_inputs_2.push_back(FALSE->clone());
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_3;
        l_inputs_3.push_back(FALSE->clone());
        l_inputs_3.push_back(TRUE->clone());
        l_inputs_3.push_back(TRUE->clone());

        std::vector<std::unique_ptr<lambda::expr>> l_inputs_4;
        l_inputs_4.push_back(TRUE->clone());
        l_inputs_4.push_back(FALSE->clone());
        l_inputs_4.push_back(FALSE->clone());
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_5;
        l_inputs_5.push_back(TRUE->clone());
        l_inputs_5.push_back(FALSE->clone());
        l_inputs_5.push_back(TRUE->clone());
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_6;
        l_inputs_6.push_back(TRUE->clone());
        l_inputs_6.push_back(TRUE->clone());
        l_inputs_6.push_back(FALSE->clone());
        std::vector<std::unique_ptr<lambda::expr>> l_inputs_7;
        l_inputs_7.push_back(TRUE->clone());
        l_inputs_7.push_back(TRUE->clone());
        l_inputs_7.push_back(TRUE->clone());

        std::vector<data_point> l_data;
        l_data.emplace_back(std::move(l_inputs_0), false);
        l_data.emplace_back(std::move(l_inputs_1), true);
        l_data.emplace_back(std::move(l_inputs_2), true);
        l_data.emplace_back(std::move(l_inputs_3), false);
        l_data.emplace_back(std::move(l_inputs_4), true);
        l_data.emplace_back(std::move(l_inputs_5), false);
        l_data.emplace_back(std::move(l_inputs_6), false);
        l_data.emplace_back(std::move(l_inputs_7), true);

        // define data pointers
        std::vector<const data_point*> l_data_pointers;
        std::transform(l_data.begin(), l_data.end(),
                       std::back_inserter(l_data_pointers),
                       [](const auto& a_data_point) { return &a_data_point; });

        auto l_model =
            learn_model({}, l_data_pointers, std::chrono::milliseconds(1), 1000,
                        3, 1000, 5, 20);
        std::cout << "model: " << *l_model << std::endl;
    }

    // add(x, y) = z
    {
        using namespace dml::encode;

        // POSITIVE DATA
        std::vector<std::unique_ptr<lambda::expr>> l_pos_inputs_0;
        l_pos_inputs_0.emplace_back(binary_numeral(0, 0));
        l_pos_inputs_0.emplace_back(binary_numeral(0, 0));
        l_pos_inputs_0.emplace_back(binary_numeral(0, 0));

        std::vector<std::unique_ptr<lambda::expr>> l_pos_inputs_1;
        l_pos_inputs_1.emplace_back(binary_numeral(0, 0));
        l_pos_inputs_1.emplace_back(binary_numeral(0, 1));
        l_pos_inputs_1.emplace_back(binary_numeral(0, 1));

        std::vector<std::unique_ptr<lambda::expr>> l_pos_inputs_2;
        l_pos_inputs_2.emplace_back(binary_numeral(0, 1));
        l_pos_inputs_2.emplace_back(binary_numeral(0, 0));
        l_pos_inputs_2.emplace_back(binary_numeral(0, 1));

        std::vector<std::unique_ptr<lambda::expr>> l_pos_inputs_3;
        l_pos_inputs_3.emplace_back(binary_numeral(0, 2));
        l_pos_inputs_3.emplace_back(binary_numeral(0, 4));
        l_pos_inputs_3.emplace_back(binary_numeral(0, 6));

        std::vector<std::unique_ptr<lambda::expr>> l_pos_inputs_4;
        l_pos_inputs_4.emplace_back(binary_numeral(0, 8));
        l_pos_inputs_4.emplace_back(binary_numeral(0, 4));
        l_pos_inputs_4.emplace_back(binary_numeral(0, 12));

        // NEGATIVE DATA

        std::vector<std::unique_ptr<lambda::expr>> l_neg_inputs_0;
        l_neg_inputs_0.emplace_back(binary_numeral(0, 0));
        l_neg_inputs_0.emplace_back(binary_numeral(0, 1));
        l_neg_inputs_0.emplace_back(binary_numeral(0, 0));

        std::vector<std::unique_ptr<lambda::expr>> l_neg_inputs_1;
        l_neg_inputs_1.emplace_back(binary_numeral(0, 1));
        l_neg_inputs_1.emplace_back(binary_numeral(0, 1));
        l_neg_inputs_1.emplace_back(binary_numeral(0, 3));

        std::vector<std::unique_ptr<lambda::expr>> l_neg_inputs_2;
        l_neg_inputs_2.emplace_back(binary_numeral(0, 1));
        l_neg_inputs_2.emplace_back(binary_numeral(0, 5));
        l_neg_inputs_2.emplace_back(binary_numeral(0, 7));

        std::vector<std::unique_ptr<lambda::expr>> l_neg_inputs_3;
        l_neg_inputs_3.emplace_back(binary_numeral(0, 1));
        l_neg_inputs_3.emplace_back(binary_numeral(0, 0));
        l_neg_inputs_3.emplace_back(binary_numeral(0, 2));

        std::vector<std::unique_ptr<lambda::expr>> l_neg_inputs_4;
        l_neg_inputs_4.emplace_back(binary_numeral(0, 1));
        l_neg_inputs_4.emplace_back(binary_numeral(0, 6));
        l_neg_inputs_4.emplace_back(binary_numeral(0, 22));

        std::vector<data_point> l_data;
        l_data.emplace_back(std::move(l_pos_inputs_0), true);
        l_data.emplace_back(std::move(l_pos_inputs_1), true);
        l_data.emplace_back(std::move(l_pos_inputs_2), true);
        l_data.emplace_back(std::move(l_pos_inputs_3), true);
        l_data.emplace_back(std::move(l_pos_inputs_4), true);
        l_data.emplace_back(std::move(l_neg_inputs_0), false);
        l_data.emplace_back(std::move(l_neg_inputs_1), false);
        l_data.emplace_back(std::move(l_neg_inputs_2), false);
        l_data.emplace_back(std::move(l_neg_inputs_3), false);
        l_data.emplace_back(std::move(l_neg_inputs_4), false);

        // define data pointers
        std::vector<const data_point*> l_data_pointers;
        std::transform(l_data.begin(), l_data.end(),
                       std::back_inserter(l_data_pointers),
                       [](const auto& a_data_point) { return &a_data_point; });

        // CONSTRUCT HELPERS
        std::list<std::unique_ptr<lambda::expr>> l_helpers;

        // insert the 'add' helper
        l_helpers.emplace_back(dml::predef::binary_add(l_helpers.size()));
        l_helpers.emplace_back(dml::predef::binary_compare(l_helpers.size()));

        auto l_model = learn_model(l_helpers, l_data_pointers,
                                   std::chrono::milliseconds(10), 10000000, 3,
                                   1000, 200, 50);
        std::cout << "model: " << *l_model << std::endl;
    }
}

// struct hole : lambda::expr {
//     virtual ~hole() = default;
//
//     // ACCESSOR METHODS
//     bool equals(const std::unique_ptr<expr>& a_other) const override {
//         const hole* l_casted = dynamic_cast<const hole*>(a_other.get());
//
//         if(!l_casted)
//             return false;
//
//         return m_index == l_casted->m_index;
//     }
//     void print(std::ostream& a_ostream) const override {
//         a_ostream << "H" << m_index;
//     }
//     std::unique_ptr<expr> clone() const override {
//         return std::unique_ptr<expr>(new hole(m_index));
//     }
//
//     // MUTATOR METHODS
//     void update_size() override{
//         m_size = 1;
//     }
//     void lift(size_t a_lift_amount, size_t a_cutoff) override{
//         // do nothing
//     }
//
//     // MEMBER VARIABLES
//     size_t m_index;
//
//   private:
//     hole(size_t a_index);
// };

using lce = std::unique_ptr<lambda::expr>;
using signature = std::list<std::pair<lce, lce>>;

// struct dfb_key {
//     lce m_type;
//     size_t m_min_depth_requirement;
//     bool operator<(const dfb_key& a_other) {
//         if (*m_type < *a_other.m_type) return true;
//         if (a_other.m_type < m_type) return false;
//         return m_min_depth_requirement < a_other.m_min_depth_requirement;
//     }
// };
//
// std::multimap<dfb_key, lce> curry_sample(const signature& a_signature) {
//
// }

void model_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    // TEST(test_boolify);
    // TEST(test_eval_binning_program);
    // TEST(test_model_construct_and_print);
    // TEST(test_model_eval);
    // TEST(test_build_function_body);
    // TEST(test_build_function);
    // TEST(test_build_model);
    // TEST(test_learn_model);
    // TEST(test_implement_dtt);
}

#endif // UNIT_TEST
