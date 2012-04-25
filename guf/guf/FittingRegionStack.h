#ifndef DSTORM_GUF_DATACUBE_H
#define DSTORM_GUF_DATACUBE_H

#include "FittingRegion.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <nonlinfit/plane/fwd.h>
#include "guf/psf/LengthUnit.h"
#include <dStorm/engine/JobInfo.h>
#include "Config.h"

namespace dStorm {
namespace guf {

struct FittingRegionStack;

struct FittingRegionStackCreator {
    class Plane;
    boost::ptr_vector< Plane > planes;

  public:
    FittingRegionStackCreator( const Config&, const dStorm::engine::JobInfo& );
    ~FittingRegionStackCreator();
    std::auto_ptr< FittingRegionStack >
        set_image( const dStorm::engine::ImageStack& image,
                   const guf::Spot& position ) const;
};

struct FittingRegionStack 
{
    typedef boost::ptr_vector< FittingRegion > Planes;
    Planes planes;
    friend class FittingRegionStackCreator;
    FittingRegionStack();

  public:
    typedef typename Planes::iterator iterator;
    iterator begin() { return planes.begin(); }
    iterator end() { return planes.end(); }
    const FittingRegion& operator[](int i) const { return planes[i]; }
    int size() const { return planes.size(); }

    Centroid residue_centroid() const;
};

}
}

#endif
