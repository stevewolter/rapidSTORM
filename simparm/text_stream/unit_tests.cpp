#include "../Object.h"
#include "RootNode.h"
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

namespace simparm {
namespace text_stream {

void run_commands( boost::shared_ptr<RootNode> r )
{
    while (true) {
        Object a1("Level1", "");
        Object a2("Level2", "");
        Object a3("Level3", "");
        Object a4("Level4", "");

        a4.attach_ui( a3.attach_ui( a2.attach_ui( a1.attach_ui( r ) ) ) );
    }
}

void check_concurrency() {
    boost::shared_ptr<RootNode> r( new RootNode() );
    boost::thread thread = boost::thread( boost::bind( &run_commands, r ) );

    for (int i = 0; i < 100; ++i) {
        std::stringstream s("in Level1 in Level2 in Level3 in Level4 in viewable query");
        try {
            r->processCommand( s );
        } catch(const std::runtime_error&) {}
    }

    thread.interrupt();
}

boost::unit_test::test_suite* make_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "gaussian_psf" );
    rv->add( BOOST_TEST_CASE( &check_concurrency ) );
    return rv;
}

}
}
