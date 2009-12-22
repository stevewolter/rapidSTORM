#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H

#include "ImageDiscretizer.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm {
namespace output {
namespace DiscretizedImage {

template <typename Colorizer, typename ImageListener>
const typename ImageDiscretizer<Colorizer, ImageListener>::HighDepth
ImageDiscretizer<Colorizer, ImageListener>::background_threshold = 1;

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
    input::Traits< cimg_library::CImg<int> > out_traits( traits );
    this->publish().setSize( out_traits );
    total_pixel_count = traits.size.x() * traits.size.y();

    cimg_library::CImg<HistogramPixel>
        new_pixel_map(traits.size.x(), traits.size.y());
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
::change( int x, int y, HighDepth to )
{
    this->publish().pixelChanged( x, y );

    pixels_by_value[to]
        .push_back( pixels_by_position(x,y) );
}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
::updatePixel(int x, int y, float from, float to) 
{
    colorizer.updatePixel( x, y, from, to );

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
        change( x, y, n );
    }
}

template <typename Colorizer, typename ImageListener>
inline unsigned long int ImageDiscretizer<Colorizer, ImageListener>::non_background_pixels()
{
    long int accum = 0;
    for (unsigned int i = 0; i < background_threshold; i++)
        accum += histogram[i];
    return pixels_by_position.size() - accum;
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

template <typename Colorizer, typename ImageListener>
float 
ImageDiscretizer<Colorizer, ImageListener>::key_value( LowDepth key )
{
    int n = -1; 
    while ( transition[n+1] <= key ) n++;
    return (n+0.5f) / disc_factor;
}

#ifdef HAVE_LIBGRAPHICSMAGICK__
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
            typename Colorizer::Pixel p 
                = colorizer.getPixel(x, y, 
                     transition[ discretize(binned_pixels[x]) ] );
            pixels[x].red = p.r;
            pixels[x].green = p.g;
            pixels[x].blue = p.b;
        }
        to_image.syncPixels();
    }
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
    static const int major_shift = 
        (( QuantumDepth == 16 ) ? 8 : 0);
    for (LowDepth i = 0; i < out_depth; i++) {
        typename Colorizer::Pixel p
            = colorizer.getKeyPixel(i);
        pixels[i]
            = Magick::Color(
                (p.r << major_shift) | p.r,
                (p.g << major_shift) | p.g,
                (p.b << major_shift) | p.b );
    }

    rv->syncPixels();
    return rv;
}
#endif

}
}
}

#endif
