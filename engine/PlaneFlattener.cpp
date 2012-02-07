#include "debug.h"
#include "PlaneFlattener.h"
#include <dStorm/image/iterator.h>
#include <dStorm/image/slice.h>
#include <dStorm/image/constructors.h>
#include <dStorm/traits/ScaledProjection.h>

namespace dStorm {
namespace engine {

PlaneFlattener::PlaneFlattener( const dStorm::engine::InputTraits& traits )
: optics(traits)
{
    buffer = Image2D( traits.size.head<2>() );
    Transformed::Size sz = traits.size.head<2>();
    sz.z() -= 1 * camera::pixel;

    const traits::ScaledProjection& in_result = 
        dynamic_cast< const traits::ScaledProjection& >
            ( *traits.plane(0).projection() );

    for (int p = 1; p < traits.plane_count(); ++p) {
        Transformed t;
        for ( Transformed::iterator i = t.begin(); i != t.end(); ++i )
            *i = value( in_result.point_in_image_space(
                traits.plane( p ).projection()->
                    pixel_in_sample_space( i.uposition()) ) );
        transformed.push_back( t );
    }
}

const Image2D
PlaneFlattener::flatten_image( const engine::Image& multiplane )
{
    if ( multiplane.depth_in_pixels() == 1 ) return multiplane.slice(2, 0 * camera::pixel);

    std::copy( 
        multiplane.slice(2, 0 * camera::pixel).begin(), 
        multiplane.slice(2, 0 * camera::pixel).end(), 
        buffer.begin() );

    typedef dStorm::Image<StormPixel,1> Line;

    for (int plane = 1; plane < multiplane.depth_in_pixels(); ++plane)  {
        DEBUG("Flattening plane " << plane);
        dStorm::Image< engine::StormPixel, 2 > p = 
            multiplane.slice( 2, plane * camera::pixel );
        Transformed::const_iterator t = transformed[plane-1].begin();
        for ( dStorm::Image< engine::StormPixel, 2 >::const_iterator i = p.begin(); i != p.end(); ++i )
        {
            int bx = floor( t->x() ), by = floor( t->y() );
            for (int xo = 0; xo <= 1; ++xo)
                for (int yo = 0; yo <= 1; ++yo) {
                    int x = bx + xo, y = by + yo;
                    if ( buffer.contains( x, y ) ) {
                        float factor = (1 - std::abs( x - t->x() )) 
                                * (1 - std::abs( y - t->y() ));
                        int value = round( *i * factor );
                        buffer(x,y) += value;
                    }
                }
        }
    }

    return buffer;
}

}
}
