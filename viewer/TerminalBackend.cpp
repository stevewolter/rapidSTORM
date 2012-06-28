#include "TerminalBackend.h"
#include "ColourScheme.h"
#include "Status_decl.h"

#include "TerminalBackend.h"

#include <dStorm/outputs/BinnedLocalizations_impl.h>
#include "ImageDiscretizer_inline.h"
#include "Display_inline.h"
#include "density_map/LinearInterpolation.h"

#include <dStorm/display/store_image.h>
#include <dStorm/image/iterator.h>

#include "Config.h"
#include "Status.h"

#include "LiveBackend_converter.h"

namespace dStorm {
namespace viewer {

TerminalBackend::TerminalBackend(std::auto_ptr<Colorizer> col, Status& status)
: image( NULL, status.config.binned_dimensions.make(), status.config.interpolator.make(), status.config.crop_border() ),
  colorizer(col),
  discretization( 4096, 
        status.config.histogramPower(), image(),
        *colorizer),
  cache(),
  status(status)
{
    image.set_listener(&discretization);
    discretization.setListener(&cache);
}

TerminalBackend::~TerminalBackend() {
}

std::auto_ptr< display::Change > TerminalBackend::get_state() const
{
    std::auto_ptr<display::Change> rv
        ( new display::Change(ColourScheme::KeyCount) );

    display::ResizeChange size = cache.getSize();
    size.keys.clear();
    size.keys.push_back( display::KeyDeclaration("ADC", "total A/D counts per pixel", colorizer->brightness_depth()) );
    rv->do_resize = true;
    rv->resize_image = size;
    rv->do_change_image = true;
    rv->image_change.new_image = display::Image(size.size);

    if ( int(size.keys.size()) < ColourScheme::KeyCount ) {
        for (int j = 1; j < ColourScheme::KeyCount; ++j ) {
            rv->resize_image.keys.push_back( colorizer->create_key_declaration( j ) );
        }
    }
    return rv;
}

void TerminalBackend::save_image(
    std::string filename, const Config& config
)
{ 
    image.store_results(true);
    std::auto_ptr<display::Change> rv = get_result( config.save_with_key() );
    if ( ! config.save_scale_bar() )
        for (int i = 0; i < Im::Dim; ++i)
            rv->resize_image.pixel_sizes[i].value = -1 / camera::pixel;

    display::StorableImage i( filename, *rv );
    i.scale_bar = quantity<si::length>(config.scale_bar_length());
    store_image(i);
}

void TerminalBackend::set_histogram_power(float power) {
    discretization.setHistogramPower( power ); 
}

void TerminalBackend::set_top_cutoff(float cutoff) {
    discretization.set_top_cutoff( cutoff ); 
}

std::auto_ptr<display::Change> 
TerminalBackend::get_result(bool with_key) const {
    std::auto_ptr<display::Change> c = get_state();
    c->do_clear = true;
    c->clear_image.background = colorizer->get_background();
    display::Image& im = c->image_change.new_image;
    for ( display::Image::iterator i = im.begin(); i != im.end(); i++ ) {
        *i = discretization.get_pixel( i.position().head<Im::Dim>() );
    }
    if ( with_key && Colorizer::KeyCount > 0 ) {
        assert( int(c->changed_keys.size()) >= Colorizer::KeyCount );
        display::Change::Keys::iterator key = c->changed_keys.begin();
        int key_count = colorizer->brightness_depth();
        key->reserve( key_count + key->size() );
        for (int i = 0; i < key_count; i++)
            key->push_back( display::KeyChange( i, 
                colorizer->getKeyPixel( i ), discretization.key_value( i ) ) );
        int index = 1;
        for ( ++key ; key != c->changed_keys.end(); ++key ) {
            key->clear();
            colorizer->create_full_key( *key, index++ );
        }
    } else {
        c->changed_keys.clear();
        c->resize_image.keys.clear();
    }


    return c;
}

std::auto_ptr<Backend>
TerminalBackend::change_liveness( Status& s ) {
    if ( s.config.showOutput() ) {
        return std::auto_ptr<Backend>( new LiveBackend<ColourScheme>(*this, s) );
    } else {
        return std::auto_ptr<Backend>();
    }
}


}
}
