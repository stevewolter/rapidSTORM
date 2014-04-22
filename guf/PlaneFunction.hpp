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
    nonlinfit::plane::Distance< Lambda, Tag, nonlinfit::plane::squared_deviations > lsq;
    nonlinfit::plane::Distance< Lambda, Tag, nonlinfit::plane::negative_poisson_likelihood > mle;
    nonlinfit::FunctionConverter<double, typename Tag::Number> lsq_converter;
    nonlinfit::FunctionConverter<double, typename Tag::Number> mle_converter;

  public:
    Implementation( Lambda& expression ) : lsq(expression), mle(expression), lsq_converter(lsq), mle_converter(mle) {}
    nonlinfit::AbstractFunction<double>& for_data( const fit_window::Plane& data, DistanceMetric metric ) {
        const typename Tag::Data& typed_data
            = dynamic_cast< const fit_window::PlaneImpl<Tag>& >( data ).data;
        switch (metric) {
        case LeastSquares:
            lsq.set_data(typed_data);
            return lsq_converter;
        case PoissonLikelihood:
            mle.set_data(typed_data);
            return mle_converter;
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
