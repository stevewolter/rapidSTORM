#include "expression/types.h"
#include "expression/Config.h"
#include "expression/LValue.h"
#include "expression/Parser.h"
#include "CommandLine.h"
#include "Localization.h"
#include "localization/Traits.h"
#include "simparm/Entry.h"
#include <boost/mpl/for_each.hpp>
#include "output/FilterBuilder.h"
#include "output/Capabilities.h"
#include <boost/lexical_cast.hpp>

namespace dStorm {
namespace expression {

Config::Config() 
: parser( new Parser() ),
  line_count("ExpressionCount", "Number of expressions", 1)
{ 
    line_count.set_user_level( simparm::Intermediate );

    lines.push_back( new config::CommandLine( "0", parser ) );
}

Config::~Config() {
    simple.set_manager( NULL );
    for ( boost::ptr_vector< config::CommandLine >::iterator i = lines.begin(); i != lines.end(); ++i )
        i->set_manager( NULL );
}

void Config::attach_ui( simparm::NodeHandle at ) {
    listening = line_count.value.notify_on_value_change( 
        boost::bind( &Config::commit_line_count, this ) );

    current_ui = at;

    simple.set_manager( this );
    simple.attach_ui( current_ui );

    line_count.attach_ui( at );
    for ( boost::ptr_vector< config::CommandLine >::iterator i = lines.begin(); i != lines.end(); ++i )
    {
        i->set_manager( this );
        i->attach_ui( current_ui );
    }

}

void Config::commit_line_count()
{
    size_t c = line_count();
    while ( c > lines.size() ) {
        std::string ident = boost::lexical_cast<std::string>( lines.size() );
        lines.push_back( new config::CommandLine(ident, parser) );
        if ( current_ui ) {
            lines.back().set_manager( this );
            lines.back().attach_ui( current_ui );
        }
    }
    while ( c < lines.size() )
        lines.pop_back();
}

bool Config::can_work_with(output::Capabilities)
 { return true; }


std::auto_ptr<output::OutputSource> make_output_source()
{
    return std::auto_ptr<output::OutputSource>( new output::FilterBuilder<Config,Source>() );
}

}
}
