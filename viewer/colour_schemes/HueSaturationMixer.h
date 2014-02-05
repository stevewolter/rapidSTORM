#ifndef DSTORM_VIEWER_HUESATURATIONMIXER_H
#define DSTORM_VIEWER_HUESATURATIONMIXER_H

#include <dStorm/Pixel.h>
#include "viewer/Image.h"
#include "viewer/colour_schemes/base.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class HueSaturationMixer {
    typedef Eigen::Matrix<float,2,1,Eigen::DontAlign> ColourVector;

    /** Offset for tone calculations. */
    ColourVector base_tone;
    /** Tone for currently processed points in cartesian Hue/Sat space. */
    ColourVector tone_point;

    dStorm::Image<ColourVector,Im::Dim> colours;

  public:
    HueSaturationMixer( double base_hue, double base_saturation );
    void set_base_tone( double hue, double saturation ) { base_tone[0] = hue; base_tone[1] = saturation; }
    ~HueSaturationMixer();
    void set_tone( float hue );
    void merge_tone( const Im::Position&, 
                     float old_data_weight, float new_data_weight );
    void setSize( const dStorm::Image<ColourVector,Im::Dim>::Size& size );

    Pixel getPixel( const Im::Position& p,
                           unsigned char val )  const;
    inline Pixel getKeyPixel( unsigned char val ) const 
        { return Pixel( val ); }

    inline void updatePixel(const Im::Position& p,
                            float oldVal, float newVal) 
        { merge_tone(p, oldVal, newVal - oldVal); }
};

}
}
}

#endif
