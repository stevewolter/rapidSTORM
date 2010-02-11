#include "FileBasedMethod_impl.h"
#include <CImg.h>

namespace dStorm {
namespace input {

template class FileBasedMethod< cimg_library::CImg<unsigned char> >;
template class FileBasedMethod< cimg_library::CImg<unsigned short> >;
template class FileBasedMethod< cimg_library::CImg<unsigned int> >;
template class FileBasedMethod< cimg_library::CImg<float> >;
template class FileBasedMethod< cimg_library::CImg<double> >;

}
}
