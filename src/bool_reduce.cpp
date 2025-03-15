#include <map>
#include <functional>
#include "include/bool_reduce.hpp"

using replacement_rule = std::pair<bool_node, bool_node>;

////////////////////////////////////////////////////
////////////////// NODE DATA TYPES /////////////////
////////////////////////////////////////////////////

// equality comparisons
bool operator==(const zero_t&, const zero_t&) { return true; }
bool operator==(const one_t&, const one_t&) { return true; }
bool operator==(const var_t& a_lhs, const var_t& a_rhs) { return a_lhs.m_index == a_rhs.m_index; }
bool operator==(const invert_t&, const invert_t&) { return true; }
bool operator==(const disjoin_t&, const disjoin_t&) { return true; }
bool operator==(const conjoin_t&, const conjoin_t&) { return true; }
bool operator==(const helper_t& a_lhs, const helper_t& a_rhs) { return a_lhs.m_index == a_rhs.m_index; }

// less than comparisons
bool operator<(const zero_t&, const zero_t&) { return false; }
bool operator<(const one_t&, const one_t&) { return false; }
bool operator<(const var_t& a_lhs, const var_t& a_rhs) { return a_lhs.m_index < a_rhs.m_index; }
bool operator<(const invert_t&, const invert_t&) { return false; }
bool operator<(const disjoin_t&, const disjoin_t&) { return false; }
bool operator<(const conjoin_t&, const conjoin_t&) { return false; }
bool operator<(const helper_t& a_lhs, const helper_t& a_rhs) { return a_lhs.m_index < a_rhs.m_index; }

////////////////////////////////////////////////////
////////////////// MAKER FUNCTIONS /////////////////
////////////////////////////////////////////////////

bool_node zero()
{
    return bool_node(zero_t{}, {});
}
bool_node one()
{
    return bool_node(one_t{}, {});
}
bool_node var(size_t a_index)
{
    return bool_node(var_t{a_index}, {});
}
bool_node invert(const bool_node& a_x)
{
    return bool_node(invert_t{}, {a_x});
}
bool_node disjoin(const bool_node& a_x, const bool_node& a_y)
{
    return bool_node(disjoin_t{}, {a_x, a_y});
}
bool_node conjoin(const bool_node& a_x, const bool_node& a_y)
{
    return bool_node(conjoin_t{}, {a_x, a_y});
}
bool_node helper(size_t a_index, const std::vector<bool_node>& a_children)
{
    return bool_node(helper_t{a_index}, a_children);
}

////////////////////////////////////////////////////
////////////////// OSTREAM INSERTER ////////////////
////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& a_ostream, const bool_node& a_node)
{
    if (std::get_if<zero_t>(&a_node.m_data))
        return a_ostream << "zero";

    if (std::get_if<one_t>(&a_node.m_data))
        return a_ostream << "one";

    if (const var_t* l_data = std::get_if<var_t>(&a_node.m_data))
        return a_ostream << l_data->m_index;

    if (std::get_if<invert_t>(&a_node.m_data))
        return a_ostream << "~{" << a_node.m_children[0] << "}";

    if (std::get_if<disjoin_t>(&a_node.m_data))
        return a_ostream << "[" << a_node.m_children[0] << "]|[" << a_node.m_children[1] << "]";

    if (std::get_if<conjoin_t>(&a_node.m_data))
        return a_ostream << "(" << a_node.m_children[0] << ")&(" << a_node.m_children[1] << ")";

    if (const helper_t* l_data = std::get_if<helper_t>(&a_node.m_data))
    {
        a_ostream << "helper" << l_data->m_index << "<";
        
        if (a_node.m_children.size() > 0)
            a_ostream << a_node.m_children[0];
        
        for (int i = 1; i < a_node.m_children.size(); ++i)
            a_ostream << "," << a_node.m_children[i];
        
        return a_ostream << ">";

    }

    throw std::runtime_error("Error, invalid node data type.");
    
}

bool operator<(const terminate_t&,const terminate_t&) { return false; }
bool operator<(const make_function_t&,const make_function_t&) { return false; }

////////////////////////////////////////////////////
//////////////// FUNCTION GENERATION ///////////////
////////////////////////////////////////////////////

