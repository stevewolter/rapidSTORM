#ifndef DSTORM_GUF_PLANEFUNCTION_H
#define DSTORM_GUF_PLANEFUNCTION_H

#include <nonlinfit/AbstractFunction.h>
#include <memory>
#include "DistanceMetric.h"
#include "Data_fwd.h"
#include "DataPlane.h"

namespace dStorm {
namespace guf {

template <class Lambda>
struct PlaneFunction {
    template <class ComputationWay> class Implementation;
  public:
    typedef typename nonlinfit::get_abstract_function<Lambda,double>::type 
        abstraction;
    template <typename ComputationWay>
    static std::auto_ptr< PlaneFunction > 
        create( Lambda&, ComputationWay );
    virtual ~PlaneFunction() {}
    virtual abstraction& for_data( const DataPlane&, DistanceMetric ) = 0;
};

}
}

#endif
