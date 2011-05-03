#include "debug.h"
#include "PlaneFlattener.h"
#include <dStorm/image/iterator.h>
#include <dStorm/image/slice.h>
#include <dStorm/image/constructors.h>
#include <dStorm/matrix_operators.h>

namespace dStorm {
namespace engine {

PlaneFlattener::PlaneFlattener( const dStorm::engine::InputTraits& traits )
: optics(traits)
{
    buffer = Image2D( traits.size.start<2>() );
    coordinates.resize( 3, traits.size.x() / camera::pixel );
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
        for (int y = 0; y < multiplane.height_in_pixels(); ++y) {
            coordinates.row(1).fill( y );

            int p = 0;
            Line line = multiplane.slice(2, plane * camera::pixel)
                                  .slice(1, y * camera::pixel );
            for ( Line::iterator i = line.begin(), e = line.end(); i != e; 
                  ++i, ++p ) 
                coordinates( 0, p ) = i.position().x();

            DEBUG("Assembled line " << y << " to\n" << coordinates);
            optics.plane(plane).points_in_sample_space( coordinates );
            DEBUG("Intermediate-Transformed line " << y << " to\n" << coordinates);
            optics.plane(0).points_in_image_space( coordinates );

            DEBUG("Transformed line " << y << " to\n" << coordinates);
            p = 0;
            for ( Line::iterator i = line.begin(), e = line.end(); i != e; 
                  ++i, ++p ) 
            {
                int bx = floor( coordinates(0,p) ), by = floor( coordinates(1,p) );
                for (int xo = 0; xo <= 1; ++xo)
                  for (int yo = 0; yo <= 1; ++yo) {
                    int x = bx + xo, y = by + yo;
                    if ( buffer.contains( x, y ) ) {
                        float factor = (1 - std::abs( x - coordinates(0,p) )) 
                               * (1 - std::abs( y - coordinates(1,p) ));
                        int value = round( *i * factor );
                        buffer(x,y) += value;
                    }
                  }
            }
        }
    }

    return buffer;
}

}
}
