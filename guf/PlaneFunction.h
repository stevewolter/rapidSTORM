#ifndef DSTORM_GUF_PLANEFUNCTION_H
#define DSTORM_GUF_PLANEFUNCTION_H

#include <nonlinfit/AbstractFunction.h>
#include <memory>
#include "guf/DistanceMetric.h"

namespace dStorm {
namespace fit_window { class Plane; }
namespace guf {

struct PlaneFunction {
  public:
    template <class Lambda, typename ComputationWay>
    static std::auto_ptr<nonlinfit::AbstractFunction<double>> 
        create( Lambda&, const fit_window::Plane&, bool mle );
};

}
}

#endif
