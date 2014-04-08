#ifndef DSTORM_EXPRESSION_TYPES_H
#define DSTORM_EXPRESSION_TYPES_H

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/optional/optional.hpp>
#include <Eigen/Core>
#include "localization/Traits_decl.h"
#include <boost/fusion/include/vector.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "expression/tokens_decl.h"
#include "expression/Variable.h"
#include "expression/DynamicQuantity.h"

namespace dStorm {
namespace expression {

namespace fsn = boost::fusion;

struct variable_table_index {
    int i;
  public:
    variable_table_index( int i = -1 ) : i(i) {}
    operator int() const { return i; }
};

typedef boost::ptr_vector<Variable> variable_table;

struct VariableTable
: public variable_table {};

typedef boost::variant< 
    variable_table_index,
    bool,
    DynamicQuantity,
    boost::recursive_wrapper< tokens::power >,
    boost::recursive_wrapper< tokens::plus >,
    boost::recursive_wrapper< tokens::minus >,
    boost::recursive_wrapper< tokens::multiplies >,
    boost::recursive_wrapper< tokens::divides >,
    boost::recursive_wrapper< tokens::equal_to >,
    boost::recursive_wrapper< tokens::not_equal_to >,
    boost::recursive_wrapper< tokens::less >,
    boost::recursive_wrapper< tokens::greater >,
    boost::recursive_wrapper< tokens::less_equal >,
    boost::recursive_wrapper< tokens::greater_equal >,
    boost::recursive_wrapper< tokens::logical_and >,
    boost::recursive_wrapper< tokens::logical_or >,
    boost::recursive_wrapper< tokens::logical_not >,
    boost::recursive_wrapper< tokens::choice >
> tree_node;

struct AbstractSyntaxTree 
{
    tree_node root_node;
};

bool operator==( const DynamicUnit& a, const DynamicUnit& b );
bool operator!=( const DynamicUnit& a, const DynamicUnit& b );

DynamicQuantity pow( const DynamicQuantity& a, const DynamicQuantity& b );
DynamicQuantity operator+( const DynamicQuantity& a, const DynamicQuantity& b );
DynamicQuantity operator-( const DynamicQuantity& a, const DynamicQuantity& b );
DynamicQuantity operator*( const DynamicQuantity& a, const DynamicQuantity& b );
DynamicQuantity operator/( const DynamicQuantity& a, const DynamicQuantity& b );

bool operator==( const DynamicQuantity& a, const DynamicQuantity& b );
bool operator!=( const DynamicQuantity& a, const DynamicQuantity& b );
bool operator<=( const DynamicQuantity& a, const DynamicQuantity& b );
bool operator>=( const DynamicQuantity& a, const DynamicQuantity& b );
bool operator<( const DynamicQuantity& a, const DynamicQuantity& b );
bool operator>( const DynamicQuantity& a, const DynamicQuantity& b );

std::ostream& operator<<( std::ostream& o, const Variable* v );
std::ostream& operator<<( std::ostream& o, const dStorm::expression::DynamicUnit& v );
std::ostream& operator<<( std::ostream& o, const dStorm::expression::DynamicQuantity& v );

}
}

BOOST_FUSION_ADAPT_STRUCT(
    dStorm::expression::DynamicQuantity,
    (double, value)
    (dStorm::expression::DynamicUnit, unit)
)

#endif
