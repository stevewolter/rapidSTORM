#include "Parser.h"
#include <string>
#include <boost/fusion/include/vector.hpp>
#include "localization_variable_decl.h"
#include <dStorm/localization/Traits.h>
#include "UnitChecker.h"
#include "Evaluator.h"
#include "Simplifier.h"
#include <dStorm/Localization.h>
#include "dejagnu.h"
#include "tokens.h"
#include "UnitTable.h"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <complex>
#include <map>
#include <cassert>

namespace dStorm {
namespace expression {
namespace parser {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace fsn = boost::fusion;

template <typename Iterator>
struct expression_parser : qi::grammar<Iterator, tree_node(), ascii::space_type>
{
   expression_parser();

    UnitTable units;
    const SIPrefixTable prefixes;
    qi::symbols<char, variable_table_index> symbols;
    qi::rule<Iterator, double(), ascii::space_type> exponent;
    qi::rule<Iterator, tree_node(), ascii::space_type> unit, number, atom, atoms,
        power, product, sum, choice, boolean, comparison, negated, anded, ored, numeric;

};

namespace fsn = boost::fusion;

struct new_unit {
    template <typename Arg, typename Arg2, typename Arg3>
    struct result { typedef tree_node type; };

    template <typename Arg, typename Arg2, typename Arg3>
    tree_node operator()(Arg v, Arg2 u, Arg3 e) const
    {
        DynamicUnit rv( DynamicUnit::Zero() );
        rv[u] = e;
        return DynamicQuantity( v, rv );
    }

};

template <typename Operator>
struct trinary_op_from_vector {
    template <typename Left>
    struct result { typedef tree_node type; };

    template <typename Left>
    tree_node operator()(Left lhs) const { 
        return Operator( fsn::at_c<0>(lhs), fsn::at_c<1>(lhs), fsn::at_c<2>(lhs) ); 
    }
};

template <typename Operator>
struct make_trinary_op {
    template <typename Left, typename Right, typename Rightest>
    struct result { typedef tree_node type; };

    template <typename Left, typename Right, typename Rightest>
    tree_node operator()(Left lhs, Right rhs, Rightest rrhs) const { 
        return Operator( lhs, rhs, rrhs ); 
    }
};

template <typename Operator>
struct make_binary_op {
    template <typename Left, typename Right>
    struct result { typedef tree_node type; };

    template <typename Left, typename Right>
    tree_node operator()(Left lhs, Right rhs) const
        { return Operator( lhs, rhs ); }
};

template <typename Operator>
struct make_unary_op {
    template <typename Left>
    struct result { typedef tree_node type; };

    template <typename Left>
    tree_node operator()(Left lhs) const
        { return tree_node( Operator( lhs ) ); }

};

template <typename Iterator>
expression_parser<Iterator>::expression_parser() : expression_parser::base_type(ored)
{
    namespace phx = boost::phoenix;
    using qi::labels::_val;
    using qi::labels::_1;
    using qi::labels::_2;
    using qi::labels::_3;

#define UNARY(x) _val = phx::function< make_unary_op<tokens::x> >()(_val)
#define BINARY(x) _val = phx::function< make_binary_op<tokens::x> >()(_val, _1)
#define TRINARY(x) _val = phx::function< make_trinary_op<tokens::x> >()(_val, _1, _2)

    unit = qi::lexeme[(prefixes >> units)[_val = phx::function<new_unit>()(_1, _2, 1) ] ]
         | (units)[_val = phx::function<new_unit>()(1.0, _1, 1) ];
    number = qi::double_[_val = phx::function<new_unit>()(_1, 0, 0)];
    atom = (unit | number | ("(" >> numeric >> ")") )[_val = _1]
                >> -( ("^" >> atom)[BINARY(power)] );
    power = symbols[_val = _1] | ( atom[_val = _1] >> *( atom[BINARY(multiplies)] ) );
    product = power[_val = _1] >> *( 
        ("*" >> power)[BINARY(multiplies)] |
        ("/" >> power)[BINARY(divides)] );
    sum = product[_val = _1] >> *( 
        ("+" >> product)[BINARY(plus)] |
        ("-" >> product)[BINARY(minus)] );
    choice = sum[_val = _1] 
        | ( "(" >> boolean >> ")" >> "?" >> sum >> ":" >> sum )
                [_val = phx::function< make_trinary_op<tokens::choice> >()(_1, _2, _3) ];
    numeric %= choice;
    comparison = 
        (numeric[_val = _1] >> ( ( "==" >> numeric)[BINARY(equal_to)] |
                    ( ">=" >> numeric)[BINARY(greater_equal)] |
                    ( "<=" >> numeric)[BINARY(less_equal)] |
                    ( ">" >> numeric)[BINARY(greater)] |
                    ( "<" >> numeric)[BINARY(less)] |
                    ( "!=" >> numeric)[BINARY(not_equal_to)] |
                    ( "=" >> numeric)[BINARY(equal_to)] ))
        | ("(" >> boolean >> ")")[_val = _1];
    negated = comparison[_val = _1] | ("!" >> comparison[UNARY(logical_not)]);
    anded = negated[_val = _1] >> *( "&&" >> negated[BINARY(logical_and)]);
    ored = anded[_val = _1] >> *( "||" >> anded[BINARY(logical_or)]);
    boolean %= ored;


}

}

struct Parser::Pimpl {
    parser::expression_parser< std::string::const_iterator > parser;
    VariableTable variables;

