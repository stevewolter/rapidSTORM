#ifndef DSTORM_GUF_PLANEFUNCTION_HPP
#define DSTORM_GUF_PLANEFUNCTION_HPP

#include "PlaneFunction.h"
#include <nonlinfit/plane/Distance.hpp>
#include <nonlinfit/AbstractFunctionAdapter.h>
#include <nonlinfit/FunctionConverter.h>
#include "DataPlane.h"

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
    abstraction& for_data( const DataPlane& data, DistanceMetric metric ) {
        if ( metric == LeastSquares ) {
            lsq.set_data( data.get_data<Tag>() );
            return lsq;
        } else if ( metric == PoissonLikelihood ) {
            mle.set_data( data.get_data<Tag>() );
            return mle;
        } else 
            throw std::logic_error("Non-implemented metric");
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
