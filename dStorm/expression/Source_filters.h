#ifndef DSTORM_EXPRESSION_SOURCE_FILTERS_H
#define DSTORM_EXPRESSION_SOURCE_FILTERS_H

#include "types.h"
#include "tokens.h"
#include <dStorm/traits/tags.h>
#include "Source.h"
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace expression {
namespace source {

struct LValue {
    typedef boost::variant<DynamicQuantity, bool> EvaluationResult;
    typedef std::vector<Localization>::iterator iterator;

    virtual ~LValue() {}
    virtual LValue* clone() const = 0;
    virtual iterator evaluate( const variable_table&, const input::Traits<Localization>&, 
        iterator begin, iterator end ) const = 0;
    virtual void announce( const variable_table&, input::Traits<Localization>& ) const = 0;
};

struct ExpressionBasedLValue : public LValue {
    mutable tree_node original, simple;
    ExpressionBasedLValue( const tree_node& expression, const tree_node& def_simple ) : original(expression), simple(def_simple) {}

    void simplify( const variable_table&, const input::Traits<Localization>& ) const;
    void evaluate( const variable_table&, iterator begin, iterator end, EvaluationResult* result ) const;
};

struct Filter : public ExpressionBasedLValue {
    Filter( const tree_node& expression ) : ExpressionBasedLValue(expression, true) {}
    LValue* clone() const { return new Filter(*this); }
    void announce( const variable_table& vt, input::Traits<Localization>& t ) const { simplify(vt, t); }
    iterator evaluate( const variable_table&, const input::Traits<Localization>& bounds,
                  iterator, iterator ) const;
};

struct Assignment : public ExpressionBasedLValue {
    boost::shared_ptr<variable> v;

    Assignment( const variable& v,  const tree_node& expression );
    Assignment* clone() const { return new Assignment(*this); }
    
    void announce( const variable_table& vt, input::Traits<Localization>& t ) const;
    iterator evaluate( const variable_table&, const input::Traits<Localization>& bounds,
                  iterator, iterator ) const;
};

}
}
}

#endif
