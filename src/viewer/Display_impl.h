#include "Display.h"
#include "Display_inline.h"
#include <boost/units/io.hpp>

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
    const input::Traits< cimg_library::CImg<int> >& traits
)
{ 
    int width = traits.size.x() / cs_units::camera::pixel,
        height = traits.size.y() / cs_units::camera::pixel;
    ps.resize( width * height, false );
    ps_step = width;

    dStorm::Display::ResizeChange r;
    r.width = width;
    r.height = height;
    r.key_size = Colorizer::BrightnessDepth;
    r.pixel_size = 
        traits.resolution.start<2>().sum().value() / 2;
    nm_per_pixel = r.pixel_size * 1E9;

    if ( do_show_window ) {
        props.initial_size = r;
        window_id = dStorm::Display::Manager::getSingleton()
                .register_data_source( props, vph );
        do_show_window = false;
    } else {
        next_change->do_resize = true;
        next_change->resize_image = r;
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

}
}
