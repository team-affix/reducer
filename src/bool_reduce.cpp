#include <map>
#include <functional>
#include "include/bool_reduce.hpp"

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
bool operator==(const param_t& a_lhs, const param_t& a_rhs) { return a_lhs.m_index == a_rhs.m_index; }
bool operator==(const helper_t& a_lhs, const helper_t& a_rhs) { return a_lhs.m_index == a_rhs.m_index; }

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
bool_node param(size_t a_index)
{
    return bool_node(param_t{a_index}, {});
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

    if (const param_t* l_data = std::get_if<param_t>(&a_node.m_data))
        return a_ostream << "param" << l_data->m_index;

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

////////////////////////////////////////////////////
//////////////// RULES OF REPLACEMENT //////////////
////////////////////////////////////////////////////

std::list<std::pair<bool_node, bool_node>> g_bidirectional_rules =
{
    // basic rules involving 1 and 0
    {zero(), invert(one())},
    {zero(), disjoin(zero(), zero())},
    {zero(), conjoin(zero(), zero())},
    {zero(), conjoin(zero(), one())},
    {zero(), conjoin(one(), zero())},
    {one(), invert(zero())},
    {one(), disjoin(zero(), one())},
    {one(), disjoin(one(), zero())},
    {one(), disjoin(one(), one())},
    {one(), conjoin(one(), one())},

    // associativity
    {conjoin(param(0), conjoin(param(1), param(2))), conjoin(conjoin(param(0), param(1)), param(2))},
    {disjoin(param(0), disjoin(param(1), param(2))), disjoin(disjoin(param(0), param(1)), param(2))},

    // commutativity
    {conjoin(param(0), param(1)), conjoin(param(1), param(0))},
    {disjoin(param(0), param(1)), disjoin(param(1), param(0))},

    // double-negation
    {param(0), invert(invert(param(0)))},
    
    // demorgans
    {invert(conjoin(param(0), param(1))), disjoin(invert(param(0)), invert(param(1)))},
    {invert(disjoin(param(0), param(1))), conjoin(invert(param(0)), invert(param(1)))},

};

// unifies a parameterized lhs with a concrete rhs, leaving a resolutions map
bool unify(
    const bool_node& a_parameterized,
    const bool_node& a_concrete,
    std::map<size_t, bool_node>& a_resolutions)
{
    if (const param_t* l_param = std::get_if<param_t>(&a_parameterized.m_data))
    {
        // look up current resolution in map
        auto l_resolution = a_resolutions.find(l_param->m_index);
        // if not found, store current concrete
        if (l_resolution == a_resolutions.end())
            a_resolutions[l_param->m_index] = a_concrete;
        // if found, check that concrete values match
        else if (l_resolution->second != a_concrete)
            return false;
    }
    // both lhs and rhs are concrete, just ensure the datas match
    else if (a_parameterized.m_data != a_concrete.m_data)
        return false;
    

    // loop through the children and make sure they all unify
    for (int i = 0; i < a_parameterized.m_children.size(); ++i)
    {
        if (!unify(a_parameterized.m_children[i], a_concrete.m_children[i], a_resolutions))
            return false;
    }

    return true;

}

bool_node substitute_params(const bool_node& a_original, const std::map<size_t, bool_node>& a_bindings)
{
    // look up param if parameter
    if (const param_t* l_param = std::get_if<param_t>(&a_original.m_data))
        return a_bindings.at(l_param->m_index);
    
    std::vector<bool_node> l_result_children;

    // loop through the children and make sure they all unify
    for (const bool_node& l_child : a_original.m_children)
        l_result_children.push_back(substitute_params(l_child, a_bindings));

    // substitute children
    return bool_node(a_original.m_data, l_result_children);
}

std::function<void()> stage_rule_fwd(const std::pair<bool_node, bool_node>& a_pair, bool_node& a_node)
{
    std::map<size_t, bool_node> l_bindings;
    if (!unify(a_pair.first, a_node, l_bindings))
        return nullptr;
    return [&a_node, &a_pair, l_bindings] { a_node = substitute_params(a_pair.second, l_bindings); };
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

void test_param_construct_and_equality_check()
{
    assert(param(0) == param(0));
    assert(param(0) != param(1));
    bool_node l_node = param(1);
    // check data field
    param_t* l_data;
    assert(l_data = std::get_if<param_t>(&l_node.m_data));
    assert(l_data->m_index == 1);
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

    {
        bool_node l_node = param(7);
        std::stringstream l_ss;
        l_ss << l_node;
        assert(l_ss.str() == "param7");
    }

    {
        bool_node l_node = helper(8, {var(5), param(0), invert(param(1))});
        std::stringstream l_ss;
        l_ss << l_node;
        assert(l_ss.str() == "helper8<5,param0,~{param1}>");
    }
    
}

void test_unify()
{
    std::map<size_t, bool_node> l_bindings;

    // test unify same thing
    assert(unify(var(0), var(0), l_bindings));
    assert(l_bindings.size() == 0);
    l_bindings.clear();

    // test unify different things
    assert(!unify(var(0), var(1), l_bindings));
    assert(l_bindings.size() == 0);
    l_bindings.clear();

    // test unify parameter with var
    assert(unify(param(0), var(2), l_bindings));
    assert(l_bindings.size() == 1);
    assert(l_bindings[0] == var(2));
    l_bindings.clear();

    // test unify invert with invert
    assert(unify(invert(var(0)), invert(var(0)), l_bindings));
    assert(l_bindings.size() == 0);
    l_bindings.clear();

    // test unify fails with inequal inners
    assert(!unify(invert(var(0)), invert(var(1)), l_bindings));
    assert(l_bindings.size() == 0);
    l_bindings.clear();

    // test unify param with internal of invert
    assert(unify(invert(param(0)), invert(var(3)), l_bindings));
    assert(l_bindings.size() == 1);
    assert(l_bindings[0] == var(3));
    l_bindings.clear();

    // test unify fails with multiple resolutions of same parameter
    assert(!unify(disjoin(param(0), param(0)), disjoin(var(0), var(1)), l_bindings));
    assert(l_bindings.size() == 1);
    l_bindings.clear();

    // test unify fails with multiple resolutions of same parameter
    assert(!unify(disjoin(param(0), invert(param(0))), disjoin(var(0), invert(var(1))), l_bindings));
    assert(l_bindings.size() == 1);
    l_bindings.clear();

    // test unify succeeds with single resolution of same parameter
    assert(unify(disjoin(param(0), invert(param(0))), disjoin(var(2), invert(var(2))), l_bindings));
    assert(l_bindings.size() == 1);
    l_bindings.clear();
    
}

void test_substitute_params()
{
    std::map<size_t, bool_node> l_bindings =
    {
        {0, invert(var(1))},
        {1, disjoin(var(0), var(3))},
        {2, invert(conjoin(var(4), var(5)))},
    };

    // sub with 0 params
    assert(disjoin(var(1), var(2)) == substitute_params(disjoin(var(1), var(2)), l_bindings));

    // sub with 1 params
    assert(disjoin(var(1), invert(invert(conjoin(var(4), var(5))))) == substitute_params(disjoin(var(1), invert(param(2))), l_bindings));

    // sub with 2 params
    assert(disjoin(invert(var(1)), disjoin(var(0), var(3))) == substitute_params(disjoin(param(0), param(1)), l_bindings));
    
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
    TEST(test_param_construct_and_equality_check);
    TEST(test_helper_construct_and_equality_check);
    TEST(test_bool_node_ostream_inserter);
    TEST(test_unify);
    TEST(test_substitute_params);
    
}

#endif
