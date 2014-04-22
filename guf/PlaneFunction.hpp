#ifndef DSTORM_GUF_PLANEFUNCTION_HPP
#define DSTORM_GUF_PLANEFUNCTION_HPP

#include "guf/PlaneFunction.h"
#include "nonlinfit/plane/Distance.hpp"
#include "nonlinfit/FunctionConverter.h"
#include "nonlinfit/plane/DataFacade.hpp"
#include "fit_window/chunkify.hpp"

namespace dStorm {
namespace guf {

template <class Lambda, class Tag, class DistanceMetric>
struct PlaneFunctionImplementation 
: public nonlinfit::AbstractFunction<double>
{
    nonlinfit::plane::Distance< Lambda, Tag, DistanceMetric > unconverted;
    nonlinfit::FunctionConverter<double, typename Tag::Number> converted;
    typename Tag::Data data;

  public:
    PlaneFunctionImplementation( Lambda& expression, const fit_window::Plane& plane )
        : unconverted(expression), converted(unconverted) {
        fit_window::chunkify(plane, data);
        unconverted.set_data(data);
    }

    int variable_count() const OVERRIDE { return converted.variable_count(); }
    bool evaluate( Derivatives& p ) OVERRIDE { return converted.evaluate(p); }
    void get_position( Position& p ) const OVERRIDE { converted.get_position(p); }
    void set_position( const Position& p ) OVERRIDE { converted.set_position(p); }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <class Function, typename ComputationWay>
std::auto_ptr< nonlinfit::AbstractFunction<double> >
PlaneFunction::create( Function& e, const fit_window::Plane& data, bool mle )
{
    if (mle) {
        return std::auto_ptr< nonlinfit::AbstractFunction<double> >( 
            new PlaneFunctionImplementation<Function, ComputationWay, nonlinfit::plane::negative_poisson_likelihood>(e, data) );
    } else {
        return std::auto_ptr< nonlinfit::AbstractFunction<double> >( 
            new PlaneFunctionImplementation<Function, ComputationWay, nonlinfit::plane::squared_deviations>(e, data) );
    }
}

}
}

#endif