    Pimpl()
    { 
        static_cast< variable_table& >(variables) = variables_for_localization_fields();
        for (boost::ptr_vector<Variable>::iterator i = variables.begin(); i != variables.end(); ++i) {
            parser.symbols.add( i->name.c_str(), i - variables.begin() );
        }
    }
};

Parser::Parser()
: pimpl( new Pimpl() )
{
}

void Parser::add_variable( std::auto_ptr<Variable> v )
{
    pimpl->parser.symbols.add( v->name.c_str(), pimpl->variables.size() );
    pimpl->variables.push_back( v );
}

VariableTable& Parser::get_variable_table() { return pimpl->variables; }
const VariableTable& Parser::get_variable_table() const { return pimpl->variables; }

boost::shared_ptr<AbstractSyntaxTree> Parser::parse_boolean( const std::string& what ) const
{
    boost::shared_ptr<AbstractSyntaxTree> ast( new AbstractSyntaxTree() );
    std::string::const_iterator start = what.begin(), end = what.end();
    bool r = phrase_parse( start, end, pimpl->parser, boost::spirit::ascii::space, ast->root_node );
    if ( ! r || start != end ) {
        throw std::runtime_error("Unable to parse boolean expression '" + what + "'");
    }
    return ast;
}

boost::shared_ptr<AbstractSyntaxTree> Parser::parse_numeric( const std::string& what ) const
{
    boost::shared_ptr<AbstractSyntaxTree> ast( new AbstractSyntaxTree() );
    std::string::const_iterator start = what.begin(), end = what.end();
    bool r = phrase_parse( start, end, pimpl->parser.numeric, boost::spirit::ascii::space, ast->root_node );
    if ( r && start == end ) {
        return ast;
    } else
        throw std::runtime_error("Unable to parse numeric expression '" + what + "'");
}

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

void test_parser( TestState& state ) {
    Parser parser;
    dStorm::input::Traits<dStorm::Localization> traits;
    traits.position_x().is_given = true;
    Simplifier s( traits, parser.get_variable_table() );
    UnitChecker c( traits, parser.get_variable_table() );
    Evaluator v( parser.get_variable_table() );

    for ( unsigned int i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i ) {
        boost::shared_ptr<AbstractSyntaxTree> ast;
        try {
            if ( inputs[i].is_boolean )
                ast = parser.parse_boolean( inputs[i].input );
            else
                ast = parser.parse_numeric( inputs[i].input );
        } catch ( const std::runtime_error& ) {
            ast.reset();
        }

        if ( inputs[i].expect_parse_fail ) {
            state( ! ast.get(), "Fail parsing " + inputs[i].input );
        } else {
            state( ast.get(), "Succeeded parsing " + inputs[i].input );
        }
    }
}

}
}
