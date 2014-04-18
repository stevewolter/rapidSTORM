#define SIMPARM_OPTIONAL_ENABLE_CHECK
#include <boost/test/unit_test.hpp>
#include "simparm/Entry.h"
#include "simparm/Entry.hpp"
#include "simparm/Attribute.hpp"
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/io.hpp>

namespace simparm {

template class Entry< boost::optional<float> >;

void test_opt_node() {
    simparm::Entry< boost::optional<int> > optional_double("OptName", "OptDesc", boost::optional<int>() );
    
    BOOST_CHECK( ! optional_double().is_initialized() );

    optional_double = 5;
    BOOST_CHECK( optional_double().is_initialized() );
    BOOST_CHECK_EQUAL( *optional_double(), 5 );
}

void test_units() {
    simparm::Entry< boost::units::quantity<boost::units::si::length,float> >
      meters("Name", "Desc", 0.0f * boost::units::si::meter);

    boost::units::quantity<boost::units::si::length,float> some_meters
      ( 1.0f * boost::units::si::metre );

    meters = some_meters;
    BOOST_CHECK_EQUAL( meters(), some_meters );
    meters = 5 * boost::units::si::metre;
    BOOST_CHECK_EQUAL( meters(), 5 * boost::units::si::metre );
}

boost::unit_test::test_suite* test_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "simparm" );
    rv->add( BOOST_TEST_CASE(&test_opt_node) );
    rv->add( BOOST_TEST_CASE(&test_units) );
    return rv;
}

}
