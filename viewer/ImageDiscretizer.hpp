#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H

#include <boost/units/io.hpp>
#include <dStorm/image/MetaInfo.h>
#include "dStorm/image/iterator.h"
#include "dStorm/image/Image.hpp"
#include <algorithm>

#include "viewer/ImageDiscretizer.h"

namespace dStorm {
namespace viewer {

template <typename ImageListener>
Discretizer<ImageListener>
::Discretizer(int d, float hp,
    const Image<float,Im::Dim>& binned_image,
    ColourScheme& colorizer) 
: colorizer(colorizer),
  max_value(10), 
  max_value_used_for_disc_factor(0.1),
  disc_factor( (d-1) * 1.0 / max_value_used_for_disc_factor ),
  histogram( d, 0 ),
  transition( d, 0 ),
  in_depth( d ),
  out_depth( colorizer.brightness_depth() - 1 ),
  pixels_above_used_max_value(0),
  histogram_power( hp ),
  cutoff_factor( 1 ),
  binned_image(binned_image)
{
}

template <typename ImageListener>
Discretizer<ImageListener>
::~Discretizer() {}

template <typename ImageListener>
void Discretizer<ImageListener>
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

template <typename ImageListener>
void Discretizer<ImageListener>
  ::clean(bool final)
{
    if ( final || pixels_above_used_max_value >
           non_background_pixels() / 100 )
    {
        rediscretize();
        publish_differences_in_transitions( NULL, transition );
    }
    normalize_histogram();
    colorizer.clean(final);
    this->publish().clean(final);
}

template <typename ImageListener>
void Discretizer<ImageListener>::rediscretize()
{
    for (size_t i = 0; i < histogram.size(); i++)
        histogram[i] = 0;

    float new_disc_fac = (in_depth-1) * 1.0 / (max_value * std::max( cutoff_factor, 1E-10f ) );

    for( InputImage::const_iterator i = binned_image.begin();
        i != binned_image.end(); i++)
    {
        HighDepth n = discretize( *i, new_disc_fac );

        if ( discretize( *i ) != n )
            this->publish().pixelChanged( i.position(), n );

        ++histogram[n];
    }

    disc_factor = new_disc_fac;
    max_value_used_for_disc_factor = max_value;
    pixels_above_used_max_value = 0;
}

template <typename ImageListener>
void Discretizer<ImageListener>
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

template <typename ImageListener>
void Discretizer<ImageListener>
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

template <typename ImageListener>
void Discretizer<ImageListener>
::clear()
{
    this->publish().clear();
    colorizer.clear();

    for (unsigned int i = 0; i < in_depth; i++)
        histogram[i] = (i == 0) ? total_pixel_count : 0;

    pixels_above_used_max_value = 0;
    max_value = max_value_used_for_disc_factor;
}

template <typename ImageListener>
void Discretizer<ImageListener>
::setHistogramPower(float power) 
{
    this->histogram_power = power;
    normalize_histogram();
    publish_differences_in_transitions( NULL, transition );
}

template <typename ImageListener>
void Discretizer<ImageListener>
::set_top_cutoff(float cutoff) 
{
    this->cutoff_factor = cutoff;
    rediscretize();
    normalize_histogram();
    publish_differences_in_transitions( NULL, transition );
}

template < typename ImageListener>
void Discretizer< ImageListener>
::updatePixel(const Im::Position& p, float from, float to) 
{
    colorizer.updatePixel( p, from, to );

    if ( ImageListener::NeedLiveHistogram ) {
        if ( to > max_value_used_for_disc_factor ) {
            if ( from <= max_value_used_for_disc_factor )
                ++pixels_above_used_max_value;
            max_value = std::max( to, max_value );
        }

        HighDepth o = discretize( from ),
                n = discretize( to );
        
        if ( o != n ) {
            ++histogram[ n ];
            if ( histogram[o] > 0U )
                --histogram[o];
            this->publish().pixelChanged( p, n );
        }
    } else {
        max_value = std::max( max_value, to );
    }
}


}
}

#endif
