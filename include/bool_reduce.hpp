#ifndef BOOL_REDUCER_CTX
#define BOOL_REDUCER_CTX

#include <variant>
#include <ostream>
#include <list>
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

// equality comparisons
bool operator==(const zero_t&, const zero_t&);
bool operator==(const one_t&, const one_t&);
bool operator==(const var_t&, const var_t&);
bool operator==(const invert_t&, const invert_t&);
bool operator==(const disjoin_t&, const disjoin_t&);
bool operator==(const conjoin_t&, const conjoin_t&);
bool operator==(const param_t&, const param_t&);
bool operator==(const helper_t&, const helper_t&);

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

using bool_node = node<bool_op_data>;

////////////////////////////////////////////////////
////////////////// MAKER FUNCTIONS /////////////////
////////////////////////////////////////////////////
bool_node zero();
bool_node one();
bool_node var(size_t a_index);
bool_node invert(const node<bool_op_data>& a_x);
bool_node disjoin(const node<bool_op_data>& a_x, const node<bool_op_data>& a_y);
bool_node conjoin(const node<bool_op_data>& a_x, const node<bool_op_data>& a_y);
bool_node param(size_t a_index);
bool_node helper(size_t a_index, const std::vector<node<bool_op_data>>& a_children);

////////////////////////////////////////////////////
////////////////// OSTREAM INSERTER ////////////////
////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& a_ostream, const bool_node& a_node);

////////////////////////////////////////////////////
//////////////// RULES OF REPLACEMENT //////////////
////////////////////////////////////////////////////


#endif
