#ifndef DSTORM_RECONSTRUCTION_H
#define DSTORM_RECONSTRUCTION_H

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
    template <typename T> void
    ReconstructionByDilationII(const cimg_library::CImg<T>& src,
                               const cimg_library::CImg<T>& mask,
                               cimg_library::CImg<T>& target)
        throw();

    template <typename T> 
        bool firstIsBrighter( const cimg_library::CImg<T>& a,
                              const cimg_library::CImg<T>& b ) throw();
    template <typename T> 
        cimg_library::CImg<T> add_border_to_image
            (const cimg_library::CImg<T>& src, int n) throw();

}

#endif
