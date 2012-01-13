#include "AbstractMap.hpp"
#include <boost/mpl/vector.hpp>
#include <boost/units/systems/si/length.hpp>
#include <cassert>
#include <Eigen/Core>
#include "Evaluator.hpp"
#include <nonlinfit/functions/Constant.h>
#include <nonlinfit/functions/Polynom.h>
#include <nonlinfit/functions/Zero.h>
#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/sum/Lambda.h>
#include "dejagnu.h"

namespace nonlinfit {
namespace sum {

struct test_functor {
    typedef std::pair<int,int> result_type;
    result_type operator()( int function, int parameter ) const {
        return result_type( function - function % 2, parameter % 2 );
    }
};

void test_AbstractMap(TestState & state)
{
    std::bitset<5> bits;
    bits[0] = 1;
    bits[1] = 1;
    bits[4] = 1;

    AbstractMap<5> map(10,bits);
    
    state( map.function_count() == 10 );
    state( map.output_variable_count() == 23 );
    state( map(0,0) == 0 );
    state( map(9,0) == 0 );
    state( map(0,2) == 2 );
    state( map(0,2) != map(7,2) );
    state( map(0,3) != map(7,3) );
    state( map(0,4) == map(7,4) );

    AbstractMap<5> map2;
    for (int i = 0; i < 10; ++i) map2.add_function( test_functor() );
    state( map2.function_count() == 10 );
    state( map2.output_variable_count() == 10 );
    state( map2(9,4) == map2(8,0) );
    state( map2(9,4) != map2(8,1) );
};

void test_Evaluator( TestState& state)
{
    typedef sum::Lambda< boost::mpl::vector<constant::Expression, constant::Expression, constant::Expression > > 
        MyExpression;
    typedef get_evaluator< MyExpression, 
        plane::xs_joint<double,boost::units::si::length,1>::type >::type MyEvaluator;
    MyExpression m;
    m.get_part( boost::mpl::int_<0>() )( constant::Amount() ) = 5.0;
    m.get_part( boost::mpl::int_<1>() )( constant::Amount() ) = 4.0;
    m.get_part( boost::mpl::int_<2>() )( constant::Amount() ) = 8.0;
    MyEvaluator eval(m);

    bool can_evaluate = eval.prepare_iteration( 42 );
    state(( can_evaluate ));

    int dummy;
    eval.prepare_chunk( dummy );

    Eigen::Matrix< double, 1, 1 > derivatives;
    eval.derivative( derivatives.col(0), TermParameter< boost::mpl::int_<0>, constant::Amount >() );
    state(( derivatives(0,0) == 1.0 ));
    eval.derivative( derivatives.col(0), TermParameter< boost::mpl::int_<2>, constant::Amount >() );
    state(( derivatives(0,0) == 1.0 ));

    Eigen::Matrix< double, 1, 1 > value;
    eval.value( value );
    state(( value(0,0) == 17.0 ));
}

void test_Expression(TestState& state) {
    typedef sum::Lambda< boost::mpl::vector<constant::Expression, zero::Expression, static_power::Expression > > 
        MyExpression;
    typedef sum::Lambda< boost::mpl::vector<constant::Expression, zero::Expression, static_power::Expression, constant::Expression, static_power::Expression, zero::Expression, zero::Expression > > 
        MyOtherExpression;
    BOOST_STATIC_ASSERT(( boost::mpl::size< MyExpression::Variables >::type::value == 4 ));
    BOOST_STATIC_ASSERT(( boost::mpl::size< MyOtherExpression::Variables >::type::value == 8 ));
    MyExpression m;
    m.get_part( boost::mpl::int_<0>() )( constant::Amount() ) = 5.0;
    m.get_part( boost::mpl::int_<2>() )( static_power::Prefactor() ) = 1.0;
    const MyExpression& m2 = m;
    state(( m2.get_part( boost::mpl::int_<0>() )( constant::Amount() ).value() == 5.0 ));
    state(( m2.get_part( boost::mpl::int_<2>() )( static_power::Prefactor() ).value() == 1.0 ));

    MyOtherExpression mom;
    mom.get_part( boost::mpl::int_<0>() )( constant::Amount() ) = 5.0;
    mom.get_part( boost::mpl::int_<3>() )( constant::Amount() ) = 15.0;
    const MyOtherExpression& mom2 = mom;
    state(( mom2.get_part( boost::mpl::int_<0>() )( constant::Amount() ).value() == 5.0 ));
    state(( mom2.get_part( boost::mpl::int_<3>() )( constant::Amount() ).value() == 15.0 ));
}

void run_unit_tests(TestState& state) {
    test_AbstractMap(state);
    test_Evaluator(state);
    test_Expression(state);
}

}
}
