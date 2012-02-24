#include "debug.h"
#include "PlaneFlattener.h"
#include <dStorm/image/iterator.h>
#include <dStorm/image/slice.h>
#include <dStorm/image/constructors.h>
#include <dStorm/traits/ScaledProjection.h>
#include <dStorm/engine/InputTraits.h>

namespace dStorm {
namespace engine {

PlaneFlattener::PlaneFlattener( 
    const dStorm::engine::InputTraits& traits,
    const std::vector<float> weights
) : traits(traits), weights(weights)
{
    buffer = Image2D( traits.image(0).size );

    const traits::ScaledProjection& in_result = 
        dynamic_cast< const traits::ScaledProjection& >
            ( traits.plane(0).projection() );

    for (int p = 1; p < traits.plane_count(); ++p) {
        Transformed t( traits.image(0).size );
        const traits::Projection& proj = traits.plane( p ).projection();
        for ( Transformed::iterator i = t.begin(); i != t.end(); ++i )
            *i = value( in_result.point_in_image_space(
                    proj.pixel_in_sample_space( i.position()) ) );
        transformed.push_back( t );
    }

    assert( int(weights.size()) == traits.plane_count() - 1 );
}

const Image2D
PlaneFlattener::flatten_image( const engine::ImageStack& multiplane )
{
    if ( multiplane.plane_count() == 1 ) return multiplane.plane(0);

    std::copy( 
        multiplane.plane(0).begin(), 
        multiplane.plane(0).end(), 
        buffer.begin() );

    typedef dStorm::Image<StormPixel,1> Line;

    assert( multiplane.plane_count() == int(transformed.size()) + 1 );
    for (int plane = 1; plane < multiplane.plane_count(); ++plane)  {
        DEBUG("Flattening plane " << plane);
        dStorm::Image< engine::StormPixel, 2 > p = multiplane.plane(plane);
        DEBUG("Sizes are " << p.sizes().transpose() << " and " << transformed[plane-1].sizes().transpose() );
        assert( (p.sizes() == transformed[plane-1].sizes()).all() );
        Transformed::const_iterator t = transformed[plane-1].begin();
        for ( dStorm::Image< engine::StormPixel, 2 >::const_iterator i = p.begin(); i != p.end(); ++i, ++t )
        {
            int bx = floor( t->x() ), by = floor( t->y() );
            for (int xo = 0; xo <= 1; ++xo)
                for (int yo = 0; yo <= 1; ++yo) {
                    int x = bx + xo, y = by + yo;
                    if ( buffer.contains( x, y ) ) {
                        float factor = (1 - std::abs( x - t->x() )) 
                                * (1 - std::abs( y - t->y() ));
                        int value = round( *i * factor * weights[plane-1] );
                        buffer(x,y) += value;
                    }
                }
        }
    }

    return buffer;
}

}
}
