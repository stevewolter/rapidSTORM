#include "dejagnu.h"
#include "iterator.h"
#include "constructors.h"

using boost::units::camera::pixel;

namespace dStorm {

void image_iterator_unit_test( TestState& state )
{
    typedef dStorm::Image<int,3> Image;
    Image::Size sz;
    sz << 5 * pixel, 10 * pixel, 7 * pixel;
    Image image(sz);
    for (int x = 0; x < image.width_in_pixels(); ++x)
        for (int y = 0; y < image.height_in_pixels(); ++y)
            for (int z = 0; z < image.depth_in_pixels(); ++z)
                image(x,y,z) = x+100*y+10000*z;

    dStorm::Image<int,3>::iterator i = image.begin(), e = image.end();
    int count = 0;
    for ( ; i != e; ++i, --i, ++i ) {
        assert( count == i.position().x().value() + sz.x().value() * i.position().y().value() + (sz.x() * sz.y()).value() * i.position().z().value() );
        assert( *i == i.position().x().value() + 100 * i.position().y().value() + 10000 * i.position().z().value() );
        ++count;
    }
    state( count == (sz.x() * sz.y() * sz.z()).value() );

    count = 0;
    bool position_right = true, value_right = true;
    for ( dStorm::Image<int,3>::const_iterator i = static_cast<const Image&>(image).begin(), e = image.end(); i != e; ++i ) {
        position_right = position_right &&
            ( count == i.position().x().value() + (sz.x() * i.position().y()).value() + (sz.x() * sz.y() * i.position().z()).value() );
        value_right = value_right &&
            ( *i == i.position().x().value() + (100 * i.position().y()).value() + (10000 * i.position().z()).value() );
        ++count;
    }
    state( position_right, "Image iterator gets all positions right" );
    state( value_right, "Image iterator gets all values right" );
    state( count == (sz.x() * sz.y() * sz.z()).value(), "Image iterator iterates completely" );
}

}
