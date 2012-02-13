class TestState;

void reconstruction_by_dilation_test( TestState& state );

namespace locprec {
void run_unit_tests( TestState& state ) {
    reconstruction_by_dilation_test(state);
}
}
