#ifndef DSTORM_EXPRESSION_SIMPLIFIER_H
#define DSTORM_EXPRESSION_SIMPLIFIER_H

#include "tokens.h"
#include "types.h"
#include <boost/utility/enable_if.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/type_traits/is_base_of.hpp>

namespace dStorm {
namespace expression {

class Simplifier : public boost::static_visitor< tree_node >
{
    const input::Traits<Localization>& traits;
    const variable_table& variables;

    template <typename Result>
    Result apply_me( const tree_node& t ) {
        return boost::get<Result>( boost::apply_visitor(*this, t ) );
    }

    bool is_static( const tree_node& t ) {
        bool rv = boost::get<DynamicQuantity>(&t) || boost::get<bool>(&t);
        return rv;
    }

  public:
    Simplifier( const input::Traits<Localization>& traits, const variable_table& t ) : traits(traits), variables(t) {}

    tree_node operator()( const DynamicQuantity& d ) { return d; }
    tree_node operator()( const bool d ) { return d; }
    tree_node operator()( const variable_table_index i ) { 
        const Variable& d = variables[i];
        if ( d.is_static(traits) ) 
            return d.get(traits); 
        else 
            return i;
    }
    template <typename Operator>
    typename boost::enable_if< boost::is_base_of<tokens::trinary_op, Operator>, tree_node >::type
    operator()( const Operator& o ) { 
        tree_node nodes[3];
        nodes[0] = boost::apply_visitor(*this, o[0]);
        nodes[1] = boost::apply_visitor(*this, o[1]);
        nodes[2] = boost::apply_visitor(*this, o[2]);
        if ( is_static(nodes[0]) && is_static(nodes[1]) && is_static(nodes[2]) ) {
            return o( apply_me<typename Operator::first_argument_type>(nodes[0]),
                    apply_me<typename Operator::second_argument_type>(nodes[1]),
                    apply_me<typename Operator::third_argument_type>(nodes[2]) );
        } else {
            return Operator( nodes[0], nodes[1], nodes[2] );
        }
    }
    template <typename Operator>
    typename boost::enable_if< boost::is_base_of<tokens::binary_op, Operator>, tree_node >::type
    operator()( const Operator& o ) { 
        tree_node nodes[2];
        nodes[0] = boost::apply_visitor(*this, o[0]);
        nodes[1] = boost::apply_visitor(*this, o[1]);
        if ( is_static(nodes[0]) && is_static(nodes[1]) ) {
            tree_node result = o(
                boost::get<typename Operator::first_argument_type>(nodes[0]),
                boost::get<typename Operator::second_argument_type>(nodes[1]) );
            return result;
        } else {
            return Operator( nodes[0], nodes[1] );
        }
    }
    template <typename Operator>
    typename boost::enable_if< boost::is_base_of<tokens::unary_op, Operator>, tree_node >::type
    operator()( const Operator& o ) { 
        tree_node nodes[1];
        nodes[0] = boost::apply_visitor(*this, o[0]);
        if ( is_static(nodes[0]) ) {
            return o( apply_me<typename Operator::argument_type>(nodes[0]) );
        } else {
            return Operator( nodes[0] );
        }
    }
};

}
}

#endif

