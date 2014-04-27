#ifndef DSTORM_ENGINE_IMAGE_H
#define DSTORM_ENGINE_IMAGE_H

#include "engine/Image_decl.h"
#include "image/Image.h"
#include <vector>
#include "units/frame_count.h"

namespace dStorm {
namespace engine {

class ImageStack {
public:
    typedef Image2D Plane;
    int plane_count() const { return planes_.size(); }
    Image2D &plane( int i ) { return planes_[i]; }
    const Image2D &plane( int i ) const { return planes_[i]; }
    Image2D &background( int i ) { return background_[i]; }
    const Image2D &background( int i ) const { return background_[i]; }

    typedef Image2D value_type;
    typedef std::vector< Image2D >::iterator iterator;
    typedef std::vector< Image2D >::const_iterator const_iterator;
    typedef std::vector< Image2D >::reference reference;
    typedef std::vector< Image2D >::const_reference const_reference;

    iterator begin() { return planes_.begin(); }
    const_iterator begin() const { return planes_.begin(); }
    iterator end() { return planes_.end(); }
    const_iterator end() const { return planes_.end(); }

    void push_back( const Plane& );
    void set_background( int plane, Image2D background ) { background_[plane] = background; }
    void clear();

    frame_index frame_number() const { return fn; }
    frame_index& frame_number() { return fn; }
    void set_frame_number( frame_index n ) { fn = n; }

    ImageStack();
    ImageStack( frame_index );
    ImageStack( const Image2D& );

    bool has_invalid_planes() const;

private:
    std::vector< Plane > planes_, background_;
    frame_index fn;
};

}
}

#endif
