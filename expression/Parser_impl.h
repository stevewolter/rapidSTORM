#include "Parser.h"
#include "tokens.h"

namespace dStorm {
namespace expression {
namespace parser {

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

#if 0
    si_prefix = prefixes[_val = _1] | qi::eps[_val = 1.0];
    unit_p = (qi::lexeme[si_prefix >> units] >> exponent)[push_unit(ops, types)];
    number_with_unit = (qi::double_[push_value(ops, types)]) >> *unit_p;
    op = number_with_unit | ( "(" >> number >> ")" ) | symbols[push_variable(ops, types)];
    function = op |
        (qi::lit("exp") >> op )[inst_op(exp_token())];
    factor = function >> *( 
        ("*" >> function)[inst_op(times_token())] |
        ("/" >> function)[inst_op(divide_token())] );
    term = factor >> *( 
        ("+" >> factor)[inst_op(plus_token())] |
        ("-" >> factor)[inst_op(minus_token())] );
    number = term | ("(" >> boolean >> ")" >> "?" >> number >> ":" >> number)[inst_op(choice_token())];
    comparison = ( "(" >> boolean >> ")" ) |
        number >> ( ( "==" >> number)[inst_op(equal_token())] |
                    ( ">=" >> number)[inst_op(ge_token())] |
                    ( "<=" >> number)[inst_op(le_token())] |
                    ( ">" >> number)[inst_op(greater_token())] |
                    ( "<" >> number)[inst_op(less_token())] |
                    ( "!=" >> number)[inst_op(unequal_token())] |
                    ( "=" >> number)[inst_op(equal_token())] );
    not_expression = comparison | ("!" >> comparison)[inst_op(not_token())];
    and_expression = not_expression >> *( 
        ("&&" >> not_expression)[inst_op(and_token())] );
    or_expression = and_expression >> *( 
        ("||" >> comparison)[inst_op(or_token())] );
    boolean = or_expression;
    start = qi::eps[clear(boost::phoenix::ref(ops))] >> qi::eps[clear(boost::phoenix::ref(types))] >>
            boolean;
#endif
}

}
}
}
