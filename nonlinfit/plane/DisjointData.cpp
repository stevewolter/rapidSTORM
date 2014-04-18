#include "nonlinfit/plane/DisjointData.hpp"
#include <boost/test/unit_test.hpp>

namespace nonlinfit {
namespace plane {

static void check_serialization() {

    typedef DisjointData< int, 7 > Data;
    Data data;

    for (int i = 0; i < 28; ++i)
        data.push_back( Data::value_type( (i%7), - (i/7) * 5, i * 500, i * 10, 0 ) );
    data.pad_last_chunk();

    int i = 0;
    for ( Data::const_iterator j = data.begin(); j != data.end(); ++j ) {
        BOOST_CHECK_EQUAL( j->position(0), i%7 );
        BOOST_CHECK_EQUAL( j->position(1), - (i/7) * 5 );
        BOOST_CHECK_EQUAL( j->value(), i * 500 );
        ++i;
    }

}

boost::unit_test::test_suite* register_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "plane" );
    rv->add( BOOST_TEST_CASE( &check_serialization ) );
    return rv;
}

}
}
