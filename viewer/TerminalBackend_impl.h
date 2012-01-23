#ifndef DSTORM_VIEWER_TERMINALBACKEND_IMPL_H
#define DSTORM_VIEWER_TERMINALBACKEND_IMPL_H

#include "debug.h"
#include "TerminalBackend.h"

#include <dStorm/outputs/BinnedLocalizations_impl.h>
#include "ImageDiscretizer_inline.h"
#include "Display_inline.h"

#include <dStorm/display/Manager.h>
#include <dStorm/image/iterator.h>

#include "Config.h"
#include "Status.h"

namespace dStorm {
namespace viewer {

template <typename Hueing>
TerminalBackend<Hueing>::TerminalBackend(const Colorizer& col, Status& status)
: image( status.config.binned_dimensions.make(), status.config.crop_border() ),
  colorizer(col),
  discretization( 4096, 
        status.config.histogramPower(), image(),
        colorizer),
  cache(),
  status(status)
{
    image.setListener(&discretization);
    discretization.setListener(&cache);
}

template <typename Hueing>
TerminalBackend<Hueing>::~TerminalBackend() {
    DEBUG("Destructing viewer implementation");
}

template <typename Hueing>
void TerminalBackend<Hueing>::save_image(
    std::string filename, const Config& config
)
{ 
    DEBUG("Storing results");
    std::auto_ptr<display::Change> result
        = get_result(config.save_with_key());
    if ( ! config.save_scale_bar() )
        for (int i = 0; i < Im::Dim; ++i)
            result->resize_image.pixel_sizes[i].value = -1 / camera::pixel;

    status.manager->store_image( filename, *result);
    DEBUG("Finished");
}

template <typename Hueing>
void TerminalBackend<Hueing>::set_histogram_power(float power) {
    discretization.setHistogramPower( power ); 
}

template <typename Hueing>
std::auto_ptr<display::Change> 
TerminalBackend<Hueing>::get_result(bool with_key) const {
    DEBUG("Getting results");
    std::auto_ptr<display::Change> c = cache.get_result(colorizer);
    c->do_clear = true;
    c->clear_image.background = colorizer.get_background();
    display::Image& im = c->image_change.new_image;
    DEBUG("Writing result image of size " << im.sizes());
    for ( display::Image::iterator i = im.begin(); i != im.end(); i++ ) {
        *i = discretization.get_pixel( i.position().head<Im::Dim>() );
    }
    if ( with_key && Colorizer::KeyCount > 0 ) {
        DEBUG("Creating keys");
        assert( int(c->changed_keys.size()) >= Colorizer::KeyCount );
        display::Change::Keys::iterator key = c->changed_keys.begin();
        int key_count = Colorizer::BrightnessDepth;
        key->reserve( key_count + key->size() );
        for (int i = 0; i < key_count; i++)
            key->push_back( display::KeyChange( i, 
                colorizer.getKeyPixel( i ), discretization.key_value( i ) ) );
        int index = 1;
        for ( ++key ; key != c->changed_keys.end(); ++key ) {
            key->clear();
            colorizer.create_full_key( *key, index++ );
        }
        DEBUG("Created keys");
    } else {
        c->changed_keys.clear();
        c->resize_image.keys.clear();
    }


    return c;
}

}
}

#endif
