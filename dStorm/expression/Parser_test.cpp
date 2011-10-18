#include "Parser.h"
#include "localization_variable_decl.h"
#include "Evaluator.h"
#include "Simplifier.h"
#include "UnitChecker.h"
#include <dStorm/localization/Traits.h>
#include <dStorm/Localization.h>

using namespace dStorm::expression;

struct test {
    std::string input;
    bool is_boolean;
    bool expect_parse_fail;
};

test inputs[] = {
    { "m", false, false },
    { "m^2", false, false },
    { "m^-1", false, false },
    { "nm^2", false, false },
    { "nm^2 Mm", false, false },
    { "nm^2 Mm^-2", false, false },
    { "nm^2 / Mm^2", false, false },
    { "nm + pm", false, false },
    { "pm <= nm", true, false },
    { "1 nm <= posx", true, false },
    { "5 nm + 20 nm <= posx + 5 Mm && 10 nm > posx", true, false },
    { "(20 nm)^(posx / nm)", false, false },
    { "posx < 5 m", true, false },
    { "posx", false, false },
    { "posx*2", false, false },
    { "(posx < 5 m)", true, false },
    { "(posx < 5 m) ? posx : posx*2", false, false },
    { "10 nm > (posx < 5 m) ? posx : posx*2", true, false },
    { "5 nm + 20 nm <= posx + 5 Mm && 10 nm > (posx < 5 m) ? posx : posx*2", true, false },
};

int main() {
    parser::expression_parser< std::string::const_iterator > parser;
    std::auto_ptr< variable_table > variables = variables_for_localization_fields();
    for (boost::ptr_vector<variable>::iterator i = variables->begin(); i != variables->end(); ++i) {
        parser.symbols.add( i->name.c_str(), i - variables->begin() );
    }
    std::string input;
    std::string::const_iterator begin, end;
    bool r;
    tree_node result;

    input = "amp";
    begin = input.begin(), end = input.end();
    r = phrase_parse( begin, end, parser.symbols, boost::spirit::ascii::space, result );
    assert( "amp parsed with parser.symbols " && r && begin == end );
    begin = input.begin(), end = input.end();
    r = phrase_parse( begin, end, parser.power, boost::spirit::ascii::space, result );
    assert( "amp parsed with parser.power" && r && begin == end );
    begin = input.begin(), end = input.end();
    r = phrase_parse( begin, end, parser.numeric, boost::spirit::ascii::space, result );
    assert( "amp parsed with parser.numeric" && r && begin == end );

    for ( unsigned int i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i ) {
        input = inputs[i].input;
        begin = input.begin(), end = input.end();

        if ( inputs[i].is_boolean )
            r = phrase_parse( begin, end, parser, boost::spirit::ascii::space, result );
        else
            r = phrase_parse( begin, end, parser.numeric, boost::spirit::ascii::space, result );

        if ( r && begin == end ) {
            assert ( ! inputs[i].expect_parse_fail );
            //std::cerr << "Result stack: " << result << std::endl;
            dStorm::input::Traits<dStorm::Localization> traits;
            traits.position().is_given(0,0) = true;
            Simplifier s( traits, *variables );
            tree_node simplified = boost::apply_visitor(s, result);
            //std::cerr << "Simplified stack: " << simplified << std::endl;

            UnitChecker c( traits, *variables );
            UnitChecker::result r = boost::apply_visitor(c, simplified);
            //std::cerr << "Units checked as " << r << std::endl;

            Evaluator v(*variables);
            dStorm::Localization loc;
            loc.position().x() = 7E-9 * boost::units::si::meter;
            v.set_localization( &loc );

            //std::cerr << "Evaluation result: " << boost::apply_visitor(v, simplified) << std::endl;
        } else {
            if ( inputs[i].expect_parse_fail ) continue;
            if ( ! r ) std::cerr << "Unsuccessfully parsed: " << input << std::endl;
            if ( begin != end )
                std::cerr << "Did not parse to end of string. Last character is '" << *begin << "' at index " << begin - input.begin() << std::endl;
            assert( /* Parsing should not have failed */ false );
        }
    }
}
