#ifndef DSTORM_EXPRESSION_EVALUATOR_H
#define DSTORM_EXPRESSION_EVALUATOR_H

#include "expression/tokens.h"
#include "expression/types.h"
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/variant/apply_visitor.hpp>

namespace dStorm {
namespace expression {

class Evaluator : public boost::static_visitor< boost::variant<DynamicQuantity,bool> >
{
    const Localization* current;
    const variable_table& variables;

    template <typename Result>
    Result apply_me( const tree_node& t ) {
        return boost::get<Result>( boost::apply_visitor(*this, t ) );
    }

  public:
    typedef boost::variant<DynamicQuantity,bool> result;

    Evaluator(const variable_table& t) : variables(t) {}
    void set_localization( const Localization* v ) { current = v; }

    result operator()( const DynamicQuantity& d ) { return d; }
    result operator()( const bool d ) { return d; }
    result operator()( const variable_table_index d ) { return variables[d].get( *current ); }
    template <typename Operator>
    typename boost::enable_if< boost::is_base_of<tokens::trinary_op, Operator>, result >::type
    operator()( const Operator& o ) { 
        return o( apply_me<typename Operator::first_argument_type>(o[0]),
                  apply_me<typename Operator::second_argument_type>(o[1]),
                  apply_me<typename Operator::third_argument_type>(o[2]) );
    }
    template <typename Operator>
    typename boost::enable_if< boost::is_base_of<tokens::binary_op, Operator>, result >::type
    operator()( const Operator& o ) { 
        return o( apply_me<typename Operator::first_argument_type>(o[0]),
                  apply_me<typename Operator::second_argument_type>(o[1]) );
    }
    template <typename Operator>
    typename boost::enable_if< boost::is_base_of<tokens::unary_op, Operator>, result >::type
    operator()( const Operator& o ) { 
        return o( apply_me<typename Operator::argument_type>(o[0]) );
    }
};

}
}

#endif