bool_node build_function(
    size_t& a_arity,
    const std::vector<size_t>& a_helper_arities,
    monte_carlo::simulation<choice_t, std::mt19937>& a_simulation,
    const size_t& a_recursion_limit)
{
    ////////////////////////////////////////////////////
    ////////////// POPULATE CHOICE VECTOR //////////////
    ////////////////////////////////////////////////////
    std::vector<choice_t> l_choices;

    l_choices.push_back(zero_t{});
    l_choices.push_back(one_t{});

    // allow choosing any variable (allow overshooting by 1)
    for (int i = 0; i <= a_arity; ++i)
        l_choices.push_back(var_t{(size_t)i});

    if (a_recursion_limit > 0)
    {
        // if the recursion limit has not been reached, push non-nullary function types into list

        l_choices.push_back(invert_t{});
        l_choices.push_back(disjoin_t{});
        l_choices.push_back(conjoin_t{});
        
        // allow choosing any helper
        for (int i = 0; i < a_helper_arities.size(); ++i)
            l_choices.push_back(helper_t{(size_t)i});
    }

    ////////////////////////////////////////////////////
    ////////////// CHOOSE A NODE TO PLACE //////////////
    ////////////////////////////////////////////////////
    choice_t l_choice = a_simulation.choose(l_choices);
    
    // extract the op data, it should always be an op data type.
    bool_op_data l_op_data = std::get<bool_op_data>(l_choice);

    ////////////////////////////////////////////////////
    //////////////// CONSTRUCT CHILDREN ////////////////
    ////////////////////////////////////////////////////
    size_t l_node_arity = 0;
    
    if (const var_t* l_var_data = std::get_if<var_t>(&l_op_data))
    {
        // allow overshooting by 1. In this case, increment var count.
        if (l_var_data->m_index == a_arity)
            ++a_arity;
    }
    else if (std::get_if<invert_t>(&l_op_data))
        l_node_arity = 1;
    else if (std::get_if<disjoin_t>(&l_op_data) || std::get_if<conjoin_t>(&l_op_data))
        l_node_arity = 2;
    else if (const helper_t* l_helper_data = std::get_if<helper_t>(&l_op_data))
        l_node_arity = a_helper_arities[l_helper_data->m_index];
    
    std::vector<bool_node> l_children;
    
    // loop through, construct children
    for (int i = 0; i < l_node_arity; ++i)
        l_children.push_back(build_function(a_arity, a_helper_arities, a_simulation, a_recursion_limit - 1));

    ////////////////////////////////////////////////////
    //////////////// CONSTRUCT THE NODE ////////////////
    ////////////////////////////////////////////////////
    return bool_node(l_op_data, l_children);

}

bool_node build_model(
    const size_t&           a_arity,
    std::vector<size_t>&    a_helper_arities,
    std::vector<bool_node>& a_helpers,
    monte_carlo::simulation<choice_t, std::mt19937>& a_simulation,
    const size_t& a_recursion_limit)
{
    ////////////////////////////////////////////////////
    ////////////// CONSTRUCT HELPERS LIST //////////////
    ////////////////////////////////////////////////////
    std::vector<size_t>    l_helper_arities;
    std::vector<bool_node> l_helpers;

    ////////////////////////////////////////////////////
    //////////////// CONSTRUCT HELPERS /////////////////
    ////////////////////////////////////////////////////
    std::vector<choice_t> l_choices {terminate_t{}, make_function_t{}};
    
    choice_t l_choice;

    do
    {
        size_t    l_helper_arity = 0;
        bool_node l_helper = build_function(l_helper_arity, l_helper_arities, a_simulation, a_recursion_limit);

        l_helper_arities.push_back(l_helper_arity);
        l_helpers.push_back(l_helper);
        
        l_choice = a_simulation.choose(l_choices);
    }
    while(std::get_if<make_function_t>(&l_choice));

    ////////////////////////////////////////////////////
    ////////////// BIND GLOBALS TO LOCALS //////////////
    ////////////////////////////////////////////////////
    std::vector<choice_t> l_mapping_choices;

    // push all global variable indices into the mapping vector
    for (int i = 0; i < a_arity; ++i)
        l_mapping_choices.push_back(var_t{(size_t)i});
    
    size_t l_final_helper_arity = l_helper_arities.back();

    std::vector<bool_node> l_model_children;
    
    // map from helper vars to model vars
    for (int i = 0; i < l_final_helper_arity; ++i)
    {
        choice_t l_choice = a_simulation.choose(l_mapping_choices);
        const bool_op_data l_op_data = std::get<bool_op_data>(l_choice);
        l_model_children.push_back(bool_node(l_op_data, {}));
    }

    // construct the final node
    return helper(a_helpers.size() - 1, l_model_children);

}

////////////////////////////////////////////////////
////////////////////// TESTING /////////////////////
////////////////////////////////////////////////////
#ifdef UNIT_TEST

#include <sstream>
#include "test_utils.hpp"

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
    assert(disjoin(var(0), var(1)) == disjoin(var(0), var(1)));
    assert(disjoin(var(0), var(1)) != disjoin(var(1), var(1)));
    assert(disjoin(var(0), var(1)) != disjoin(var(0), var(2)));
    bool_node l_node = disjoin(var(0), var(1));
    // check data field
    assert(std::get_if<disjoin_t>(&l_node.m_data));
}

void test_conjoin_construct_and_equality_check()
{
    assert(conjoin(var(0), var(1)) == conjoin(var(0), var(1)));
    assert(conjoin(var(0), var(1)) != conjoin(var(1), var(1)));
    assert(conjoin(var(0), var(1)) != conjoin(var(0), var(2)));
    bool_node l_node = conjoin(var(0), var(1));
    // check data field
    assert(std::get_if<conjoin_t>(&l_node.m_data));
}

void test_helper_construct_and_equality_check()
{
    assert(helper(0, {var(0), var(1)}) == helper(0, {var(0), var(1)}));
    assert(helper(0, {var(0), var(1)}) != helper(1, {var(0), var(1)}));
    assert(helper(0, {var(0), var(1)}) != helper(0, {var(1), var(1)}));
    assert(helper(0, {var(0), var(1)}) != helper(0, {var(0), var(0)}));
    assert(helper(0, {var(0), var(1)}) != helper(0, {var(0), var(0), var(1)}));
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
        assert(l_ss.str() == "zero");
    }

    {
        bool_node l_node = one();
        std::stringstream l_ss;
        l_ss << l_node;
        assert(l_ss.str() == "one");
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
    
}

#endif
