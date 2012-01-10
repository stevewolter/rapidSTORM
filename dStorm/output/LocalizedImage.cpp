#include "LocalizedImage.h"
#include <iostream>

namespace dStorm {
namespace output {

LocalizedImage::LocalizedImage() 
: forImage(0 * camera::frame), smoothed(NULL), candidates(NULL)
{
}

LocalizedImage::LocalizedImage(frame_index i) 
: forImage(i), smoothed(NULL), candidates(NULL)
{
}

void LocalizedImage::set_frame_number( frame_index n ) 
{
    forImage = n;
    for ( iterator i = begin(); i != end(); ++i )
        i->frame_number() = n;
}

}
}
