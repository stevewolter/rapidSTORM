#ifndef DSTORM_VIEWER_TERMINALBACKEND_IMPL_H
#define DSTORM_VIEWER_TERMINALBACKEND_IMPL_H

#include "debug.h"
#include "TerminalBackend.h"

#include <dStorm/outputs/BinnedLocalizations_impl.h>
#include "ColourDisplay_impl.h"
#include "ImageDiscretizer_inline.h"
#include "Display_inline.h"

#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/image/iterator.h>

namespace dStorm {
namespace viewer {

template <int Hueing>
TerminalBackend<Hueing>::TerminalBackend(const Config& config)
: image( config.res_enh(), config.border() ),
  colorizer(config),
  discretization( 4096, 
        config.histogramPower(), image(),
        colorizer),
  cache()
{
    image.setListener(&discretization);
    discretization.setListener(&cache);
}

template <int Hueing>
TerminalBackend<Hueing>::~TerminalBackend() {
    DEBUG("Destructing viewer implementation");
}

template <int Hueing>
void TerminalBackend<Hueing>::save_image(
    std::string filename, const Config& config
)
{ 
    DEBUG("Storing results");
    std::auto_ptr<dStorm::Display::Change> result
        = get_result(config.save_with_key());
    if ( ! config.save_scale_bar() )
        result->resize_image.pixel_size = 
            -1 * cs_units::camera::pixels_per_meter;

    dStorm::Display::Manager::getSingleton().store_image( 
        filename, *result);
    DEBUG("Finished");
}

template <int Hueing>
void TerminalBackend<Hueing>::set_histogram_power(float power) {
    /* The \c image member is not involved here, so we have to lock
        * it ourselves. */
    ost::MutexLock lock( image.getMutex() );
    discretization.setHistogramPower( power ); 
}

template <int Hueing>
void TerminalBackend<Hueing>::set_resolution_enhancement(float re)  { 
    image.set_resolution_enhancement( re ); 
}

template <int Hueing>
std::auto_ptr<dStorm::Display::Change> 
TerminalBackend<Hueing>::get_result(bool with_key) const {
    DEBUG("Getting results");
    std::auto_ptr<dStorm::Display::Change> c = cache.get_result(colorizer);
    c->do_clear = true;
    c->clear_image.background = colorizer.get_background();
    Im& im = c->image_change.new_image;
    DEBUG("Writing result image of size " << im.width_in_pixels() << " " << im.height_in_pixels());
    for ( Im::iterator i = im.begin(); i != im.end(); i++ ) {
        *i = discretization.get_pixel( i.x(), i.y() );
    }
    if ( with_key && Colorizer::KeyCount > 0 ) {
        DEBUG("Creating keys");
        assert( int(c->changed_keys.size()) >= Colorizer::KeyCount );
        dStorm::Display::Change::Keys::iterator key = c->changed_keys.begin();
        int key_count = Colorizer::BrightnessDepth;
        dStorm::Display::KeyChange* k = key->allocate(key_count);
        for (int i = 0; i < key_count; i++) {
            k->index = i;
            k->color = colorizer.getKeyPixel( i );
            k->value = discretization.key_value( i );
            k++;
        }
        key->commit( key_count );
        int index = 1;
        for ( ++key ; key != c->changed_keys.end(); ++key ) {
            key->clear();
            colorizer.create_full_key( *key, index++ );
        }
        DEBUG("Created keys");
    }


    return c;
}

}
}

#endif
