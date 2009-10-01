#include "Input.h"
#include <CImgBuffer/Source.h>
#include <CImgBuffer/ImageTraits.h>
#include <CImg.h>
#include <CImgBuffer/Buffer_impl.h>
#include <CImgBuffer/Slot_impl.h>

template class CImgBuffer::Buffer<dStorm::Image>;
template class CImgBuffer::Slot<dStorm::Image>;

namespace dStorm {


Input::Input(std::auto_ptr<CImgBuffer::Source<Image> > c)

: CImgBuffer::Buffer< Image >(c) {}

}
