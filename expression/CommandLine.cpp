#include "CommandLine.h"
#include "Filter.h"
#include "VariableLValue.h"

namespace dStorm {
namespace expression {
namespace config {

CommandLine::CommandLine( std::string ident, boost::shared_ptr<Parser> parser )
:   disambiguator("CommandLine" + ident, "Command line") ,
    lvalue("LValue", "Value to assign to"),
    expression("Expression", "Expression to assign from"),
    parser(parser),
    manager(NULL)
{
    lvalue.set_user_level( simparm::Intermediate );
    expression.set_user_level( simparm::Intermediate );

    expression.setHelpID( "#ExpressionFilter_Expression" );
    lvalue.setHelpID( "#ExpressionFilter_LValue" );

    lvalue.addChoice( make_filter() );
    const VariableTable& variables = parser->get_variable_table();
    for ( variable_table::const_iterator i = variables.begin(); i != variables.end(); ++i )
        lvalue.addChoice( make_variable_lvalue(*i) );

}

CommandLine::~CommandLine() {
    manager = NULL;
}

void CommandLine::set_expression_string()
{
    if ( expression() != "" )
        lvalue().set_expression_string( expression(), *parser );
    publish();
}

void CommandLine::attach_ui( simparm::NodeHandle at )
{
    listening[0] = lvalue.value.notify_on_value_change( 
        boost::bind( &CommandLine::set_expression_string, this ) );
    listening[1] = expression.value.notify_on_value_change( 
        boost::bind( &CommandLine::set_expression_string, this ) );
    simparm::NodeHandle r = disambiguator.attach_ui( at );
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

