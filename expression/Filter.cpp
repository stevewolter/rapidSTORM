#include "Filter.h"
#include "LValue.h"
#include <stdexcept>
#include <boost/variant/get.hpp>

namespace dStorm {
namespace expression {

namespace source {

class Filter : public ExpressionBasedLValue {
    void announce( const variable_table& vt, input::Traits<Localization>& t ) const { 
        simplify(vt, t); 
    }
    iterator evaluate( const variable_table&, const input::Traits<Localization>& bounds,
                  iterator, iterator ) const;
public:
    Filter( boost::shared_ptr< const AbstractSyntaxTree > expression ) 
        : ExpressionBasedLValue(expression->root_node, true) {}
    LValue* clone() const { return new Filter(*this); }
};

Filter::iterator Filter::evaluate( const variable_table& tbl, const input::Traits<Localization>&,
                      iterator begin, iterator end ) const
{
    const int count = end - begin;
    EvaluationResult r[count];
    bool good[count];
    ExpressionBasedLValue::evaluate(tbl, begin, end, r);
    for ( int i = 0; i < count; ++i )
        /* The result of this evaluation should *always* be a boolean,
         * since our grammar guarantees it. */
        good[i] = boost::get<bool>(r[i]);

    int end_of_good = count;
    for ( int i = 0; i < end_of_good; ) {
        if ( ! good[end_of_good-1] )
            --end_of_good;
        else if ( good[i] )
            ++i;
        else {
            --end_of_good;
            std::swap( *(begin+i), *(begin+end_of_good) );
            ++i;
        }
    }
    return begin+end_of_good;
}

std::auto_ptr<LValue> make_filter( const std::string& expression, const Parser& parser ) {
    return std::auto_ptr<LValue>( new Filter( parser.parse_boolean( expression ) ) );
}

}

namespace config {

class Filter : public LValue {
    boost::shared_ptr<AbstractSyntaxTree> expression;

    source::Filter* make_lvalue() const { return new source::Filter(expression); }
    void set_expression_string( const std::string& expression, Parser& parser )  {
        this->expression = parser.parse_boolean( expression );
    }
public:
    Filter() : LValue("expression", "Filter") {}
    Filter* clone() const { return new Filter(*this); }
};

std::auto_ptr<LValue> make_filter() {
    return std::auto_ptr<LValue>( new Filter() );
}

}
}
}
