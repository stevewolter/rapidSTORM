#ifndef DSTORM_EXPRESSION_EVALUATOR_H
#define DSTORM_EXPRESSION_EVALUATOR_H

#include "tokens.h"
#include "types.h"
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>

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

#if 0
class Simplifier : public boost::static_visitor<>
{
    const std::vector<token>& token_stack;
    std::vector<token> result;
    const input::Traits<Localization>& traits;
  public:
    Simplifier( const std::vector<token>& input, const input::Traits<Localization>& t )
        : token_stack(input), traits(t) {}
    void operator()( const double d ) { result.push_back(d); }
    void operator()( const bool d ) { result.push_back(d); }
    void operator()( const tokens::variable* d ) { 
            boost::optional<double> eval = d->get( traits );
            if ( eval.is_initialized() ) result.push_back(*eval); else result.push_back( token(d) );
    }
    void operator()( const boost::shared_ptr<tokens::operand>& d ) { if ( ! d->apply(result) ) result.push_back(d); }

    template <typename ResultType>
    boost::optional<ResultType> evaluate() {
        result.clear();
        for ( std::vector<token>::const_iterator i = token_stack.begin(); i != token_stack.end(); ++i ) {
            boost::apply_visitor( *this, *i );
        }
        if ( result.size() == 1 ) 
            return boost::get<ResultType>(result.back());
        else
            return boost::optional<ResultType>();
    }

    const std::vector<token>& get_simplified_expression() const { return result; }
};
#endif

}
}

#endif
