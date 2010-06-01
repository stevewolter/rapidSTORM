#include "ColourDisplay.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace dStorm {
namespace viewer {
namespace ColourSchemes {

enum ColourPart { V, P, Q, T };
static const ColourPart color_index_table[6][3] 
    = { { V, T, P}, { Q, V, P}, { P, V, T}, { P, Q, V},
        { T, P, V}, { V, P, Q} };

void rgb_weights_from_hue_saturation
    (float hue, float saturation, RGBWeight &array) 
{
    int hue_index = std::max(0, std::min<int>( floor( hue * 6 ), 5 ));
    float f = hue * 6 - hue_index;

    float parts[4];
    parts[V] = 1;
    parts[P] = 1 - saturation;
    parts[Q] = 1 - f*saturation;
    parts[T] = 1 - (1 - f) * saturation;

    for (int c = 0; c < 3; c++)
        array[c] = parts[ color_index_table[hue_index][c] ];

}

static const Eigen::Rotation2D<float> 
    rot_90_deg_clockw( - M_PI / 2 ),
    rot_30_deg_clockw( - M_PI / 6 );

void convert_xy_tone_to_hue_sat( 
    float x, float y, float& hue, float& sat ) 
{
    Eigen::Vector2f v(x,y), ov = v;
    if ( v.squaredNorm() < 1E-4 )  {
        hue = 0;
        sat = 0;
    } else {
        sat = v.norm();

        int rotations = 0;
        if ( v.y() < 0 ) { v *= -1; rotations += 6; }
        if ( v.x() < 0 ) {
            std::swap( v.x(), v.y() );
            v.y() *= -1;
            rotations += 3; 
        }

        while ( v.y() > 0.6 * v.x() ) {
            v = rot_30_deg_clockw * v;
            rotations+= 1;
        }

        float approximate_position = (2*v.y()) / v.x();
        hue = (rotations + approximate_position) / 12;
    }
}

}
}
}
