#include "expression/Source.h"
#include "CommandLine.h"
#include "expression/Config.h"
#include "expression/LValue.h"

#include "expression/Simplifier.h"
#include "expression/Evaluator.h"

#include "base/Engine.h"
#include <boost/bind/bind.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/thread/locks.hpp>

namespace dStorm {
namespace expression {

Source::Source( const Config& config, std::auto_ptr<output::Output> downstream ) 
: output::Filter(downstream),
  command_lines( config.lines ),
  simple_filters( config.simple ),
  parser( config.parser ),
  repeater( NULL )
{
}

Source::~Source() {
    simple_filters.set_manager( NULL );
    std::for_each( command_lines.begin(), command_lines.end(),
        boost::bind( &config::CommandLine::set_manager, _1, static_cast<config::ExpressionManager*>(NULL) ) );
}

void Source::attach_ui_( simparm::NodeHandle at ) {
    simple_filters.set_manager( this );
    std::for_each( command_lines.begin(), command_lines.end(),
        boost::bind( &config::CommandLine::set_manager, _1, this ) );

    simple_filters.attach_ui( at );
    std::for_each( command_lines.begin(), command_lines.end(),
        boost::bind( &config::CommandLine::attach_ui, _1, boost::ref(at) ) );

    Filter::attach_children_ui( at );
}

Source::AdditionalData Source::announceStormSize(const Announcement& a)
{
    {
        boost::lock_guard<boost::mutex> guard(mutex);
        repeater = a.engine;
        my_announcement = a;
        simple_filters.set_visibility(a);
        for ( boost::ptr_vector< source::LValue >::iterator i = expressions.begin(); i != expressions.end(); ++i ) 
            if ( &*i != NULL )
                i->announce(parser->get_variable_table(), *my_announcement);
    }
    return Filter::announceStormSize(*my_announcement);
}

void Source::receiveLocalizations(const EngineResult& er)
{
    EngineResult rv(er);
    EngineResult::iterator end = rv.end();
    {
        boost::lock_guard<boost::mutex> guard(mutex);
        for ( boost::ptr_vector< source::LValue >::iterator i = expressions.begin(); i != expressions.end(); ++i )  {
            if ( &*i ) {
                end = i->evaluate( parser->get_variable_table(), *my_announcement, rv.begin(), end );
            }
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
        expression->announce( parser->get_variable_table(), *my_announcement );
    expressions.replace(i->second, expression);

    if ( repeater && repeater->can_repeat_results() ) repeater->repeat_results();
}

}
}
