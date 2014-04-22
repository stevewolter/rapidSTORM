#ifndef DSTORM_GUF_PLANEFUNCTION_HPP
#define DSTORM_GUF_PLANEFUNCTION_HPP

#include "guf/PlaneFunction.h"
#include <nonlinfit/plane/Distance.hpp>
#include <nonlinfit/FunctionConverter.h>
#include "fit_window/PlaneImpl.h"

namespace dStorm {
namespace guf {

template <class Lambda, class Tag>
struct PlaneFunction::Implementation 
: public PlaneFunction
{
    template <typename Metric>
    struct for_metric {
        typedef nonlinfit::FunctionConverter< 
                double,
                nonlinfit::plane::Distance< Lambda, Tag, Metric > 
        > type;
    };
    typename for_metric< nonlinfit::plane::squared_deviations >::type lsq;
    typename for_metric< nonlinfit::plane::negative_poisson_likelihood >::type mle;
  public:
    Implementation( Lambda& expression ) : lsq(expression), mle(expression) {}
    nonlinfit::AbstractFunction<double>& for_data( const fit_window::Plane& data, DistanceMetric metric ) {
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

template <class Function, typename ComputationWay>
std::auto_ptr< PlaneFunction >
PlaneFunction::create( Function& e, ComputationWay )
{
    return std::auto_ptr< PlaneFunction >( 
        new Implementation<Function, ComputationWay>(e) );
}

}
}

#endif
