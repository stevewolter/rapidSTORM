#ifndef DSTORM_EXPRESSION_TOKENS_H
#define DSTORM_EXPRESSION_TOKENS_H

#include "expression/tokens_decl.h"

#include <array>

#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/io.hpp>

#include "expression/types.h"

namespace dStorm {
namespace expression {

namespace tokens {

typedef std::array< tree_node, 1 > unary_op;
typedef std::array< tree_node, 2 > binary_op;
typedef std::array< tree_node, 3 > trinary_op;

struct power : public binary_op {
    typedef DynamicQuantity first_argument_type;
    typedef DynamicQuantity second_argument_type;
    typedef DynamicQuantity result_type;

    power( const tree_node& base, const tree_node& exponent ) 
        { (*this)[0] = base; (*this)[1] = exponent; }
    DynamicQuantity operator()( const DynamicQuantity& lhs, const DynamicQuantity& rhs ) const
        { return pow( lhs, rhs ); }
};
inline std::ostream& operator<<( std::ostream& o, const power& v )  \
    { return (o << "(" << v[0] << " pow " << v[1] << ")"); }

#define ADAPT_STD_FUNCTIONAL_BINARY(x,y) \
struct x : public binary_op, public std::x<y> { \
    x( const tree_node& a, const tree_node& b ) { (*this)[0] = a; (*this)[1] = b; } \
}; \
inline std::ostream& operator<<( std::ostream& o, const x& v )  \
    { return (o << "(" << v[0] << " " << #x << " " << v[1] << ")"); }

#define ADAPT_STD_FUNCTIONAL_UNARY(x,y) \
struct x : public unary_op, public std::x<y> { \
    x( const tree_node& a ) { (*this)[0] = a; } \
}; \
inline std::ostream& operator<<( std::ostream& o, const x& v )  \
    { return (o << "(" << v[0] << " " << #x << ")"); }

ADAPT_STD_FUNCTIONAL_BINARY(multiplies, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(divides, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(plus, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(minus, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(equal_to, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(not_equal_to, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(less, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(greater, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(less_equal, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(greater_equal, DynamicQuantity)
ADAPT_STD_FUNCTIONAL_BINARY(logical_and, bool)
ADAPT_STD_FUNCTIONAL_BINARY(logical_or, bool)
ADAPT_STD_FUNCTIONAL_UNARY(logical_not, bool)

struct choice : public trinary_op {
    typedef bool first_argument_type;
    typedef DynamicQuantity second_argument_type;
    typedef DynamicQuantity third_argument_type;
    typedef DynamicQuantity result_type;

    choice( const tree_node& a, const tree_node& b, const tree_node& c )
        { (*this)[0] = a; (*this)[1] = b; (*this)[2] = c; }
    DynamicQuantity operator()( bool c, const DynamicQuantity& a, const DynamicQuantity& b ) const
        { return (c) ? a : b; }
};
inline std::ostream& operator<<( std::ostream& o, const choice& v )  \
    { return (o << "(" << v[0] << " if_true " << v[1] << " else " << v[2] << ")"); }

}

}
}

#endif
