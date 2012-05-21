#include "types.h"
#include "Config.h"
#include "LValue.h"
#include "Parser.h"
#include "CommandLine.h"
#include <dStorm/Localization.h>
#include <dStorm/localization/Traits.h>
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/traits/scalar_iterator.h>
#include <dStorm/traits/tags.h>
#include <boost/mpl/for_each.hpp>
#include <dStorm/output/FilterBuilder.h>
#include <dStorm/output/Capabilities.h>

namespace dStorm {
namespace expression {

Config::Config() 
: simparm::Listener( simparm::Event::ValueChanged ),
  parser( new Parser() ),
  new_line("NewExpression", "Add expression"),
  next_ident(0),
  current_ui( NULL )
{ 
    new_line.userLevel = simparm::Object::Intermediate;
    lines.push_back( new config::CommandLine( "0", parser ) );
    ++next_ident;
}

Config::~Config() {
    simple.set_manager( NULL );
    for ( boost::ptr_vector< config::CommandLine >::iterator i = lines.begin(); i != lines.end(); ++i )
        i->set_manager( NULL );
}

void Config::attach_ui( simparm::Node& at ) {
    current_ui = &at;

    simple.set_manager( this );
    simple.attach_ui( *current_ui );
    for ( boost::ptr_vector< config::CommandLine >::iterator i = lines.begin(); i != lines.end(); ++i )
    {
        i->set_manager( this );
        i->attach_ui( *current_ui );
    }

    new_line.attach_ui( at );
    receive_changes_from( new_line.value );
}

void Config::operator()(const simparm::Event& e)
{
    if ( new_line.triggered() ) {
        std::stringstream ident;
        ident << next_ident++;
        new_line.untrigger();
        lines.push_back( new config::CommandLine(ident.str(), parser) );
        lines.back().set_manager( this );
        if ( current_ui != NULL ) {
            lines.back().attach_ui( *current_ui );
        }
    }
}

bool Config::can_work_with(output::Capabilities)
 { return true; }


std::auto_ptr<output::OutputSource> make_output_source()
{
    return std::auto_ptr<output::OutputSource>( new output::FilterBuilder<Config,Source>() );
}

}
}
