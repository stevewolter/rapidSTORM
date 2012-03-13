#include "debug.h"
#include "VariableLValue.h"
#include "LValue.h"
#include <boost/variant/get.hpp>

namespace dStorm {
namespace expression {

namespace source {

struct Assignment : public ExpressionBasedLValue {
    const Variable& v;

    Assignment( const Variable& v,  const tree_node& expression );
    Assignment* clone() const { return new Assignment(*this); }
    
    void announce( const variable_table& vt, input::Traits<Localization>& t ) const;
    iterator evaluate( const variable_table&, const input::Traits<Localization>& bounds,
                  iterator, iterator ) const;
};

Assignment::Assignment( const Variable& v,  const tree_node& expression )
: ExpressionBasedLValue(expression, DynamicQuantity()), v(v) {}

void Assignment::announce( const variable_table& vt, input::Traits<Localization>& t ) const 
{ 
    ExpressionBasedLValue::simplify(vt, t); 
    DEBUG( "Simplifying announcement " << this << " from " << original << " to " << simple );
    tree_node& result = ExpressionBasedLValue::simple;
    if ( DynamicQuantity *rv = boost::get<DynamicQuantity>(&result) ) {
        v.set( t, *rv );
    } else if ( v.is_static(t) ) {
        throw std::runtime_error("The expression for the static quantity " + v.name
                                    + " contains dynamic variables");
    } else {
        v.set( t, DynamicQuantity() );
    }
}

Assignment::iterator
Assignment::evaluate( const variable_table& vt, const input::Traits<Localization>& bounds,
                iterator begin, iterator end ) const
{
    DEBUG( "Evaluating announcement " << this << " (was " << original << ") from " << simple );
    EvaluationResult r[end-begin];
    ExpressionBasedLValue::evaluate(vt, begin, end, r);

    iterator end_of_good = end;
    for (iterator i = begin; i < end_of_good; ) {
        bool good = v.set( bounds, *i, boost::get<DynamicQuantity>(r[i-begin]) );
        if ( good )
            ++i;
        else {
            --end_of_good;
            *i = *end_of_good;
            r[i-begin] = r[end_of_good-begin];
        }
    }
    return end_of_good;
}

std::auto_ptr<LValue> make_variable_lvalue( 
    const Variable& v, const std::string& s, const Parser& p )
{
    return std::auto_ptr<LValue>( new Assignment( v, p.parse_numeric(s)->root_node ) );
}

}

namespace config {

struct VariableLValue : public LValue
{
    const Variable& v;
    boost::shared_ptr<AbstractSyntaxTree> expression;

    VariableLValue( const Variable& v ) 
        : LValue(v.name, v.name), v(v) {}
    VariableLValue* clone() const { return new VariableLValue(*this); }
    source::LValue* make_lvalue() const { return new source::Assignment( v, expression->root_node ); }
    void set_expression_string( const std::string& expression, Parser& parser ) {
        this->expression = parser.parse_numeric( expression );
        DEBUG( this << " parsed expression " << this->expression->root_node << " from " << expression );
    }
};

std::auto_ptr<LValue> make_variable_lvalue( const Variable& i ) {
    return std::auto_ptr<LValue>( new VariableLValue(i) );
}

}
}
}
