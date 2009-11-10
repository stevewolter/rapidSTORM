#include "SerialSource_impl.h"
#include "ImageTraits.h"
#include <CImg.h>

namespace dStorm {
namespace input {

template class SerialSource< cimg_library::CImg<unsigned char> >;
template class SerialSource< cimg_library::CImg<unsigned short> >;
template class SerialSource< cimg_library::CImg<unsigned int> >;
template class SerialSource< cimg_library::CImg<float> >;
template class SerialSource< cimg_library::CImg<double> >;

}
}
