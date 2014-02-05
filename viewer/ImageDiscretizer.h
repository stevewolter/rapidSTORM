#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_H

#include <dStorm/input/Traits.h>
#include <dStorm/Pixel.h>
#include "viewer/Publisher.h"
#include "viewer/HighDepth.h"
#include <vector>
#include "viewer/Image.h"
#include "viewer/ColourScheme.h"
#include "density_map/VirtualListener.h"

namespace dStorm {
namespace viewer {

struct DiscretizationListener {
    static const bool NeedLiveHistogram = true;
    typedef dStorm::image::MetaInfo<Im::Dim> MetaInfo;

    /** The setSize method is called before any announcements are made,
        *  and is called afterwards when the image size changes. */
    void setSize( const MetaInfo& );
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
    typedef DiscretizationListener::MetaInfo MetaInfo;

    void setSize(const MetaInfo&) {}
    void pixelChanged(Im::Position, HighDepth) {}
    void changeBrightness(HighDepth at) {}
    void clean(bool) {}
    void clear() {}
    void notice_key_change( int, Pixel, float ) {}
};

template <typename ImageListener>
class Discretizer 
: public density_map::VirtualListener<Im::Dim>,
  public Publisher<ImageListener>
{
    typedef ColourScheme::BrightnessType LowDepth;

    typedef Image<float,Im::Dim> InputImage;

    typedef std::vector<LowDepth> TransitionTable;

    unsigned int total_pixel_count;
    ColourScheme& colorizer;

    float max_value, max_value_used_for_disc_factor,
          disc_factor;

    std::vector<unsigned int> histogram;
    TransitionTable transition;

    static const HighDepth background_threshold;
    unsigned int in_depth, out_depth,
                 pixels_above_used_max_value;
    float histogram_power, cutoff_factor;

    const InputImage& binned_image;

    HighDepth discretize( float value, float disc_factor ) const
        { return std::min<HighDepth>( in_depth-1,
            HighDepth(round( value * disc_factor )) ); }
    HighDepth discretize( float value ) const
        { return discretize(value, disc_factor); }

    void rediscretize();
    void normalize_histogram();
    void publish_differences_in_transitions( 
        TransitionTable* old_table, TransitionTable& new_table );
    inline unsigned long int non_background_pixels();

    template <class> friend class Discretizer;

  public:
    Discretizer(
        int intermediate_depth, 
        float histogram_power, 
        const InputImage& binned_image,
        ColourScheme& colorizer);
    template <typename OtherListener>
    Discretizer( 
        const Discretizer<OtherListener>&, 
        const InputImage& binned_image,
        ColourScheme& colorizer);
    ~Discretizer();

    void setSize( const MetaInfo& );
    void updatePixel(const Im::Position&, float, float);
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
    void set_top_cutoff(float value);
};

template < typename ImageListener>
const HighDepth
Discretizer< ImageListener>::background_threshold = 1;

template < typename ImageListener>
float 
Discretizer< ImageListener>::key_value( LowDepth key ) const
{
    unsigned int n = -1; 
    while ( n+1 < in_depth && transition[n+1] <= key ) n++;
    return (n+0.5f) / disc_factor;
}

template < typename ImageListener>
inline unsigned long int Discretizer< ImageListener>::non_background_pixels()
{
    long int accum = 0;
    assert( histogram.size() >= background_threshold );
    for (unsigned int i = 0; i < background_threshold; i++)
        accum += histogram[i];
    return binned_image.size_in_pixels() - accum;
}

}
}

#endif
