#ifndef DSTORM_GUF_DATAPLANE_H
#define DSTORM_GUF_DATAPLANE_H

#include <memory>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/area.hpp>
#include <boost/units/systems/camera/intensity.hpp>
#include "Spot.h"

namespace dStorm {
namespace guf {

class Centroid;
class Optics;

class DataPlane {
public:
    const Optics& optics;
    int tag_index;
    Spot highest_pixel;
    float integral, peak_intensity, background_estimate;
    boost::units::quantity<boost::units::si::area> pixel_size;
    boost::units::quantity<boost::units::si::length> standard_deviation[2];
    int pixel_count;

protected:
    DataPlane( const Optics& optics ) : optics(optics), tag_index(-1) {}
private:
    virtual const void* get_data() const = 0;
    virtual std::auto_ptr<Centroid> _residue_centroid() const = 0;

public:
    virtual ~DataPlane() {}
    template <typename Tag> 
    const typename Tag::Data& get_data() const {
        return *static_cast<const typename Tag::Data*>(get_data());
    }

    std::auto_ptr<Centroid> residue_centroid() const
        { return _residue_centroid(); }

};

}
}

#endif
