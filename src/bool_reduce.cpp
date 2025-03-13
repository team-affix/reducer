#include "include/bool_reduce.hpp"

////////////////////////////////////////////////////
////////////////// MAKER FUNCTIONS /////////////////
////////////////////////////////////////////////////
node<bool_op_data> zero()
{
    return node<bool_op_data>(zero_t{}, {});
}
node<bool_op_data> one()
{
    return node<bool_op_data>(one_t{}, {});
}
node<bool_op_data> var(size_t a_index)
{
    return node<bool_op_data>(var_t{a_index}, {});
}
node<bool_op_data> invert(const node<bool_op_data>& a_x)
{
    return node<bool_op_data>(invert_t{}, {a_x});
}
node<bool_op_data> disjoin(const node<bool_op_data>& a_x, const node<bool_op_data>& a_y)
{
    return node<bool_op_data>(disjoin_t{}, {a_x, a_y});
}
node<bool_op_data> conjoin(const node<bool_op_data>& a_x, const node<bool_op_data>& a_y)
{
    return node<bool_op_data>(conjoin_t{}, {a_x, a_y});
}
node<bool_op_data> param(size_t a_index)
{
    return node<bool_op_data>(param_t{a_index}, {});
}
node<bool_op_data> helper(size_t a_index, const std::vector<node<bool_op_data>>& a_children)
{
    return node<bool_op_data>(helper_t{a_index}, a_children);
}

////////////////////////////////////////////////////
////////////////// OSTREAM INSERTER ////////////////
////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& a_ostream, const node<bool_op_data>& a_node)
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
