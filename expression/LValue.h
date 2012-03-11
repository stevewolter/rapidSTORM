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

class ExpressionBasedLValue : public LValue {
protected:
    mutable tree_node original, simple;
    ExpressionBasedLValue( const tree_node& expression, const tree_node& def_simple ) : original(expression), simple(def_simple) {}

    void simplify( const variable_table&, const input::Traits<Localization>& ) const;
    void evaluate( const variable_table&, iterator begin, iterator end, EvaluationResult* result ) const;
};

}
}
}

#endif
