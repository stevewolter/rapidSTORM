#ifndef DSTORM_GUF_PLANEFUNCTION_HPP
#define DSTORM_GUF_PLANEFUNCTION_HPP

#include "guf/PlaneFunction.h"
#include "nonlinfit/plane/Distance.hpp"
#include "nonlinfit/FunctionConverter.h"
#include "fit_window/chunkify.hpp"

namespace dStorm {
namespace guf {

template <class Tag, class DistanceMetric>
struct PlaneFunctionImplementation 
: public nonlinfit::AbstractFunction<double>
{
    typedef std::vector<std::unique_ptr<nonlinfit::plane::Term<Tag>>> Evaluators;
    Evaluators implementations;
    nonlinfit::plane::Distance< Tag, DistanceMetric > unconverted;
    nonlinfit::FunctionConverter<double, typename Tag::Number> converted;
    typename Tag::Data xs;
    std::vector<nonlinfit::DataChunk<typename Tag::Number, Tag::ChunkSize>> ys;

    std::vector<nonlinfit::plane::Term<Tag>*> get_pointers(const Evaluators& evaluators) {
        std::vector<nonlinfit::plane::Term<Tag>*> result;
        for (const auto& e : evaluators) {
            result.push_back(e.get());
        }
        return result;
    }

  public:
    PlaneFunctionImplementation(Evaluators expression, const fit_window::Plane& plane )
        : implementations(std::move(expression)),
          unconverted(get_pointers(implementations)),
          converted(unconverted) {
        fit_window::chunkify(plane, xs);
        fit_window::chunkify_data_chunks(plane, ys);
        unconverted.set_data(xs, ys);
    }

    int variable_count() const OVERRIDE { return converted.variable_count(); }
    bool evaluate( Derivatives& p ) OVERRIDE { return converted.evaluate(p); }
    void get_position( Position& p ) const OVERRIDE { converted.get_position(p); }
    void set_position( const Position& p ) OVERRIDE { converted.set_position(p); }
    bool step_is_negligible( const Position& old_position,
                             const Position& new_position ) const {
        return converted.step_is_negligible(old_position, new_position);
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename ComputationWay>
std::auto_ptr< nonlinfit::AbstractFunction<double> >
PlaneFunction<ComputationWay>::create( Evaluators e, const fit_window::Plane& data, bool mle )
{
    if (mle) {
        return std::auto_ptr< nonlinfit::AbstractFunction<double> >( 
            new PlaneFunctionImplementation<ComputationWay, nonlinfit::plane::negative_poisson_likelihood>(std::move(e), data) );
    } else {
        return std::auto_ptr< nonlinfit::AbstractFunction<double> >( 
            new PlaneFunctionImplementation<ComputationWay, nonlinfit::plane::squared_deviations>(std::move(e), data) );
    }
}

}
}

#endif
