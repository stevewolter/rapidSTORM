#include "debug.h"
#include "PlaneFlattener.h"
#include <dStorm/image/iterator.h>
#include <dStorm/image/slice.h>
#include <dStorm/image/constructors.h>
#include <dStorm/engine/InputTraits.h>

namespace dStorm {
namespace engine {

PlaneFlattener::PlaneFlattener( 
    const dStorm::engine::InputTraits& traits,
    const std::vector<float> weights
) : traits(traits), weights(weights)
{
    buffer = Image2D( traits.image(0).size );

    traits::Projection::SamplePosition samplepos;
    typedef traits::Projection::SubpixelImagePosition Subpixel;
    Subpixel epsilon = Subpixel::Constant( 1E-3f * camera::pixel );

    for (int p = 1; p < traits.plane_count(); ++p) {
        Transformed t( traits.image(0).size );
        for ( Transformed::iterator i = t.begin(); i != t.end(); ++i ) {
            samplepos = traits.projection(0).pixel_in_sample_space( i.position() );
            *i = traits.projection(p).point_in_image_space(samplepos);
            *i = i->array().max( epsilon.array() )
                .min( (traits.image(p).upper_bound().cast<Subpixel::Scalar>() - epsilon).array() );
        }
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
        for ( Image2D::iterator i = buffer.begin(); i != buffer.end(); ++i, ++t )
        {
            int bx = floor( t->x().value() ), by = floor( t->y().value() );
            for (int xo = 0; xo <= 1; ++xo)
                for (int yo = 0; yo <= 1; ++yo) {
                    int x = bx + xo, y = by + yo;
                    float factor = (1 - std::abs( x - t->x().value() )) 
                            * (1 - std::abs( y - t->y().value() ));
                    int value = round( p(x,y) * factor * weights[plane-1] );
                    *i += value;
                }
        }
    }

    return buffer;
}

}
}

#include "dejagnu.h"
#include <dStorm/traits/AffineProjection.h>
#include <dStorm/traits/ScaledProjection.h>

namespace dStorm {
namespace engine {

void check_plane_flattener( TestState& state ) {
    dStorm::engine::InputTraits traits;
    dStorm::image::MetaInfo<2> size;
    size.size.fill( 100 * camera::pixel );
    size.set_resolution( 0, (100E-9 * si::meter) / camera::pixel);
    size.set_resolution( 1, (107E-9 * si::meter) / camera::pixel);

    dStorm::traits::Optics o1, o2;
    o1.set_projection_factory( traits::test_affine_projection() );
    o2.set_projection_factory( traits::test_scaled_projection() );
    traits.push_back( size, o1 );
    traits.plane(0).create_projection();
    traits.push_back( size, o2 );
    traits.plane(1).create_projection();

    std::vector<float> weights;
    weights.push_back( 0.5 );

    PlaneFlattener flattener( traits, weights );

    engine::ImageStack stack( 2 * camera::frame );
    stack.push_back( engine::Image2D(size.size) );
    stack.push_back( engine::Image2D(size.size) );

    int foo = 0;
    for (int z = 0; z < 2; ++z)
        for ( engine::Image2D::iterator i = stack.plane(z).begin(); i != stack.plane(z).end(); ++i )
            *i = (++foo % 23 ) + 100;

    const engine::Image2D result = flattener.flatten_image( stack );
    bool all_in_range = true;
    for ( engine::Image2D::const_iterator i = result.begin(); i != result.end(); ++i ) {
        if ( *i < 100 * 1.5 || *i > 122 * 1.5 )
            all_in_range = false;
    }

    state( all_in_range, "All interpolated data are in range" );
    state( result(0,0) == 159 );
    state( result(15,10) == 155 );
    state( result(99,99) == 174 );
}

}
}
