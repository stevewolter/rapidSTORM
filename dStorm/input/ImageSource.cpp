#include <dStorm/input/Source_impl.h>
#include <dStorm/input/ImageTraits.h>
#include <CImg.h>

namespace CImgBuffer {
template class Source< cimg_library::CImg<unsigned int> >;
template class Source< cimg_library::CImg<unsigned short> >;
template class Source< cimg_library::CImg<unsigned char> >;
template class Source< cimg_library::CImg<float> >;
}
