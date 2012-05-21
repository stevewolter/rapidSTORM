#include "CommandLine.h"
#include "Filter.h"
#include "VariableLValue.h"
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace expression {
namespace config {

CommandLine::CommandLine( std::string ident, boost::shared_ptr<Parser> parser )
:   simparm::Listener( simparm::Event::ValueChanged ),
    disambiguator("CommandLine" + ident, "Command line") ,
    lvalue("LValue", "Value to assign to"),
    expression("Expression", "Expression to assign from"),
    parser(parser),
    manager(NULL)
{
    lvalue.userLevel = simparm::Object::Intermediate;
    expression.userLevel = simparm::Object::Intermediate;

    expression.helpID = "#ExpressionFilter_Expression";
    lvalue.helpID = "#ExpressionFilter_LValue";

    lvalue.addChoice( make_filter() );
    const VariableTable& variables = parser->get_variable_table();
    for ( variable_table::const_iterator i = variables.begin(); i != variables.end(); ++i )
        lvalue.addChoice( make_variable_lvalue(*i) );

}

CommandLine::~CommandLine() {
    stop_receiving_changes_from( lvalue.value );
    manager = NULL;
}

void CommandLine::operator()( const simparm::Event& )
{
    if ( expression() != "" )
        lvalue().set_expression_string( expression(), *parser );
    publish();
}

void CommandLine::attach_ui( simparm::Node& at )
{
    receive_changes_from( lvalue.value );
    receive_changes_from( expression.value );
    simparm::NodeRef r = disambiguator.attach_ui( at );
    lvalue.attach_ui( r );
    expression.attach_ui( r );
}

void CommandLine::publish() {
    if ( manager ) {
        std::auto_ptr<source::LValue> e;
        if ( expression() != "" )
            e.reset( lvalue().make_lvalue() );
        manager->expression_changed( disambiguator.getName(), e );
    }
}
void CommandLine::set_manager( ExpressionManager* manager ) {
    this->manager = manager;
    publish();
}

}
}
}

