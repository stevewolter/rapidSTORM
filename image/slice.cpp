#include <boost/test/unit_test.hpp>
#include "image/slice.h"
#include "image/constructors.h"
#include "image/iterator.h"

using namespace boost::units;

namespace dStorm {
namespace image {

void slice_unit_test() 
{
    Image<int,4> a( Image<int,4>::Size::Constant(25*camera::pixel) );
    for ( Image<int,4>::iterator i = a.begin(); i != a.end(); ++i ) {
        *i = 0;
        for (int j = 0; j < 4; ++j) *i = *i * 100 + i.position()[j] / camera::pixel;
    }

    Image<int,2> b( a.slice(2,13*camera::pixel).slice(1,24*camera::pixel) );
    int count = 0;
    bool pixels_are_the_same = true;
    for ( Image<int,2>::iterator i = b.begin(); i != b.end(); ++i ) {
        int s = 0;
        for (int j = 0; j < 4; ++j) 
            s = s * 100 + ( (j == 1) ? 24 : (j == 2) ? 13 : (j == 3) ? int(i.position()[1] / camera::pixel) : int(i.position()[0] / camera::pixel) );
        pixels_are_the_same = pixels_are_the_same && ( s == *i );
        ++count;
    }
    BOOST_CHECK( pixels_are_the_same );
    BOOST_CHECK_EQUAL( count, 25*25 );
}

}
}
