#ifndef DSTORM_GUF_PLANEFUNCTION_HPP
#define DSTORM_GUF_PLANEFUNCTION_HPP

#include "PlaneFunction.h"
#include <nonlinfit/plane/Distance.hpp>
#include <nonlinfit/AbstractFunctionAdapter.h>
#include <nonlinfit/FunctionConverter.h>
#include "fit_window/PlaneImpl.h"

namespace dStorm {
namespace guf {

template <class Function>
template <class Tag>
struct PlaneFunction<Function>::Implementation 
: public PlaneFunction
{
    template <typename Metric>
    struct for_metric {
        typedef nonlinfit::AbstractedFunction<
            nonlinfit::FunctionConverter< 
                double,
                nonlinfit::plane::Distance< Function, Tag, Metric > 
            >
        > type;
    };
    typename for_metric< nonlinfit::plane::squared_deviations >::type lsq;
    typename for_metric< nonlinfit::plane::negative_poisson_likelihood >::type mle;
  public:
    Implementation( Function& expression ) : lsq(expression), mle(expression) {}
    abstraction& for_data( const fit_window::Plane& data, DistanceMetric metric ) {
        const typename Tag::Data& typed_data
            = dynamic_cast< const fit_window::PlaneImpl<Tag>& >( data ).data;
        switch (metric) {
        case LeastSquares:
            lsq.set_data(typed_data);
            return lsq;
        case PoissonLikelihood:
            mle.set_data(typed_data);
            return mle;
        default:
            throw std::logic_error("Non-implemented metric");
        }
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <class Function>
template <typename ComputationWay>
std::auto_ptr< PlaneFunction<Function> >
PlaneFunction<Function>::create( Function& e, ComputationWay )
{
    return std::auto_ptr< PlaneFunction<Function> >( 
        new Implementation<ComputationWay>(e) );
}

}
}

#endif
