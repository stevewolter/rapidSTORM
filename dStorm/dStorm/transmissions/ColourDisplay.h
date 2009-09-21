#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_H

#include <dStorm/transmissions/BinnedLocalizations.h>
#include <dStorm/transmissions/HighDepthImage.h>
#include <dStorm/transmissions/ViewerConfig.h>
#include <CImg.h>
#include <Eigen/Core>

namespace dStorm {

namespace ColourSchemes {
    enum Scheme { BlackWhite, BlackRedYellowWhite,
                  FixedHue, TimeHue, ExtraHue, ExtraSaturation,
                  FirstColourModel = BlackWhite,
                  LastColourModel = ExtraSaturation};

    void rgb_weights_from_hue_saturation
        ( float hue, float saturation, float *weightv, int step ) 
;
    void convert_xy_tone_to_hue_sat( 
        float x, float y, float& hue, float& sat );
};

template <int Hueing> struct ColourData { 
  public:
    typedef Eigen::Matrix<float,2,1,Eigen::DontAlign> ColourVector;

    float minV, maxV, scaleV;

    /** Offset for tone calculations. */
    ColourVector base_tone,
    /** Tone for currently processed points in radial Hue/Sat space */
                 current_tone;
    /** Tone for currently processed points in cartesian Hue/Sat space. */
    ColourVector tone_point;

    cimg_library::CImg<ColourVector> colours;
    cimg_library::CImg<float> rgb_weights;

    void set_tone( float hue, float saturation ) {
        current_tone = base_tone + ColourVector( hue, saturation );
        tone_point.x() = cosf( 2 * M_PI * current_tone[0] ) 
                            * current_tone[1];
        tone_point.y() = sinf( 2 * M_PI * current_tone[0] ) 
                            * current_tone[1];
    }

    void merge_tone( int x, int y, 
                     float old_data_weight, float new_data_weight )
    {
        assert( int(colours.width) > x && int(colours.height) > y );
        ColourVector hs;
        if ( old_data_weight < 1E-3 ) {
            colours(x,y) = tone_point;
            hs = current_tone;
        } else {
            colours(x,y) = 
                ( colours(x,y) * old_data_weight + 
                  tone_point * new_data_weight ) /
                  ( old_data_weight + new_data_weight );
            ColourSchemes::convert_xy_tone_to_hue_sat
                ( colours(x,y).x(), colours(x,y).y(), hs[0], hs[1] );
        }
        int pixels_per_colour = rgb_weights.width * rgb_weights.height;
        ColourSchemes::rgb_weights_from_hue_saturation
            ( hs[0], hs[1], &rgb_weights(x,y), pixels_per_colour );
                
    }

    ColourData(const Viewer::Config& config)
        { base_tone[0] = config.hue(); base_tone[1] = config.saturation(); } 

    void setSize( int w, int h ) {
        colours = cimg_library::CImg<ColourVector>(w,h);
        rgb_weights.resize( w, h, 1, 3, -1 );
    }
    void clear() {}
};

template <> struct ColourData<ColourSchemes::BlackWhite> {
    ColourData(const Viewer::Config& ) {} 
    void setSize(int,int) {}
    void clear() {}
};
template <> struct ColourData<ColourSchemes::BlackRedYellowWhite> {
    ColourData(const Viewer::Config& ) {} 
    void setSize(int,int) {}
    void clear() {}
};
template <> struct ColourData<ColourSchemes::FixedHue> {
  public:
    float rgb_weights[3];

    ColourData(const Viewer::Config& config) {
        ColourSchemes::rgb_weights_from_hue_saturation
            ( config.hue(), config.saturation(), rgb_weights, 1 );
    }
    void setSize(int,int) {}
    void clear() {}
};

template <int Hueing>
class ColouredImage {
    typedef cimg_library::CImg<unsigned char> Img;
    Img result_image;
    bool invert;
    int pixelChanges, changeThreshold;
    ColourData<Hueing> data;
    int xo, yo;

    void setPixel( int x, int y, int dim, unsigned char v )
        { result_image(x,y,0,dim) = (invert) ? (~v) : v; pixelChanges++; }
    inline
    unsigned char pixelValue( int x, int y, int dim, SmoothedPixel br )
        const;

  public:
    const static int 
        Dim = (Hueing == ColourSchemes::BlackWhite) ? 1 : 3,
        DesiredDepth = (Hueing == ColourSchemes::BlackRedYellowWhite)
            ? 767 : 255; ;

    ColouredImage(const Viewer::Config& config)
        : invert(config.invert()),
          pixelChanges(0), changeThreshold(20), data(config), xo(0), yo(0) {}

    /** setSize() from BinningListener */
    void setSize(int w, int h) { data.setSize(w,h); }
    inline void announce(const Output::Announcement&);
    inline void announce(const Output::EngineResult&);
    void announce(const Localization&) {}
    void clean(const cimg_library::CImg<float>&) {}

    void setSize(int width, int height, int ) {
        result_image.resize( width, height, 1, 
            (Hueing == ColourSchemes::BlackWhite) ? 1 : 3, -1 );
        result_image.fill( (invert) ? (~0) : 0 );
        pixelChanges = changeThreshold;
    }

    inline void updatePixel(int, int, float, float);

    void updatePixel(
        int x, int y, 
        SmoothedPixel, SmoothedPixel newVal 
    ) 
    {
        for (int i = 0; i < Dim; i++)
            setPixel(x, y, i, pixelValue(x+xo, y+yo, i, newVal) );
    }

    void clean() {}

    void clear() { 
        result_image.fill( (invert) ? 255 : 0 );
        pixelChanges = changeThreshold;
        data.clear();
    }

    bool need_redisplay() 
        { return pixelChanges >= changeThreshold; }
    const cimg_library::CImg<unsigned char>& get_image()
        { pixelChanges = 0; return result_image; }

    bool notify_of_all_moved_pixels() const 
        { return Hueing == ColourSchemes::TimeHue; }
    void setOffset(int nxo, int nyo) { xo = nxo; yo = nyo; }

    std::auto_ptr<Img> colour( 
        const cimg_library::CImg<SmoothedPixel>& brightness )
    {
        std::auto_ptr<Img> img( 
            new Img( brightness.width, brightness.height, 
                     brightness.depth, Dim ) );

        if (invert)
            cimg_forXYZV( *img, x, y, z, v )
              (*img)(x,y,z,v) = ~ pixelValue(x,y,v, brightness(x,y,z,0));
        else
            cimg_forXYZV( *img, x, y, z, v )
              (*img)(x,y,z,v) = pixelValue(x,y,v, brightness(x,y,z,0));

        return img;
    }
};

}

#endif
