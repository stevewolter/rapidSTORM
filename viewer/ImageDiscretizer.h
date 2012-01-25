#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_H

#include <dStorm/input/Traits.h>
#include <dStorm/outputs/BinnedLocalizations.h>
#include <dStorm/Pixel.h>
#include "Publisher.h"
#include "HighDepth.h"
#include <vector>
#include "Image.h"

namespace dStorm {
namespace viewer {

struct DiscretizationListener {
    static const bool NeedLiveHistogram = true;

    /** The setSize method is called before any announcements are made,
        *  and is called afterwards when the image size changes. */
    void setSize(
        const input::Traits< Im >&);
    /** Called when a pixel changes in the image. The parameters
        *  give the position of the changed pixel. */
    void pixelChanged(Im::Position, HighDepth to_value);
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

    void setSize(const input::Traits< Im >&) {}
    void pixelChanged(Im::Position, HighDepth) {}
    void changeBrightness(HighDepth at) {}
    void clean(bool) {}
    void clear() {}
    void notice_key_change( int, Pixel, float ) {}
};

template <typename ImageListener, typename Colorizer_>
class Discretizer 
: public outputs::BinningListener<Im::Dim>,
  public Publisher<ImageListener>
{
    typedef Colorizer_ Colorizer;
    typedef typename Colorizer::BrightnessType LowDepth;

    typedef Image<float,Im::Dim> InputImage;

    typedef std::vector<LowDepth> TransitionTable;

    unsigned int total_pixel_count;
    Colorizer& colorizer;

    float max_value, max_value_used_for_disc_factor,
          disc_factor;

    std::vector<unsigned int> histogram;
    TransitionTable transition;

    static const HighDepth background_threshold;
    unsigned int in_depth, out_depth,
                 pixels_above_used_max_value;
    float histogram_power;

    const InputImage& binned_image;

    HighDepth discretize( float value, float disc_factor ) const
        { return std::min<HighDepth>( in_depth-1,
            HighDepth(round( value * disc_factor )) ); }
    HighDepth discretize( float value ) const
        { return discretize(value, disc_factor); }

    void normalize_histogram();
    void publish_differences_in_transitions( 
        TransitionTable* old_table, TransitionTable& new_table );
    inline unsigned long int non_background_pixels();

    template <class,class> friend class Discretizer;

  public:
    Discretizer(
        int intermediate_depth, 
        float histogram_power, 
        const InputImage& binned_image,
        Colorizer& colorizer);
    template <typename OtherListener>
    Discretizer( 
        const Discretizer<OtherListener,Colorizer>&, 
        const InputImage& binned_image,
        Colorizer& colorizer);
    ~Discretizer();

    void setSize( const input::Traits<InputImage>& );
    inline void updatePixel(const Im::Position&, float, float);
    void clean(bool final);
    void clear();

    void announce(const output::Output::Announcement& a) 
        { colorizer.announce(a); }
    void announce(const output::Output::EngineResult& er)
        { colorizer.announce(er); }
    void announce(const Localization& loc)
        { colorizer.announce( loc ); }

    Pixel get_pixel( const Im::Position& p ) const {
        return colorizer.getPixel(p,
            transition[ discretize( binned_image(p) ) ]);
    }
    Pixel get_pixel( const Im::Position& p, HighDepth discretized ) const {
        return colorizer.getPixel(p, transition[ discretized ]);
    }
    Pixel get_background() const
        { return colorizer.get_background(); }

    inline float key_value( LowDepth key ) const;

    void setHistogramPower(float power);
};

}
}

#endif
