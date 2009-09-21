#ifndef DSTORM_CARBURETTOR_H
#define DSTORM_CARBURETTOR_H

#include <dStorm/engine/Image.h>
#include <CImgBuffer/Buffer.h>

namespace dStorm {
    /** The Input is the class that provides image locking for
     *  the Engine. For now, it is simply a wrapper around the CImgBuffer
     *  class. */
    class Input 
        : public CImgBuffer::Buffer< Image > 
    {
      public:
        Input(std::auto_ptr<CImgBuffer::Source<Image> > c)
;
    };
}

#endif
