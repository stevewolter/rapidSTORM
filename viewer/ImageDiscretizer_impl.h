#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H

#include <boost/units/io.hpp>

#include "ImageDiscretizer_inline.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm {
namespace viewer {
namespace DiscretizedImage {

template <typename Colorizer, typename ImageListener>
ImageDiscretizer<Colorizer, ImageListener>
::ImageDiscretizer(int d, float hp,
    const cimg_library::CImg<float>& binned_image,
    Colorizer& colorizer) 
: colorizer(colorizer),
  max_value(10), 
  max_value_used_for_disc_factor(10),
  disc_factor( (d-1) * 1.0 / max_value_used_for_disc_factor ),
  histogram( d, 0 ),
  transition( d, 0 ),
  pixels_by_value( d, HistogramPixel() ),
  in_depth( d ),
  out_depth( Colorizer::BrightnessDepth - 1 ),
  histogram_power( hp ),
  binned_image(binned_image)
{
}

template <typename Colorizer, typename ImageListener>
ImageDiscretizer<Colorizer, ImageListener>
::~ImageDiscretizer() {}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
::setSize( const input::Traits<InputImage>& traits )
{
    colorizer.setSize( traits );
    this->publish().setSize( 
        input::Traits< cimg_library::CImg<int> >(traits) );
    total_pixel_count = 
        (traits.size.x() / cs_units::camera::pixel)
      * (traits.size.y() / cs_units::camera::pixel);

    cimg_library::CImg<HistogramPixel>
        new_pixel_map(traits.size.x().value(), traits.size.y().value());
    pixels_by_position.swap( new_pixel_map );

    cimg_forXY( pixels_by_position, x, y ) {
        HistogramPixel& p = pixels_by_position(x,y);
        p.clear();
        p.x = x;
        p.y = y;
    }

    for (unsigned int i = 0; i < in_depth; i++) {
        pixels_by_value[i].clear();
        histogram[0] = (i==0) ? total_pixel_count : 0;
    }
}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
  ::clean(bool final)
{
    if ( final || pixels_above_used_max_value >
           non_background_pixels() / 100 )
    {
        for (int i = 0; i < histogram.size(); i++)
            histogram[i] = 0;

        float new_disc_fac = (in_depth-1) * 1.0 / max_value;

        cimg_forXY( binned_image, x, y ) {
            float val = binned_image(x,y);
            HighDepth o = discretize( val ),
                      n = discretize( val, new_disc_fac );

            if ( o != n )
                change( x, y, n );

            ++histogram[n];
        }

        disc_factor = new_disc_fac;
        max_value_used_for_disc_factor = max_value;
        pixels_above_used_max_value = 0;
    }
    normalize_histogram();
    colorizer.clean(final);
    this->publish().clean(final);
}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
  ::publish_differences_in_transitions
  ( TransitionTable* old_table, TransitionTable& new_table )
{
    HighDepth o = 0, n = 0;
    for (LowDepth v = 0; v < out_depth; v++) {
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

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
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
    new_transition.allocate( in_depth );

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
        
        if ( ! pixels_by_value[i].empty() ) {
            if ( abs( int(newValue) - int(oldValue) ) > 5 ) {
                for ( HistogramPixel* j = pixels_by_value[i].next; 
                                j != &pixels_by_value[i]; j = j->next)
                    this->publish().pixelChanged( j->x, j->y );
                new_transition[i] = newValue;
                histogram_has_changed = true;
            } else
                new_transition[i] = oldValue;
        } else {
            new_transition[i] = newValue;
            histogram_has_changed = true;
        }
    }

    new_transition.commit( in_depth );

    if ( histogram_has_changed ) {
        publish_differences_in_transitions(&transition, new_transition);
        std::swap( transition, new_transition );
    }
}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
::clear()
{
    this->publish().clear();
    colorizer.clear();

    for (unsigned int i = 0; i < in_depth; i++) {
        pixels_by_value[i].unlink();
        histogram[i] = (i == 0) ? total_pixel_count : 0;
    }

    pixels_above_used_max_value = 0;
    max_value = max_value_used_for_disc_factor;

    cimg_for( pixels_by_position, p, HistogramPixel )
        p->clear();
}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
::setHistogramPower(float power) 
{
    this->histogram_power = power;
    normalize_histogram();
    publish_differences_in_transitions( NULL, transition );
}

#ifdef HAVE_LIBGRAPHICSMAGICK__
template <int MagickDepth>
inline void 
    make_magick_pixel( Magick::PixelPacket& mp, const dStorm::Pixel& p );

template <>
inline void 
    make_magick_pixel<8>( Magick::PixelPacket& mp,
                          const dStorm::Pixel& p )
{
    mp.red = p.red();
    mp.blue = p.blue();
    mp.green = p.green();
    mp.opacity = p.alpha();
}

template <>
inline void 
    make_magick_pixel<16>( Magick::PixelPacket& mp,
                           const dStorm::Pixel& p )
{
    mp.red = (p.red() | (p.red() << 8));
    mp.blue = (p.blue() | (p.blue() << 8));
    mp.green = (p.green() | (p.green() << 8));
    mp.opacity = (p.alpha() | (p.alpha() << 8));
}

template <typename Colorizer, typename ImageListener>
std::auto_ptr< Magick::Image >
ImageDiscretizer<Colorizer, ImageListener>::key_image()
{
    std::auto_ptr< Magick::Image > rv
        ( new Magick::Image(
            Magick::Geometry( out_depth, 1 ),
            Magick::ColorRGB( 0, 0, 0 ) ) );
    rv->type(Magick::TrueColorType);

    Magick::PixelPacket *pixels = 
        rv->getPixels(0, 0, out_depth, 1);
    for (LowDepth i = 0; i < out_depth; i++)
        make_magick_pixel<QuantumDepth>
            ( pixels[i], colorizer.getKeyPixel(i) );

    rv->syncPixels();
    return rv;
}

template <typename Colorizer, typename ImageListener>
void
ImageDiscretizer<Colorizer, ImageListener>::write_full_image
    (Magick::Image& to_image, int start_x, int start_y)
{
    int w = binned_image.width;
    cimg_forY( binned_image, y ) {
        const float *binned_pixels = binned_image.ptr(0,y);
        Magick::PixelPacket *pixels = to_image.setPixels
            ( start_x, start_y+y, w, 1 );
        cimg_forX( binned_image, x ) {
            Pixel p = colorizer.getPixel(x, y, 
                     transition[ discretize(binned_pixels[x]) ] );
            make_magick_pixel<QuantumDepth>( pixels[x], p );
        }
        to_image.syncPixels();
    }
}

#endif

}
}
}

#endif
