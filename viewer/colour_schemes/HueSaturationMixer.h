#ifndef DSTORM_VIEWER_HUESATURATIONMIXER_H
#define DSTORM_VIEWER_HUESATURATIONMIXER_H

#include <dStorm/Image.h>
#include <dStorm/Pixel.h>
#include "base_impl.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class HueSaturationMixer {
    typedef Eigen::Matrix<float,2,1,Eigen::DontAlign> ColourVector;

    /** Offset for tone calculations. */
    ColourVector base_tone,
    /** Tone for currently processed points in radial Hue/Sat space */
                 current_tone;
    /** Tone for currently processed points in cartesian Hue/Sat space. */
    ColourVector tone_point;

    dStorm::Image<ColourVector,2> colours;
    dStorm::Image<RGBWeight,2> rgb_weights;

  public:
    HueSaturationMixer( double base_hue, double base_saturation );
    void set_base_tone( double hue, double saturation ) { base_tone[0] = hue; base_tone[1] = saturation; }
    ~HueSaturationMixer();
    void set_tone( float hue );
    void merge_tone( int x, int y, 
                     float old_data_weight, float new_data_weight );
    void setSize( const dStorm::Image<ColourVector,2>::Size& size );

    inline Pixel getPixel( int x, int y, 
                           unsigned char val )  const
        { return Pixel( val * rgb_weights(x,y) ); }
    inline Pixel getKeyPixel( unsigned char val ) const 
        { return Pixel( val ); }

    inline void updatePixel(int x, int y, 
                            float oldVal, float newVal) 
        { merge_tone(x, y, oldVal, newVal - oldVal); }
};

}
}
}

#endif
