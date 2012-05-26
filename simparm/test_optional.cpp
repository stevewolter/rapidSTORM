#define SIMPARM_OPTIONAL_ENABLE_CHECK
#include <dejagnu.h>
#include "Entry.h"
#include "Entry.hpp"
#include "Attribute.hpp"
#include <stdexcept>
#include <iostream>
#include <cassert>

template class simparm::Entry< boost::optional<float> >;

void test_opt_node() {
    simparm::Entry< boost::optional<int> > optional_double("OptName", "OptDesc");
    
    TestState().testrun( ! optional_double().is_initialized() , "optional is unset by default" );

    optional_double = 5;
    TestState().testrun( optional_double().is_initialized() , "optional is set after explicit setting" );
    TestState().testrun( *optional_double() == 5 , "optional is set correctly" );
}

int main() {
    test_opt_node();
}
