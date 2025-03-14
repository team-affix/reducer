#include <map>
#include <functional>
#include "include/bool_reduce.hpp"

using two_way_rule = std::pair<bool_node, bool_node>;

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
bool_node new_goal(std::list<two_way_rule>& a_helpers)
{
    // get index of new goal
    size_t l_goal_index = a_helpers.size();
    // construct functor
    bool_node l_functor = helper(l_goal_index, {});
    // make definition cyclic to indicate definition DNE
    a_helpers.push_back({l_functor, l_functor});
    // return new functor
    return l_functor;
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
/////////////// UNIFICATION & PARAMS ///////////////
////////////////////////////////////////////////////

// unifies a parameterized lhs with a concrete rhs, leaving a bindings map
bool unify(
    const bool_node& a_parameterized,
    const bool_node& a_concrete,
    std::map<size_t, bool_node>& a_bindings)
{
    if (const param_t* l_param = std::get_if<param_t>(&a_parameterized.m_data))
    {
        // look up current binding in map
        auto l_binding = a_bindings.find(l_param->m_index);
        // if not found, store current concrete
        if (l_binding == a_bindings.end())
        a_bindings[l_param->m_index] = a_concrete;
        // if found, check that concrete values match
        else if (l_binding->second != a_concrete)
            return false;
    }
    // both lhs and rhs are concrete, just ensure the datas match
    else if (a_parameterized.m_data != a_concrete.m_data)
        return false;
    

    // loop through the children and make sure they all unify
    for (int i = 0; i < a_parameterized.m_children.size(); ++i)
    {
        if (!unify(a_parameterized.m_children[i], a_concrete.m_children[i], a_bindings))
            return false;
    }

    return true;

}

// takes a lhs which contains parameters. It replaces each parameter with the value of its binding. If no binding is present, a goal
//     is produced, and that parameter is bound to and replaced with that new goal.
bool_node substitute_params(const bool_node& a_original, std::map<size_t, bool_node>& a_bindings, std::list<two_way_rule>& a_helpers)
{
    // look up param if parameter
    if (const param_t* l_param = std::get_if<param_t>(&a_original.m_data))
    {
        // look up current binding in map
        auto l_binding = a_bindings.find(l_param->m_index);
        // if not found, generate global goal and bind
        if (l_binding == a_bindings.end())
            return a_bindings[l_param->m_index] = new_goal(a_helpers);
        // if found, just return the binding value
        return l_binding->second;
    }
    
    std::vector<bool_node> l_result_children;

    // loop through the children and make sure they all unify
    for (const bool_node& l_child : a_original.m_children)
        l_result_children.push_back(substitute_params(l_child, a_bindings, a_helpers));

    // substitute children
    return bool_node(a_original.m_data, l_result_children);
}

////////////////////////////////////////////////////
//////////////// RULES OF REPLACEMENT //////////////
////////////////////////////////////////////////////

std::list<two_way_rule> g_two_way_rules =
{
    // invert rules
    {invert(zero()), one()},
    {invert(one()), zero()},

    // disjoin rules
    {disjoin(param(0), one()), one()},
    {disjoin(zero(), zero()), zero()},

    // conjoin rules
    {conjoin(param(0), zero()), zero()},
    {conjoin(one(), one()), one()},

    // andtrue, orfalse
    {param(0), conjoin(param(0), one())},
    {param(0), disjoin(param(0), zero())},

    // annihilation rules
    {conjoin(invert(param(0)), param(0)), zero()},
    {disjoin(invert(param(0)), param(0)), one ()},

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

    // absorption
    {disjoin(param(0), conjoin(param(0), param(1))), param(0)},
    {conjoin(param(0), disjoin(param(0), param(1))), param(0)},
    {disjoin(param(0), param(0)), param(0)},
    {conjoin(param(0), param(0)), param(0)},

    // distribution
    {conjoin(param(0), disjoin(param(1), param(2))), disjoin(conjoin(param(0), param(1)), conjoin(param(0), param(2)))},
    {disjoin(param(0), conjoin(param(1), param(2))), conjoin(disjoin(param(0), param(1)), disjoin(param(0), param(2)))},

};

// staging function, takes a pair specifying a two-way rule, and stages the evaluation of the forward rule
std::function<void()> stage_rule_fwd(bool_node& a_node, const std::pair<bool_node, bool_node>& a_pair, std::list<two_way_rule>& a_helpers)
{
    std::map<size_t, bool_node> l_bindings;
    if (!unify(a_pair.first, a_node, l_bindings))
        return nullptr;
    return [&a_node, &a_pair, l_bindings, &a_helpers]
    {
        std::map<size_t, bool_node> l_curried_bindings = l_bindings;
        a_node = substitute_params(a_pair.second, l_curried_bindings, a_helpers);
    };
}

// staging function, takes a pair specifying a two-way rule, and stages the evaluation of the backward rule
std::function<void()> stage_rule_bwd(bool_node& a_node, const std::pair<bool_node, bool_node>& a_pair, std::list<two_way_rule>& a_helpers)
{
    // just swap order of pair, and refer to stage_rule_fwd
    return stage_rule_fwd(a_node, {a_pair.second, a_pair.first}, a_helpers);
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

void test_new_goal()
{
    std::list<two_way_rule> l_helpers;

    assert(new_goal(l_helpers) == helper(0, {}));
    assert(l_helpers.size() == 1);
    assert((l_helpers.back() == two_way_rule{helper(0, {}), helper(0, {})}));
    
    assert(new_goal(l_helpers) == helper(1, {}));
    assert(l_helpers.size() == 2);
    assert((l_helpers.back() == two_way_rule{helper(1, {}), helper(1, {})}));
    
    assert(new_goal(l_helpers) == helper(2, {}));
    assert(l_helpers.size() == 3);
    assert((l_helpers.back() == two_way_rule{helper(2, {}), helper(2, {})}));
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
    
    {
        std::map<size_t, bool_node> l_bindings =
        {
            {0, invert(var(1))},
            {1, disjoin(var(0), var(3))},
            {2, invert(conjoin(var(4), var(5)))},
        };
        std::list<two_way_rule> l_helpers;
        // sub with 0 params
        assert(disjoin(var(1), var(2)) == substitute_params(disjoin(var(1), var(2)), l_bindings, l_helpers));
        assert(l_helpers.size() == 0);    
    }

    {
        std::map<size_t, bool_node> l_bindings =
        {
            {0, invert(var(1))},
            {1, disjoin(var(0), var(3))},
            {2, invert(conjoin(var(4), var(5)))},
        };
        std::list<two_way_rule> l_helpers;
        // sub with 1 params
        assert(disjoin(var(1), invert(invert(conjoin(var(4), var(5))))) == substitute_params(disjoin(var(1), invert(param(2))), l_bindings, l_helpers));
        assert(l_helpers.size() == 0);
    }
    
    {
        std::map<size_t, bool_node> l_bindings =
        {
            {0, invert(var(1))},
            {1, disjoin(var(0), var(3))},
            {2, invert(conjoin(var(4), var(5)))},
        };
        std::list<two_way_rule> l_helpers;
        // sub with 2 params
        assert(disjoin(invert(var(1)), disjoin(var(0), var(3))) == substitute_params(disjoin(param(0), param(1)), l_bindings, l_helpers));
        assert(l_helpers.size() == 0);
    }

    {
        std::map<size_t, bool_node> l_bindings =
        {
            {0, invert(var(1))},
            {1, disjoin(var(0), var(3))},
            {2, invert(conjoin(var(4), var(5)))},
        };
        std::list<two_way_rule> l_helpers;
        // sub with unbound params
        assert(helper(0, {}) == substitute_params(param(3), l_bindings, l_helpers));
        assert(l_bindings.size() == 4);
        assert(l_helpers.size() == 1);
    }

    {
        std::map<size_t, bool_node> l_bindings =
        {
            {0, invert(var(1))},
            {1, disjoin(var(0), var(3))},
            {2, invert(conjoin(var(4), var(5)))},
        };
        std::list<two_way_rule> l_helpers;
        // sub with multiple unbound params
        assert(disjoin(helper(0, {}), helper(1, {})) == substitute_params(disjoin(param(3), param(4)), l_bindings, l_helpers));
        assert(l_bindings.size() == 5);
        assert(l_helpers.size() == 2);
    }

    {
        std::map<size_t, bool_node> l_bindings =
        {
            {0, invert(var(1))},
            {1, disjoin(var(0), var(3))},
            {2, invert(conjoin(var(4), var(5)))},
        };
        std::list<two_way_rule> l_helpers;
        // sub with multiple unbound params, and multiple occurrances of same param
        assert(disjoin(helper(0, {}), helper(0, {})) == substitute_params(disjoin(param(3), param(3)), l_bindings, l_helpers));
        assert(l_bindings.size() == 4);
        assert(l_helpers.size() == 1);
    }

    {
        std::map<size_t, bool_node> l_bindings =
        {
            {0, invert(var(1))},
            {1, disjoin(var(0), var(3))},
            {2, invert(conjoin(var(4), var(5)))},
        };
        std::list<two_way_rule> l_helpers;
        // sub with multiple unbound params, and multiple occurrances of same param
        assert(disjoin(helper(0, {}), helper(0, {})) == substitute_params(disjoin(param(3), param(3)), l_bindings, l_helpers));
        assert(l_bindings.size() == 4);
        assert(l_helpers.size() == 1);
    }

    {
        std::map<size_t, bool_node> l_bindings =
        {
            {0, invert(var(1))},
            {1, disjoin(var(0), var(3))},
            {2, invert(conjoin(var(4), var(5)))},
        };
        std::list<two_way_rule> l_helpers
        {
            {helper(0, {}), helper(0, {})},
            {helper(1, {param(0)}), invert(invert(invert(param(0))))}, // just random helper definitions
        };
        // test with helpers already present
        // sub with multiple unbound params, and multiple occurrances of same param
        assert(disjoin(helper(2, {}), helper(2, {})) == substitute_params(disjoin(param(3), param(3)), l_bindings, l_helpers));
        assert(l_bindings.size() == 4);
        assert(l_helpers.size() == 3);
    }
}

void test_stage_rule_fwd()
{
    // THIS IS ONE OF THE MOST IMPORTANT TEST FUNCTIONS, AS
    //     THIS stage_rule_fwd IS ONE OF THE MOST USED FXNS
    //     AND IT IS COMPLEX.

    struct test_data
    {
        bool_node               m_original_node;
        std::list<two_way_rule> m_original_helpers;
        bool_node               m_final_node;
        std::list<two_way_rule> m_final_helpers;
    };

    data_points<two_way_rule, test_data> l_data_points
    {
        {
            two_way_rule{ zero(), invert(one()) },
            test_data{
                zero(),
                {},
                invert(one()),
                {},
            },
        },
        {
            two_way_rule{ param(0), invert(invert(param(0))) },
            test_data{
                var(0),
                {},
                invert(invert(var(0))),
                {},
            },
        },
        {
            two_way_rule{ disjoin(param(0), param(0)), param(0) },
            test_data{
                disjoin(var(0), var(0)),
                {},
                var(0),
                {},
            },
        },
        {
            two_way_rule{ disjoin(param(0), param(0)), param(0) },
            test_data{
                disjoin(var(3), var(3)),
                {},
                var(3),
                {},
            },
        },
        {
            two_way_rule{ param(0), conjoin(param(0), param(0)) },
            test_data{
                var(5),
                {},
                conjoin(var(5), var(5)),
                {},
            },
        },
        {
            two_way_rule{ param(0), conjoin(param(0), param(0)) },
            test_data{
                helper(0, {}),
                {
                    {helper(0, {}), helper(0, {})},
                },
                conjoin(helper(0, {}), helper(0, {})),
                {
                    {helper(0, {}), helper(0, {})},
                },
            },
        },
        {
            two_way_rule{ zero(), conjoin(zero(), param(0)) },
            test_data{
                zero(),
                {},
                conjoin(zero(), helper(0, {})),
                {
                    {helper(0, {}), helper(0, {})},
                },
            },
        },
        {
            two_way_rule{ conjoin(disjoin(param(0), invert(param(1))), disjoin(param(2), param(1))), disjoin(param(0), param(2)) },
            test_data{
                conjoin(disjoin(var(13), invert(var(10))), disjoin(var(14), var(10))),
                {},
                disjoin(var(13), var(14)),
                {},
            },
        },
        {
            two_way_rule{ disjoin(param(0), param(2)), conjoin(disjoin(param(0), invert(param(1))), disjoin(param(2), param(1))) },
            test_data{
                disjoin(var(13), var(14)),
                {
                    {helper(0, {}), helper(0, {})},
                },
                conjoin(disjoin(var(13), invert(helper(1, {}))), disjoin(var(14), helper(1, {}))),
                {
                    {helper(0, {}), helper(0, {})},
                    {helper(1, {}), helper(1, {})},
                },
            },
        },
        {
            two_way_rule{ helper(0, {}), invert(var(0)) }, // contrived example
            test_data{
                helper(0, {}),
                {
                    {helper(0, {}), helper(0, {})},
                },
                invert(var(0)),
                {
                    {helper(0, {}), helper(0, {})},
                },
            },
        },
    };

    // loop thru checking all test data points
    for (const auto& [l_rule, l_data] : l_data_points)
    {
        // construct test fields
        bool_node l_node = l_data.m_original_node;
        std::list<two_way_rule> l_helpers = l_data.m_original_helpers;
        // stage change
        auto l_staged = stage_rule_fwd(l_node, l_rule, l_helpers);
        // make sure staging succeeded
        assert(l_staged != nullptr);
        // make sure everything is the same still
        assert(l_node == l_data.m_original_node);
        assert(l_helpers == l_data.m_original_helpers);
        // commit change
        l_staged();
        std::cout << l_node << std::endl;
        assert(l_node == l_data.m_final_node);
        assert(l_helpers == l_data.m_final_helpers);
    }

    struct failure_test_data
    {
        bool_node               m_original_node;
        std::list<two_way_rule> m_original_helpers;
    };

    // designate some failure tests (staging should fail)
    data_points<two_way_rule, failure_test_data> l_failure_tests =
    {
        {
            two_way_rule{ zero(), invert(one()) },
            failure_test_data{
                one(),
                {},
            },
        },
        {
            two_way_rule{ zero(), invert(one()) },
            failure_test_data{
                var(0),
                {},
            },
        },
        {
            two_way_rule{ disjoin(param(0), param(0)), param(0) },
            failure_test_data{
                disjoin(var(3), var(4)),
                {},
            },
        },
        {
            two_way_rule{ conjoin(disjoin(param(0), invert(param(1))), disjoin(param(2), param(1))), disjoin(param(0), param(2)) },
            failure_test_data{
                conjoin(disjoin(var(13), invert(var(10))), disjoin(var(14), var(11))),
                {},
            },
        },
        {
            two_way_rule{ helper(0, {}), invert(var(0)) }, // contrived example
            failure_test_data{
                var(1),
                {
                    {helper(0, {}), helper(0, {})},
                },
            },
        },
    };

    for (const auto& [l_rule, l_data] : l_failure_tests)
    {
        // construct test fields
        bool_node l_node = l_data.m_original_node;
        std::list<two_way_rule> l_helpers = l_data.m_original_helpers;
        // stage change
        auto l_staged = stage_rule_fwd(l_node, l_rule, l_helpers);
        // make sure staging failed
        assert(l_staged == nullptr);
        // make sure everything is the same still
        assert(l_node == l_data.m_original_node);
        assert(l_helpers == l_data.m_original_helpers);
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
    TEST(test_param_construct_and_equality_check);
    TEST(test_helper_construct_and_equality_check);
    TEST(test_new_goal);
    TEST(test_bool_node_ostream_inserter);
    TEST(test_unify);
    TEST(test_substitute_params);
    TEST(test_stage_rule_fwd);
    
}

#endif
