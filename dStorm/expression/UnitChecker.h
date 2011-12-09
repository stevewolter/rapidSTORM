#ifndef DSTORM_EXPRESSION_UNITCHECKER_H
#define DSTORM_EXPRESSION_UNITCHECKER_H

#include "tokens.h"
#include "types.h"
#include <boost/utility/enable_if.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/type_traits/is_base_of.hpp>

namespace dStorm {
namespace expression {

class UnitChecker : public boost::static_visitor< boost::variant<DynamicQuantity,bool> >
{
    const variable_table& table;
    const input::Traits<Localization>& traits;

    template <typename Result>
    Result apply_me( const tree_node& t ) {
        return boost::get<Result>( boost::apply_visitor(*this, t ) );
    }

  public:
    typedef boost::variant<DynamicQuantity,bool> result;

    UnitChecker( const input::Traits<Localization>& traits, const variable_table& table ) : table(table), traits(traits) {}

    result operator()( const DynamicQuantity& d ) { return d; }
    result operator()( const bool d ) { return d; }
    result operator()( const variable_table_index d ) { return table[d].get(traits); }
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
