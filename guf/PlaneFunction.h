#ifndef DSTORM_GUF_PLANEFUNCTION_H
#define DSTORM_GUF_PLANEFUNCTION_H

#include <nonlinfit/AbstractFunction.h>
#include <memory>
#include "guf/DistanceMetric.h"

namespace dStorm {
namespace fit_window { class Plane; }
namespace guf {

struct PlaneFunction {
    template <class Lambda, class ComputationWay> class Implementation;
  public:
    template <class Lambda, typename ComputationWay>
    static std::auto_ptr< PlaneFunction > 
        create( Lambda&, ComputationWay );
    virtual ~PlaneFunction() {}
    virtual nonlinfit::AbstractFunction<double>& for_data( const fit_window::Plane&, DistanceMetric ) = 0;
};

}
}

#endif
