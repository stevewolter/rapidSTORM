#ifndef DSTORM_GUF_DATACUBE_H
#define DSTORM_GUF_DATACUBE_H

#include "DataPlane.h"
#include "InputPlane.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <nonlinfit/plane/fwd.h>
#include "guf/psf/LengthUnit.h"
#include <dStorm/engine/JobInfo.h>

namespace dStorm {
namespace guf {

struct DataCube;

struct InputCube {
    boost::ptr_vector< InputPlane > planes;

  public:
    InputCube( const Config&, const dStorm::engine::JobInfo& );
    std::auto_ptr< DataCube >
        set_image( const dStorm::engine::ImageStack& image,
                   const guf::Spot& position ) const;
};

struct DataCube 
{
    typedef boost::ptr_vector< DataPlane > Planes;
    Planes planes;
    friend class InputCube;
    DataCube();

  public:
    typedef typename Planes::iterator iterator;
    iterator begin() { return planes.begin(); }
    iterator end() { return planes.end(); }
    const DataPlane& operator[](int i) const { return planes[i]; }
    int size() const { return planes.size(); }

    Centroid residue_centroid() const;
};

}
}

#endif
