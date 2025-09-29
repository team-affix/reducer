#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "func.hpp"
#include "type.hpp"
#include <map>

// contains all objects of all types
using scope = std::multimap<type, const func*>;

#endif
