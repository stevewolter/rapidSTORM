#ifndef DSTORM_WXDISPLAY_SIZECONV_H
#define DSTORM_WXDISPLAY_SIZECONV_H

#include <dStorm/helpers/DisplayDataSource.h>
#include <wx/font.h>

namespace dStorm {
namespace Display {

inline wxSize mkWxSize( const dStorm::Display::Image::Size& s ) 
{
    return wxSize( s.x() / cs_units::camera::pixel,
                   s.y() / cs_units::camera::pixel );
}
inline dStorm::Display::Image::Size mkImgSize( const wxSize& s ) 
{
    dStorm::Display::Image::Size rv;
    rv.x() = s.GetWidth() * cs_units::camera::pixel,
    rv.y() = s.GetHeight() * cs_units::camera::pixel;
    return rv;
}

}
}

#endif
