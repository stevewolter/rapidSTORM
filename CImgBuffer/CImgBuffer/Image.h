#ifndef CIMGBUFFER_IMAGE_H
#define CIMGBUFFER_IMAGE_H

#include <memory>
#include <CImg.h>

namespace CImgBuffer {
    /** Wrapper around a CImg that provides an image number field for
     *  image identification. */
    template <typename T> class Image : public cimg_library::CImg<T> 
    {
      private:
        unsigned long num;

      public:
        /** The Pixel type gives the type of a single image pixel. */
        typedef T Pixel;

        /** Construct Image with width, height and number. */
        Image(unsigned int w, unsigned int h, unsigned long num) throw() 
        : cimg_library::CImg<T>(w, h), num(num) {}
        /** Construct Image from CImg and image number. */
        Image(const cimg_library::CImg<T>& i, 
            unsigned long num) throw() 
        : cimg_library::CImg<T>(i, false), num(num) {}

        inline unsigned long getImageNumber() const throw() { return num; }
        inline void setImageNumber(unsigned long n) throw() { num = n; }
    };
};

#endif
