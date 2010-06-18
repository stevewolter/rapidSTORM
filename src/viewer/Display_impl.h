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
    dStorm::Display::DataSource& vph ,
    std::auto_ptr<dStorm::Display::Change> initial_state
) 
: discretizer(disc), 
  vph(vph), 
  next_change( initial_state )
{
    if ( config.close_on_completion() )
        props.flags.close_window_on_unregister();
    if ( next_change.get() == NULL )
        next_change.reset( new dStorm::Display::Change() );
    else
        setSize( next_change->resize_image );

}

template <typename Colorizer>
void Display<Colorizer>::show_window()
{
    props.initial_size = my_size;
    window_id = dStorm::Display::Manager::getSingleton()
            .register_data_source( props, vph );
}

template <typename Colorizer>
void Display<Colorizer>::setSize(
    const dStorm::Display::ResizeChange& size
) {
    my_size = size;

    ps.resize( my_size.size.x() * my_size.size.y()
        / cs_units::camera::pixel / cs_units::camera::pixel, false );
    ps_step = my_size.size.x() / cs_units::camera::pixel;

    this->clear();
}

template <typename Colorizer>
void Display<Colorizer>::setSize(
    const input::Traits< Image<int,2> >& traits
)
{ 
    dStorm::Display::ResizeChange size;
    size.size = traits.size;
    if ( ! traits.resolution.is_set() )
        throw std::logic_error("Pixel size must be given for image display");

    size.key_size = Colorizer::BrightnessDepth;
    size.pixel_size = *traits.resolution;

    setSize( size );

    if ( window_id.get() == NULL ) {
        show_window();
    } else {
        next_change->do_resize = true;
        next_change->resize_image = my_size;
    }
}

template <typename Colorizer>
void Display<Colorizer>::clean(bool) {
    typedef dStorm::Display::Change::PixelQueue PixQ;
    const PixQ::iterator end 
        = next_change->change_pixels.end();
    for ( PixQ::iterator i = 
        next_change->change_pixels.begin(); i != end; ++i )
    {
        i->color = discretizer.get_pixel(i->x, i->y);
        ps[ i->y * ps_step + i->x ] = false;
    }
}

template <typename Colorizer>
void Display<Colorizer>::clear() {
    dStorm::Display::ClearChange &c = next_change->clear_image;
    c.background = discretizer.get_background();
    next_change->do_clear = true;

    next_change->change_pixels.clear();
    next_change->change_key.clear();
    fill( ps.begin(), ps.end(), false );
}

template <typename Colorizer>
std::auto_ptr<dStorm::Display::Change>
Display<Colorizer>::get_changes() {
    std::auto_ptr<dStorm::Display::Change> fresh
        ( new dStorm::Display::Change() );
    std::swap( fresh, next_change );
    return fresh;
}

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
        std::auto_ptr<dStorm::Display::Change>
            c = window_id->get_state();
        if ( c.get() != NULL ) {
            if ( ! config.save_with_key() ) c->change_key.clear();
            if ( ! config.save_scale_bar() ) 
                c->resize_image.pixel_size = -1 * cs_units::camera::pixels_per_meter;
            dStorm::Display::Manager::getSingleton()
                .store_image( filename, *c );
        } else
            throw std::runtime_error(
                "Could not get displayed image "
                "from window");
    }

}

}
}
