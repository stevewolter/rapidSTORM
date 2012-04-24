#ifndef DSTORM_GUF_DATAPLANE_H
#define DSTORM_GUF_DATAPLANE_H

#include <memory>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/area.hpp>

namespace dStorm {
namespace guf {

class Centroid;
template <int Dim> class Statistics;
class Optics;

class DataPlane {
protected:
    const Optics& optics_;
public:
    int tag_index;

protected:
    DataPlane( const Optics& optics ) : optics_(optics), tag_index(-1) {}
private:
    virtual const void* get_data() const = 0;
    virtual std::auto_ptr<Centroid> _residue_centroid() const = 0;

public:
    virtual ~DataPlane() {}
    template <typename Tag> 
    const typename Tag::Data& get_data() const {
        return *static_cast<const typename Tag::Data*>(get_data());
    }
    virtual const Statistics<2>& get_statistics() const = 0;

    const Optics& optics() const { return optics_; }
    virtual boost::units::quantity< boost::units::si::area > pixel_size() const = 0;
    std::auto_ptr<Centroid> residue_centroid() const
        { return _residue_centroid(); }

};

}
}

#endif
