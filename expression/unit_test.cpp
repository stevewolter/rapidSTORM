#include "dejagnu.h"

namespace dStorm {
namespace expression {

void check_localization_variable( TestState& state );
void check_unit_parser( TestState& state );
void test_parser( TestState& state );

void unit_test( TestState& state ) {
    check_localization_variable(state);
    check_unit_parser(state);
    test_parser(state);
}

}
}
