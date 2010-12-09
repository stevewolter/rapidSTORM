#ifndef DSTORM_VIEWER_HUESATURATIONMIXER_H
#define DSTORM_VIEWER_HUESATURATIONMIXER_H

#include <dStorm/Image.h>
#include "Colorizer_decl.h"
#include "Config_decl.h"
#include <dStorm/Pixel.h>

namespace dStorm {
namespace viewer {

class HueSaturationMixer {
    typedef Eigen::Matrix<float,2,1,Eigen::DontAlign> ColourVector;

    /** Offset for tone calculations. */
    ColourVector base_tone,
    /** Tone for currently processed points in radial Hue/Sat space */
                 current_tone;
    /** Tone for currently processed points in cartesian Hue/Sat space. */
    ColourVector tone_point;

    typedef ColourSchemes::RGBWeight RGBWeight;

    dStorm::Image<ColourVector,2> colours;
    dStorm::Image<RGBWeight,2> rgb_weights;

  public:
    HueSaturationMixer( const Config& config );
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

#endif
