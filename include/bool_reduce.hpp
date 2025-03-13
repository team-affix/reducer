#ifndef BOOL_REDUCER_CTX
#define BOOL_REDUCER_CTX

#include <variant>
#include <ostream>
#include "node.hpp"

////////////////////////////////////////////////////
////////////////// NODE DATA TYPES /////////////////
////////////////////////////////////////////////////
struct zero_t   {};
struct one_t    {};
struct var_t     { size_t m_index; };
struct invert_t  {};
struct disjoin_t {};
struct conjoin_t {};
struct param_t   { size_t m_index; };
struct helper_t  { size_t m_index; };

// define the data type for the expression nodes
using bool_op_data = std::variant<
    zero_t,
    one_t,
    var_t,
    invert_t,
    disjoin_t,
    conjoin_t,
    param_t,
    helper_t>;

////////////////////////////////////////////////////
////////////////// MAKER FUNCTIONS /////////////////
////////////////////////////////////////////////////
node<bool_op_data> zero();
node<bool_op_data> one();
node<bool_op_data> var(size_t a_index);
node<bool_op_data> invert(const node<bool_op_data>& a_x);
node<bool_op_data> disjoin(const node<bool_op_data>& a_x, const node<bool_op_data>& a_y);
node<bool_op_data> conjoin(const node<bool_op_data>& a_x, const node<bool_op_data>& a_y);
node<bool_op_data> param(size_t a_index);
node<bool_op_data> helper(size_t a_index, const std::vector<node<bool_op_data>>& a_children);

////////////////////////////////////////////////////
////////////////// OSTREAM INSERTER ////////////////
////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& a_ostream, const node<bool_op_data>& a_node);

#endif
