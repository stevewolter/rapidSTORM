#include "Display.h"
#include "Display_inline.h"
#include <boost/units/io.hpp>
#include <dStorm/output/Traits.h>
#include <dStorm/ImageTraits.h>

namespace dStorm {
namespace viewer {

template <typename Colorizer>
Display<Colorizer>::Display( 
    Discretizer& disc, 
    const Viewer::_Config& config,
    dStorm::Display::DataSource& vph 
) 
: discretizer(disc), 
  do_show_window( config.showOutput() ),
  vph(vph), 
  next_change( new dStorm::Display::Change() )
{
    props.name = config.getDesc();
    props.flags.close_window_on_unregister
        ( config.close_on_completion() );
}

template <typename Colorizer>
void Display<Colorizer>::setSize(
    const input::Traits< Image<int,2> >& traits
)
{ 
    my_size.size = traits.size;
    ps.resize( traits.size.x() * traits.size.y()
        / cs_units::camera::pixel / cs_units::camera::pixel, false );
    ps_step = traits.size.x() / cs_units::camera::pixel;
    if ( ! traits.resolution.is_set() )
        throw std::logic_error("Pixel size must be given for image display");

    my_size.key_size = Colorizer::BrightnessDepth;
    my_size.pixel_size = *traits.resolution;

    if ( do_show_window ) {
        props.initial_size = my_size;
        window_id = dStorm::Display::Manager::getSingleton()
                .register_data_source( props, vph );
        do_show_window = false;
    } else {
        next_change->do_resize = true;
        next_change->resize_image = my_size;
    }
    this->clear();
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
    std::string filename, bool with_key)
{
    if ( window_id.get() == NULL ) {
        throw std::runtime_error(
            "Saving images after closing the "
            "display window not supported yet.");
    } else {
        std::auto_ptr<dStorm::Display::Change>
            c = window_id->get_state();
        if ( ! with_key )
            c->change_key.clear();
        if ( c.get() != NULL )
            dStorm::Display::Manager::getSingleton()
                .store_image( filename, *c );
        else
            throw std::runtime_error(
                "Could not get displayed image "
                "from window");
    }

}

}
}
