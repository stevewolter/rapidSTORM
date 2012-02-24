#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H

#include <boost/units/io.hpp>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/Image_iterator.h>
#include <dStorm/Image_impl.h>
#include <algorithm>

#include "ImageDiscretizer_inline.h"

namespace dStorm {
namespace viewer {

template <typename ImageListener, typename Colorizer_>
Discretizer<ImageListener,Colorizer_>
::Discretizer(int d, float hp,
    const Image<float,Im::Dim>& binned_image,
    Colorizer& colorizer) 
: colorizer(colorizer),
  max_value(10), 
  max_value_used_for_disc_factor(10),
  disc_factor( (d-1) * 1.0 / max_value_used_for_disc_factor ),
  histogram( d, 0 ),
  transition( d, 0 ),
  in_depth( d ),
  out_depth( Colorizer::BrightnessDepth - 1 ),
  pixels_above_used_max_value(0),
  histogram_power( hp ),
  binned_image(binned_image)
{
}

template <typename ImageListener, typename Colorizer_>
Discretizer<ImageListener,Colorizer_>
::~Discretizer() {}

template <typename ImageListener, typename Colorizer_>
void Discretizer<ImageListener,Colorizer_>
::setSize( const MetaInfo& traits )
{
    colorizer.setSize( traits );
    this->publish().setSize( traits );
    int w = traits.size.x() / camera::pixel,
        h = traits.size.y() / camera::pixel;

    total_pixel_count = w * h;

    for (unsigned int i = 0; i < in_depth; i++) {
        histogram[0] = (i==0) ? total_pixel_count : 0;
    }
}

template <typename ImageListener, typename Colorizer_>
void Discretizer<ImageListener,Colorizer_>
  ::clean(bool final)
{
    if ( final || pixels_above_used_max_value >
           non_background_pixels() / 100 )
    {
        for (size_t i = 0; i < histogram.size(); i++)
            histogram[i] = 0;

        float new_disc_fac = (in_depth-1) * 1.0 / max_value;

        for( InputImage::const_iterator i = binned_image.begin();
            i != binned_image.end(); i++)
        {
            HighDepth n = discretize( *i, new_disc_fac );

            if ( discretize( *i ) != n )
                this->publish().pixelChanged( i.position(), n );

            ++histogram[n];
        }

        disc_factor = new_disc_fac;
        publish_differences_in_transitions( NULL, transition );
        max_value_used_for_disc_factor = max_value;
        pixels_above_used_max_value = 0;
    }
    normalize_histogram();
    colorizer.clean(final);
    this->publish().clean(final);
}

template <typename ImageListener, typename Colorizer_>
void Discretizer<ImageListener,Colorizer_>
  ::publish_differences_in_transitions
  ( TransitionTable* old_table, TransitionTable& new_table )
{
    HighDepth o = 0, n = 0;
    for (unsigned int v = 0; v <= out_depth; v++) {
        if ( old_table )
            while ( (o+1U) < in_depth && (*old_table)[o+1U] <= v ) o++;
        while ( (n+1U) < in_depth && new_table[n+1U] <= v ) n++;

        if ( !old_table || o != n ) {
            float undisc_new_val = (n + 0.5) / disc_factor;
            this->publish().notice_key_change
                ( v, colorizer.getKeyPixel(v), undisc_new_val );
        }
    }
}

template <typename ImageListener, typename Colorizer_>
void Discretizer<ImageListener,Colorizer_>
    ::normalize_histogram()
{
    const unsigned long used_histogram_pixels = 
        non_background_pixels();
    if ( used_histogram_pixels == 0U )
        return;

    const unsigned int start = background_threshold;
    const LowDepth maxVal = (out_depth - start);

    double fractions[in_depth];
    double accum = 0;
    double power = histogram_power;

    bool histogram_has_changed = false;
    TransitionTable new_transition( in_depth );

    if ( power <= 1E-5 ) {
        accum = in_depth - start;
        for (unsigned int i = start; i < in_depth; i++)
            fractions[i] = (i+1) - start;
    } else
        for (unsigned int i = start; i < in_depth; i++) {
            if (histogram[i] != 0)
                accum += pow( 
                    double(histogram[i]) / used_histogram_pixels, power );
            fractions[i] = accum;
        }

    for (unsigned int i = 0; i < start; i++)
        new_transition[i] = i;

    for (unsigned int i = start; i < in_depth; i++) {
        double q = (fractions[i] / accum);
        LowDepth newValue = std::min<int>(
            std::max<int>(maxVal*q, 0)+start, out_depth);
        const LowDepth& oldValue = transition[i];
        
        if ( abs( int(newValue) - int(oldValue) ) > 5 ) {
            this->publish().changeBrightness( i );
            new_transition[i] = newValue;
            histogram_has_changed = true;
        } else
            new_transition[i] = oldValue;
    }

    if ( histogram_has_changed ) {
        publish_differences_in_transitions(&transition, new_transition);
        std::swap( transition, new_transition );
    }
}

template <typename ImageListener, typename Colorizer_>
void Discretizer<ImageListener,Colorizer_>
::clear()
{
    this->publish().clear();
    colorizer.clear();

    for (unsigned int i = 0; i < in_depth; i++)
        histogram[i] = (i == 0) ? total_pixel_count : 0;

    pixels_above_used_max_value = 0;
    max_value = max_value_used_for_disc_factor;
}

template <typename ImageListener, typename Colorizer_>
void Discretizer<ImageListener,Colorizer_>
::setHistogramPower(float power) 
{
    this->histogram_power = power;
    normalize_histogram();
    publish_differences_in_transitions( NULL, transition );
}

}
}

#endif
