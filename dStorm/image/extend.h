#ifndef LIBDSTORM_IMAGE_EXTEND_H
#define LIBDSTORM_IMAGE_EXTEND_H

#include "dStorm/image/constructors.h"

namespace dStorm {

template <typename PixelType, int OrigDim, int NewDim>
inline Image<PixelType, NewDim> 
extend( const Image<PixelType, OrigDim>& orig, const Image<PixelType,NewDim>& )
{
    typedef Image<PixelType, NewDim> New;
    typename New::Size nsz = New::Size::Constant( 1 * boost::units::camera::pixel );
    nsz.template head< OrigDim >() = orig.sizes();
    typename New::Offsets no = New::Offsets::Constant( orig.size_in_pixels() );
    no.template head< OrigDim >() = orig.get_offsets();
    New rv( nsz, orig.get_data_reference(), no, orig.get_global_offset(), orig.frame_number() );
    return rv;
}

}

#endif
