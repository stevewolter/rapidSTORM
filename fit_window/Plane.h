#ifndef DSTORM_GUF_DATAPLANE_H
#define DSTORM_GUF_DATAPLANE_H

#include <memory>
#include "Spot.h"

namespace dStorm {
namespace fit_window {

class Centroid;
class Optics;

class Plane {
public:
    const Optics& optics;
    int tag_index;
    Spot highest_pixel;
    float integral, peak_intensity, background_estimate;
    double pixel_size;
    double standard_deviation[2];
    int pixel_count;

protected:
    Plane( const Optics& optics ) : optics(optics), tag_index(-1) {}
private:
    virtual std::auto_ptr<Centroid> _residue_centroid() const = 0;

public:
    virtual ~Plane() {}

    std::auto_ptr<Centroid> residue_centroid() const
        { return _residue_centroid(); }

};

}
}

#endif
