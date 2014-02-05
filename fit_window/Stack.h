#ifndef DSTORM_GUF_DATACUBE_H
#define DSTORM_GUF_DATACUBE_H

#include "fit_window/Plane.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <nonlinfit/plane/fwd.h>
#include "engine/JobInfo.h"
#include "fit_window/Config.h"

namespace dStorm {
namespace fit_window {

struct Stack;

struct StackCreator {
    class Plane;
    boost::ptr_vector< Plane > planes;

  public:
    template <typename Schedule>
    StackCreator( const Config&, const dStorm::engine::JobInfo&, Schedule, int max_width );
    ~StackCreator();
    std::auto_ptr< Stack >
        set_image( const dStorm::engine::ImageStack& image,
                   const Spot& position ) const;
};

struct Stack 
{
    typedef boost::ptr_vector< Plane > Planes;
    Planes planes;
    friend class StackCreator;
    Stack();

  public:
    typedef typename Planes::iterator iterator;
    iterator begin() { return planes.begin(); }
    iterator end() { return planes.end(); }
    const Plane& operator[](int i) const { return planes[i]; }
    int size() const { return planes.size(); }

    Centroid residue_centroid() const;
};

}
}

#endif
