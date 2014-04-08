#ifndef DSTORM_EXPRESSION_UNITTABLE_H
#define DSTORM_EXPRESSION_UNITTABLE_H

#include "expression/SIPrefixes.h"
#include "expression/tokens.h"

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

namespace dStorm {
namespace expression {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

struct UnitTable : public qi::symbols<char, int>
{
   UnitTable();
};

template <typename Iterator>
struct boost_units_unit_parser : qi::grammar<Iterator, double(), ascii::space_type>
{
    boost_units_unit_parser();

    const UnitTable table;
    DynamicUnit result;
    const SIPrefixTable prefixes;

    qi::rule<Iterator, double(), ascii::space_type> si_prefix, no_prefix;
    qi::rule<Iterator, int(), ascii::space_type> unitname;
    qi::rule<Iterator, double(), ascii::space_type> exponent;
    qi::rule<Iterator, double(), ascii::space_type> unitspecs, unitspec;
};

}
}

#endif
