#include "LocalizedImage.h"
#include <iostream>

namespace dStorm {
namespace output {
LocalizedImage::LocalizedImage() 
: forImage(0 * camera::frame), smoothed(NULL), candidates(NULL)
{
}

}
}
