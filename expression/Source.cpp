#include "Source.h"
#include "CommandLine.h"
#include "Config.h"
#include "LValue.h"

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
    boost::lock_guard<boost::mutex> guard(mutex);
    repeater = a.engine;
    my_announcement = a;
    simple_filters.set_visibility(a);
    for ( boost::ptr_vector< source::LValue >::iterator i = expressions.begin(); i != expressions.end(); ++i ) 
        if ( &*i != NULL )
            i->announce(*variables, *my_announcement);
    return Filter::announceStormSize(*my_announcement);
}

void Source::receiveLocalizations(const EngineResult& er)
{
    boost::lock_guard<boost::mutex> guard(mutex);
    EngineResult rv(er);
    EngineResult::iterator end = rv.end();
    for ( boost::ptr_vector< source::LValue >::iterator i = expressions.begin(); i != expressions.end(); ++i )  {
        if ( &*i ) {
            end = i->evaluate( *variables, *my_announcement, rv.begin(), end );
        }
    }
    rv.erase( end, rv.end() );
    Filter::receiveLocalizations(rv);
}

void Source::expression_changed( std::string ident, std::auto_ptr<source::LValue> expression )
{
    boost::lock_guard<boost::mutex> guard(mutex);
    if ( my_announcement.is_initialized() && ! ( repeater && repeater->can_repeat_results() ) ) return;

    std::map< std::string, int >::iterator i = expression_map.find(ident);
    if ( i == expression_map.end() ) {
        i = expression_map.insert( std::make_pair(ident, int(expressions.size())) ).first;
        expressions.push_back( NULL );
    }

    if ( expression.get() && my_announcement.is_initialized() )
        expression->announce( *variables, *my_announcement );
    expressions.replace(i->second, expression);

    if ( repeater && repeater->can_repeat_results() ) repeater->repeat_results();
}

}
}
