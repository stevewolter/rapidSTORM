#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_IMPL_H

namespace dStorm {
namespace DiscretizedImage {

template <typename Colorizer, typename ImageListener>
ImageDiscretizer<Colorizer, ImageListener>
::ImageDiscretizer(int d, int bg_th, float hp,
    const cimg_library::CImg<float>& binned_image,
    Colorizer& colorizer) 
: colorizer(colorizer),
  max_value(10), 
  max_value_used_for_disc_factor(10),
  disc_factor( (d-1) * 1.0 / max_value_used_for_disc_factor ),
  histogram( d, 0 ),
  transition( d, 0 ),
  pixels_by_value( d ),
  in_depth( d ),
  out_depth( Colorizer::BrightnessDepth - 1 ),
  background_threshold( bg_th ),
  histogram_power( hp ),
  binned_image(binned_image)
{
}

template <typename Colorizer, typename ImageListener>
ImageDiscretizer<Colorizer, ImageListener>
::~ImageDiscretizer() {}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
::setSize(int width, int height)
{
    colorizer.setSize( width, height );
    this->publish().setSize( width, height );
    total_pixel_count = width * height;

    cimg_library::CImg<HistogramPixel>
        new_pixel_map(width, height);
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
void ImageDiscretizer<Colorizer, ImageListener>
  ::clean()
{
    if ( pixels_above_used_max_value >
           non_background_pixel_count / 100 )
    {
        non_background_pixel_count = 0;
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
            if ( n >= background_threshold )
                non_background_pixel_count ++;
        }

        disc_factor = new_disc_fac;
        max_value_used_for_disc_factor = max_value;
        pixels_above_used_max_value = 0;
    }

    normalize_histogram();

    colorizer.clean();
    this->publish().clean();
}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
    ::normalize_histogram()
{
    const unsigned long used_histogram_pixels = 
        non_background_pixel_count;
    if ( used_histogram_pixels == 0U )
        return;

    const unsigned int start = background_threshold;
    const LowDepth maxVal = (out_depth - start);

    double fractions[in_depth];
    double accum = 0;
    float power = histogram_power;

    bool histogram_has_changed = false;
    TransitionTable new_transition( in_depth );

    for (unsigned int i = start; i < in_depth; i++) {
        if (histogram[i] != 0)
            accum += pow( 
                double(histogram[i]) / used_histogram_pixels, 
                double(power) );
        fractions[i] = accum;
    }

    for (unsigned int i = 0; i < start; i++)
        new_transition[i] = i;

    int count = 0;
    for (unsigned int i = start; i < in_depth; i++) {
        double q = (fractions[i] / accum);
        LowDepth newValue = std::min<int>(
            std::max<int>(maxVal*q, 0)+start, out_depth);
        const LowDepth& oldValue = transition[i];
        
        if ( ! pixels_by_value[i].empty() ) {
            if ( abs( int(newValue) - int(oldValue) ) > 5 ) {
                for ( HistogramPixel* j = pixels_by_value[i].next; 
                                j != &pixels_by_value[i]; j = j->next)
                {
                    this->publish().pixelChanged( j->x, j->y );
                    count++;
                }
                new_transition[i] = newValue;
                histogram_has_changed = true;
            } else
                new_transition[i] = oldValue;
        } else {
            new_transition[i] = newValue;
            histogram_has_changed = true;
        }
    }

    if ( histogram_has_changed ) {
        publish_differences_in_transitions( transition, new_transition );
        std::swap( transition, new_transition );
    }
}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
  ::publish_differences_in_transitions
  ( TransitionTable& old_table, TransitionTable& new_table )
{
    HighDepth o = 0, n = 0;
    for (LowDepth v = 0; v < out_depth; v++) {
        while ( (o+1U) < in_depth && old_table[o+1U] <= v ) o++;
        while ( (n+1U) < in_depth && new_table[n+1U] <= v ) n++;

        if ( o != n ) {
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

    non_background_pixel_count = 0;
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
}

template <typename Colorizer, typename ImageListener>
std::auto_ptr< cimg_library::CImg<uint8_t> >
ImageDiscretizer<Colorizer, ImageListener>::full_image()
{
    std::auto_ptr<cimg_library::CImg<uint8_t> > rv
        ( new cimg_library::CImg<uint8_t>(
            binned_image.width,
            binned_image.height,
            1, 3) );

    cimg_forXY( binned_image, x, y ) {
        typename Colorizer::Pixel p = get_pixel(x,y);
        (*rv)(x,y,0,0) = p.r;
        (*rv)(x,y,0,1) = p.g;
        (*rv)(x,y,0,2) = p.b;
    }

    return rv;
}

}
}

#endif
