#include "DisjointData.hpp"
#include "dejagnu.h"
#include <boost/units/systems/si/length.hpp>
#include <boost/units/io.hpp>

namespace nonlinfit {
namespace plane {

using namespace boost::units;

void run_unit_tests(TestState& state) {

    typedef DisjointData< int, si::length, 7 > Data;
    Data data;

    for (int i = 0; i < 28; ++i)
        data.push_back( Data::value_type( (i%7) * si::meter, - (i/7) * 5 * si::meter, i * 500, i * 10, 0 ) );
    data.pad_last_chunk();

    int i = 0;
    bool all_good = true;
    for ( Data::const_iterator j = data.begin(); j != data.end(); ++j ) {
        all_good = all_good && j->position(0) == (i%7) * si::meter && j->position(1) == - (i/7) * 5 * si::meter 
                            && j->value() == i * 500;
        ++i;
    }

    state( all_good, "Disjoint data serializes and deserializes correctly" );
}

}
}
