#ifndef DSTORM_ENGINE_IMAGE_H
#define DSTORM_ENGINE_IMAGE_H

#include "Image_decl.h"
#include "../Image.h"
#include <vector>

namespace dStorm {
namespace engine {

class ImageStack {
public:
    typedef Image2D Plane;
    int plane_count() const { return planes_.size(); }
    Image2D &get_plane( int i ) { return planes_[i]; }
    const Image2D &get_plane( int i ) const { return planes_[i]; }

    typedef std::vector< Image2D >::iterator iterator;
    typedef std::vector< Image2D >::const_iterator const_iterator;
    iterator begin() { return planes_.begin(); }
    const_iterator begin() const { return planes_.begin(); }
    iterator end() { return planes_.end(); }
    const_iterator end() const { return planes_.end(); }
private:
    std::vector< Image2D > planes_;
};

}
}

#endif
