#ifndef DSTORM_WXDISPLAY_SIZECONV_H
#define DSTORM_WXDISPLAY_SIZECONV_H

#include "display/DataSource.h"
#include <wx/font.h>

namespace simparm {
namespace wx_ui {
namespace image_window {

namespace camera = boost::units::camera;

inline wxSize mkWxSize( const dStorm::display::Image::Size& s ) 
{
    return wxSize( s.x() / camera::pixel,
                   s.y() / camera::pixel );
}
inline dStorm::display::Image::Size mkImgSize( const wxSize& s ) 
{
    dStorm::display::Image::Size rv;
    rv.fill( 1 * camera::pixel );
    rv.x() = s.GetWidth() * camera::pixel,
    rv.y() = s.GetHeight() * camera::pixel;
    return rv;
}

}
}
}

#endif
