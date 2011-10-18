#ifndef DSTORM_EXPRESSION_PARSER_H
#define DSTORM_EXPRESSION_PARSER_H

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

#include "types.h"
#include "UnitTable.h"

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

#if 0
    qi::rule<Iterator, double()> si_prefix;
    qi::rule<Iterator, ascii::space_type> unit_p, number_with_unit;
    qi::rule<Iterator, ascii::space_type> op, start, factor, term, and_expression, or_expression, comparison,
                                            not_expression, function, boolean, number;
#endif
};

}
}
}

#endif
