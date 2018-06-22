#include "expression/LValue.h"
#include "expression/Simplifier.h"
#include "expression/Evaluator.h"

namespace dStorm {
namespace expression {
namespace source {

LValue* new_clone( const LValue& v )
{
    return v.clone();
}

void ExpressionBasedLValue::simplify( const variable_table& tbl, const input::Traits<Localization>& traits ) const
{
    Simplifier s(traits, tbl);
    simple = boost::apply_visitor(s, original);
}

void ExpressionBasedLValue::evaluate( const variable_table& tbl, iterator begin, iterator end, EvaluationResult* result ) const
{
    Evaluator e(tbl);
    for ( iterator i = begin; i != end; ++i ) {
        e.set_localization( &*i );
        result[i-begin] = boost::apply_visitor(e, simple);
    }
}

}
}
}
