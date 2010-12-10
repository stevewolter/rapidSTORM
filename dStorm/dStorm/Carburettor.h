#ifndef DSTORM_CARBURETTOR_H
#define DSTORM_CARBURETTOR_H

#include <CImgBuffer/Buffer.h>

namespace CImgBuffer {
    template <typename Pixel> class Image;
}

namespace dStorm {
    /** The Carburettor is the class that provides image locking for
     *  the Engine. For now, it is simply a wrapper around the CImgBuffer
     *  class. */
    class Carburettor 
        : public CImgBuffer::Buffer< CImgBuffer::Image<unsigned short> > 
    {
      public:
        Carburettor(const CImgBuffer::Config< 
                                    CImgBuffer::Image<unsigned short> >&c)
            throw(std::exception) 
        : CImgBuffer::Buffer< CImgBuffer::Image<unsigned short> >(c) {}
    };
}

#endif
