#include <map>
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
    if (a_parameterized.m_data != a_concrete.m_data)
        return false;

    // loop through the children and make sure they all unify
    for (int i = 0; i < a_parameterized.m_children.size(); ++i)
    {
        if (!unify(a_parameterized.m_children[i], a_concrete.m_children[i], a_resolutions))
            return false;
    }

    return true;

}

// void substitute_params(const bool_node&, const std::vector<bool_node>&, bool_node&);

#ifdef UNIT_TEST

#include "test_utils.hpp"

void test_zero_construct_and_equality_check()
{
    assert(zero() == zero());
    bool_node l_node = zero();
    // make sure the data field is actually a zero
    assert(std::get_if<zero_t>(&l_node.m_data));
}

void bool_reduce_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;
    
    TEST(test_zero_construct_and_equality_check);
}

#endif
