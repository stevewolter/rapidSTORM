class TestState;

namespace boost { namespace unit_test { class test_suite; } }

namespace dStorm {
namespace guf {

void run_unit_tests( TestState& );
boost::unit_test::test_suite* test_unit_tests();

}
}
