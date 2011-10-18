#include "iterator.h"
#include "constructors.h"

using boost::units::camera::pixel;

int main() {
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
        assert( count == i.position().x() + sz.x().value() * i.position().y() + (sz.x() * sz.y()).value() * i.position().z() );
        assert( *i == i.position().x() + 100 * i.position().y() + 10000 * i.position().z() );
        ++count;
    }
    assert( count == (sz.x() * sz.y() * sz.z()).value() );

    count = 0;
    for ( dStorm::Image<int,3>::const_iterator i = static_cast<const Image&>(image).begin(), e = image.end(); i != e; ++i ) {
        assert( count == i.position().x() + sz.x().value() * i.position().y() + (sz.x() * sz.y()).value() * i.position().z() );
        assert( *i == i.position().x() + 100 * i.position().y() + 10000 * i.position().z() );
        ++count;
    }
    assert( count == (sz.x() * sz.y() * sz.z()).value() );
    return 0;
}
