#ifndef CImgBuffer_DRAIN_H
#define CImgBuffer_DRAIN_H

namespace CImgBuffer { 
    template <class T> class Image;

    /** A Drain is a base class for all objects that accept images
     *  from a pushing Source. */
    template <class T> class Drain {
      public:
        /** Accept an image. 
         *  \param index Image index as labeled by source, numbered
         *               from 0.
         *  \param i     The image for that index. If this pointer
         *               is NULL, this is considered an error in the
         *               source. */
        virtual void accept(int index, std::auto_ptr< T > i)
            throw() = 0;
    };
}

#endif
