#ifndef DSTORM_GUF_PLANEFUNCTION_H
#define DSTORM_GUF_PLANEFUNCTION_H

#include <nonlinfit/AbstractFunction.h>
#include <memory>
#include "DistanceMetric.h"

namespace dStorm {
namespace fit_window { class Plane; }
namespace guf {

template <class Lambda>
struct PlaneFunction {
  public:
    typedef typename nonlinfit::get_abstract_function<Lambda,double>::type 
        abstraction;
    template <typename ComputationWay>
    static std::auto_ptr< PlaneFunction > 
        create( Lambda&, ComputationWay );
    virtual ~PlaneFunction() {}
    virtual abstraction& for_data( const fit_window::Plane&, DistanceMetric ) = 0;
};

}
}

#endif
