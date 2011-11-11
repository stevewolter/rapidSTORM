#include "Source.h"
#include "CommandLine.h"
#include "Config.h"
#include "Source_filters.h"

#include "Simplifier.h"
#include "Evaluator.h"

#include <dStorm/Engine.h>
#include <boost/bind/bind.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/thread/locks.hpp>

namespace dStorm {
namespace expression {

Source::Source( const Config& config, std::auto_ptr<output::Output> downstream ) 
: output::Filter(downstream),
  Object("Expression", "Expression") ,
  command_lines( config.lines ),
  simple_filters( config.simple ),
  variables( config.variables ),
  repeater( NULL )
{
    registerNamedEntries();
}

Source::Source( const Source& o )
: Filter(o), Object(o), 
  command_lines( o.command_lines ),
  simple_filters( o.simple_filters ),
  expressions(),
  variables( o.variables ),
  repeater(o.repeater)
{
    registerNamedEntries();
}

Source::~Source() {
    simple_filters.set_manager( NULL );
    std::for_each( command_lines.begin(), command_lines.end(),
        boost::bind( &config::CommandLine::set_manager, _1, static_cast<config::ExpressionManager*>(NULL) ) );
}

void Source::registerNamedEntries() {
    simple_filters.set_manager( this );
    std::for_each( command_lines.begin(), command_lines.end(),
        boost::bind( &config::CommandLine::set_manager, _1, this ) );

    push_back( Filter::getNode() );
}

Source::AdditionalData Source::announceStormSize(const Announcement& a)
{
    repeater = a.engine;
    my_announcement = a;
    simple_filters.set_visibility(a);
    for ( boost::ptr_vector< source::LValue >::iterator i = expressions.begin(); i != expressions.end(); ++i ) 
        if ( &*i != NULL )
            i->announce(*variables, *my_announcement);
    return Filter::announceStormSize(*my_announcement);
}

Source::Result Source::receiveLocalizations(const EngineResult& er)
{
    EngineResult rv(er);
    EngineResult::iterator end = rv.end();
    for ( boost::ptr_vector< source::LValue >::iterator i = expressions.begin(); i != expressions.end(); ++i )  {
        if ( &*i )
            end = i->evaluate( *variables, *my_announcement, rv.begin(), end );
    }
    rv.erase( end, rv.end() );
    return Filter::receiveLocalizations(rv);
}

void Source::expression_changed( std::string ident, std::auto_ptr<source::LValue> expression )
{
    boost::optional< boost::lock_guard<boost::recursive_mutex> > guard;
    if ( my_announcement.is_initialized() && ! repeater ) return;
    if ( my_announcement.is_initialized() && my_announcement->output_chain_mutex )
        guard = in_place( boost::ref(*my_announcement->output_chain_mutex) );

    std::map< std::string, int >::iterator i = expression_map.find(ident);
    if ( i == expression_map.end() ) {
        i = expression_map.insert( std::make_pair(ident, int(expressions.size())) ).first;
        expressions.push_back( NULL );
    }

    if ( expression.get() && my_announcement.is_initialized() )
        expression->announce( *variables, *my_announcement );
    expressions.replace(i->second, expression);

    if ( repeater ) repeater->repeat_results();
}

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

Filter::iterator Filter::evaluate( const variable_table& tbl, const input::Traits<Localization>&,
                      iterator begin, iterator end ) const
{
    const int count = end - begin;
    EvaluationResult r[count];
    bool good[count];
    ExpressionBasedLValue::evaluate(tbl, begin, end, r);
    for ( int i = 0; i < count; ++i )
        /* No error should happen since our grammar guarantees correct behaviour. */
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

Assignment::Assignment( const variable& v,  const tree_node& expression )
: ExpressionBasedLValue(expression, DynamicQuantity()), v(v.clone()) {}

void Assignment::announce( const variable_table& vt, input::Traits<Localization>& t ) const 
{ 
    ExpressionBasedLValue::simplify(vt, t); 
    tree_node& result = ExpressionBasedLValue::simple;
    if ( DynamicQuantity *rv = boost::get<DynamicQuantity>(&result) ) {
        v->set( t, *rv );
    } else if ( v->is_static(t) )
        throw std::runtime_error("The expression for the static quantity " + v->name
                                    + " contains dynamic variables");
    else
        v->set( t, DynamicQuantity() );
}

Assignment::iterator
Assignment::evaluate( const variable_table& vt, const input::Traits<Localization>& bounds,
                iterator begin, iterator end ) const
{
    EvaluationResult r[end-begin];
    ExpressionBasedLValue::evaluate(vt, begin, end, r);

    iterator end_of_good = end;
    for (iterator i = begin; i < end_of_good; ) {
        bool good = v->set( bounds, *i, boost::get<DynamicQuantity>(r[i-begin]) );
        if ( good )
            ++i;
        else {
            --end_of_good;
            std::swap( *i, *end_of_good );
            std::swap( r[i-begin], r[end_of_good-begin] );
        }
    }
    return end_of_good;
}

}
}
}
