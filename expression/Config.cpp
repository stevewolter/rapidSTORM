#include "types.h"
#include "Config.h"
#include "LValue.h"
#include "Parser.h"
#include "CommandLine.h"
#include "localization_variable_decl.h"
#include <dStorm/Localization.h>
#include <dStorm/localization/Traits.h>
#include <simparm/Entry.hh>
#include <dStorm/traits/scalar_iterator.h>
#include <dStorm/traits/tags.h>
#include <boost/mpl/for_each.hpp>
#include <dStorm/output/FilterBuilder.h>
#include <dStorm/output/Capabilities.h>

namespace dStorm {
namespace expression {

Config::Config() 
: simparm::Object("Expression", "Expression filter"),
  simparm::Listener( simparm::Event::ValueChanged ),
  variables( variables_for_localization_fields() ),
  simple( variables ),
  new_line("NewExpression", "Add expression"),
  next_ident(0)
{ 
    new_line.userLevel = simparm::Object::Intermediate;
    registerNamedEntries(); 
    new_line.trigger();
}

Config::Config(const Config& o) 
: simparm::Object(o),
  simparm::Listener( simparm::Event::ValueChanged ),
  variables(o.variables), simple(o.simple),
  lines(o.lines), new_line(o.new_line), next_ident(o.next_ident)
{ 
    registerNamedEntries(); 
}

Config::~Config() {
    simple.set_manager( NULL );
    for ( boost::ptr_vector< config::CommandLine >::iterator i = lines.begin(); i != lines.end(); ++i )
        i->set_manager( NULL );
}

void Config::registerNamedEntries() {
    simple.set_manager( this );
    for ( boost::ptr_vector< config::CommandLine >::iterator i = lines.begin(); i != lines.end(); ++i )
        i->set_manager( this );
    push_back( new_line );
    receive_changes_from( new_line.value );
}

void Config::operator()(const simparm::Event& e)
{
    if ( new_line.triggered() ) {
        std::stringstream ident;
        ident << next_ident++;
        new_line.untrigger();
        lines.push_back( new config::CommandLine(ident.str(), variables) );
        push_back( lines.back() );
    }
}

bool Config::can_work_with(output::Capabilities)
 { return true; }


}

namespace output {

template <>
std::auto_ptr<OutputSource> make_output_source<expression::Source>()
{
    return std::auto_ptr<OutputSource>( new FilterBuilder<expression::Source>() );
}

}

}
