#include "Config.h"
#include "Display.h"
#include "Display_inline.h"
#include <boost/units/io.hpp>
#include <dStorm/output/Traits.h>
#include <dStorm/ImageTraits.h>
#include "Status.h"

namespace dStorm {
namespace viewer {

template <typename Colorizer>
Display<Colorizer>::Display( 
    MyDiscretizer& disc, 
    const Config& config,
    dStorm::display::DataSource& vph ,
    const Colorizer& colorizer,
    std::auto_ptr<dStorm::display::Change> initial_state
) 
: discretizer(disc), 
  colorizer(colorizer),
  vph(vph), 
  next_change( initial_state )
{
    if ( config.close_on_completion() )
        props.flags.close_window_on_unregister();
    if ( next_change.get() == NULL )
        next_change.reset( new dStorm::display::Change(Colorizer::KeyCount) );
    else
        setSize( next_change->resize_image );

}

template <typename Colorizer>
void Display<Colorizer>::show_window()
{
    if ( window_id.get() == NULL && my_size.is_initialized() ) {
        props.initial_size = *my_size;
        window_id = dStorm::display::Manager::getSingleton()
                .register_data_source( props, vph );
    }
}

template <typename Colorizer>
void Display<Colorizer>::setSize(
    const dStorm::display::ResizeChange& size
) {
    my_size = size;
    BaseDisplay::setSize( size.size );
    this->clear();
}

template <typename Colorizer>
void Display<Colorizer>::setSize(
    const input::Traits< Image<int,2> >& traits
)
{ 
    dStorm::display::ResizeChange size;
    size.size = traits.size;

    size.keys.push_back(
        dStorm::display::KeyDeclaration("ADC", "total A/D counts", Colorizer::BrightnessDepth) );
    for (int j = 1; j < Colorizer::KeyCount; ++j) {
        size.keys.push_back( colorizer.create_key_declaration(j) );
        colorizer.create_full_key( next_change->changed_keys[j] , j );
    }
    for (int i = 0; i < 2; ++i) 
        if ( traits.resolution(i).is_initialized() )
            size.pixel_sizes[i] = *traits.resolution(i);
        else
            size.pixel_sizes[i].value = -1 / camera::pixel;

    setSize( size );

    if ( window_id.get() == NULL ) {
        show_window();
    } else {
        next_change->do_resize = true;
        next_change->resize_image = *my_size;
    }
}

template <typename Colorizer>
void Display<Colorizer>::clean(bool) {
    typedef dStorm::display::Change::PixelQueue PixQ;
    const PixQ::iterator end 
        = next_change->change_pixels.end();
    for ( PixQ::iterator i = 
        next_change->change_pixels.begin(); i != end; ++i )
    {
        i->color = discretizer.get_pixel(*i);
        is_on(*i) = false;
    }
}

template <typename Colorizer>
void Display<Colorizer>::clear() {
    dStorm::display::ClearChange &c = next_change->clear_image;
    c.background = discretizer.get_background();
    next_change->do_clear = true;

    next_change->change_pixels.clear();
    next_change->changed_keys.front().clear();
    for (int j = 1; j < Colorizer::KeyCount; ++j)
        colorizer.create_full_key( next_change->changed_keys[j] , j );
    BaseDisplay::clear();
}

template <typename Colorizer>
std::auto_ptr<dStorm::display::Change>
Display<Colorizer>::get_changes() {
    std::auto_ptr<dStorm::display::Change> fresh
        ( new dStorm::display::Change(Colorizer::KeyCount) );
    clean(false);
    std::swap( fresh, next_change );
    return fresh;
}

struct KeyClearer : public std::unary_function<void,dStorm::display::Change&> {
    KeyClearer( const Config& c ) 
        : clear_key( ! c.save_with_key() ), clear_scale_bar( ! c.save_scale_bar() ) {}
    void operator()( dStorm::display::Change& c ) {
        if (clear_key) {
            c.changed_keys.clear();
            c.resize_image.keys.clear();
        }
        if (clear_scale_bar) {
            c.resize_image.pixel_sizes[0].value = -1 / camera::pixel;
            c.resize_image.pixel_sizes[1].value = -1 / camera::pixel;
        }
    }

  private:
    const bool clear_key, clear_scale_bar;
};

template <typename Colorizer>
void
Display<Colorizer>::save_image(
    std::string filename, const Config& config)
{
    if ( window_id.get() == NULL ) {
        throw std::runtime_error(
            "Saving images after closing the "
            "display window not supported yet.");
    } else {
        window_id->store_current_display( filename, KeyClearer(config) );
    }

}

}
}
