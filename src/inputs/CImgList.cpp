#include "config.h"
#ifdef HAVE_TIFFIO_H
#define cimg_use_tiff
#endif

#include "CImgList.h"

namespace dStorm { 
namespace CImgList {

template <typename PixelType>
Source<PixelType>::Source(const char *src )
: simparm::Object("CImgListSource", "Images"),
  input::Source< cimg_library::CImg<PixelType> >
    (*this, input::BaseSource::Pushing | input::BaseSource::Pullable),
  sourceImages(src)
{
    if ( sourceImages.is_empty() )
        throw std::runtime_error("Image file is empty.");
}

template <> std::string ident<uint8_t>() { return "IU8"; }
template class Source<uint8_t>;

template <> std::string ident<uint32_t>() { return "IU32"; }
template class Source<uint32_t>;

template <> std::string ident<float>() { return "F"; }
template class Source<float>;

template <> std::string ident<uint16_t>() { return "IU16"; }
template class Source<uint16_t>;

}}
