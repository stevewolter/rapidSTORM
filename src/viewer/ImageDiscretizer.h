#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/input/Traits.h>
#include <dStorm/outputs/BinnedLocalizations.h>
#include <dStorm/Pixel.h>
#include "Publisher.h"
#include "HighDepth.h"

namespace dStorm {
namespace viewer {

struct DiscretizationListener {
    static const bool NeedLiveHistogram = true;

    /** The setSize method is called before any announcements are made,
        *  and is called afterwards when the image size changes. */
    void setSize(
        const input::Traits< Image<int,2> >&);
    /** Called when a pixel changes in the image. The parameters
        *  give the position of the changed pixel. */
    void pixelChanged(int x, int y, HighDepth to_value);
    /** Update the output brightness of all pixels with input brightness
     *  level \c at */
    void changeBrightness(HighDepth at);
    /** The listener should apply pending changes. */
    void clean(bool final);
    /** The state should be reset to an empty image. */
    void clear();
    /** This method is called when the discretization parameters change.
        *  @param index The index of this colour in the key.
        *  @param pixel The color for which the discretization changed.
        *               This parameter's type in an implementation should
        *               not be 
        *  @param value The new upper bound on the number of A/D counts for
        *               this pixel. */
    void notice_key_change( int index, Pixel pixel, float value );
};

struct DummyDiscretizationListener {
    static const bool NeedLiveHistogram = false;

    void setSize(const input::Traits< Image<int,2> >&) {}
    void pixelChanged(int, int, HighDepth) {}
    void changeBrightness(HighDepth at) {}
    void clean(bool) {}
    void clear() {}
    void notice_key_change( int, Pixel, float ) {}
};

template <typename ImageListener>
class Discretizer 
: public outputs::BinningListener,
  public Publisher<ImageListener>
{
    typedef typename ImageListener::Colorizer Colorizer;
    typedef typename Colorizer::BrightnessType LowDepth;

    typedef Image<float,2> InputImage;

    typedef data_cpp::Vector<LowDepth> TransitionTable;

    unsigned int total_pixel_count;
    Colorizer& colorizer;

    float max_value, max_value_used_for_disc_factor,
          disc_factor;

    data_cpp::Vector<unsigned int> histogram;
    TransitionTable transition;

    static const HighDepth background_threshold;
    unsigned int in_depth, out_depth,
                 pixels_above_used_max_value;
    float histogram_power;

    const InputImage& binned_image;

    HighDepth discretize( float value, float disc_factor ) 
        { return std::min<HighDepth>( in_depth-1,
            HighDepth(round( value * disc_factor )) ); }
    HighDepth discretize( float value ) 
        { return discretize(value, disc_factor); }

    void normalize_histogram();
    void publish_differences_in_transitions( 
        TransitionTable* old_table, TransitionTable& new_table );
    inline unsigned long int non_background_pixels();

    template <class> friend class Discretizer;

  public:
    Discretizer(
        int intermediate_depth, 
        float histogram_power, 
        const Image<float,2>& binned_image,
        Colorizer& colorizer);
    template <typename OtherListener>
    Discretizer( 
        const Discretizer<OtherListener>&, 
        const Image<float,2>& binned_image,
        Colorizer& colorizer);
    ~Discretizer();

    void setSize( const input::Traits<InputImage>& );
    inline void updatePixel(int, int, float, float);
    void clean(bool final);
    void clear();

    void announce(const output::Output::Announcement& a) 
        { colorizer.announce(a); }
    void announce(const output::Output::EngineResult& er)
        { colorizer.announce(er); }
    void announce(const Localization& loc)
        { colorizer.announce( loc ); }

    Pixel get_pixel( int x, int y ) {
        return colorizer.getPixel(x, y, 
            transition[ discretize( binned_image(x,y) ) ]);
    }
    Pixel get_pixel( int x, int y, HighDepth discretized ) {
        return colorizer.getPixel(x, y, transition[ discretized ]);
    }
    Pixel get_background() 
        { return colorizer.get_background(); }

    inline float key_value( LowDepth key );

    void setHistogramPower(float power);
};

}
}

#endif