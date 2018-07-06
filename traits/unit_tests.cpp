#include <boost/test/unit_test.hpp>

namespace dStorm {
namespace traits {

void test_support_point_projection();

}
}

bool init_unit_tests() {
    boost::unit_test::framework::master_test_suite().
        add(BOOST_TEST_CASE(&dStorm::traits::test_support_point_projection));
    return true;
}

int main(int argc, char* argv[]) {
    return ::boost::unit_test::unit_test_main( &init_unit_tests, argc, argv );
}
