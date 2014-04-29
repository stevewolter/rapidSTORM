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
: public FitFunction 
{
    typedef std::vector<std::unique_ptr<nonlinfit::plane::Term<Tag>>> Evaluators;
    Evaluators implementations;
    nonlinfit::plane::Distance< Tag, DistanceMetric > unconverted;
    nonlinfit::FunctionConverter<double, typename Tag::Number> converted;
    typename Tag::Data xs;

    std::vector<nonlinfit::plane::Term<Tag>*> get_pointers(const Evaluators& evaluators) {
        std::vector<nonlinfit::plane::Term<Tag>*> result;
        for (const auto& e : evaluators) {
            result.push_back(e.get());
        }
        return result;
    }

  public:
    PlaneFunctionImplementation(Evaluators expression,
                                const fit_window::Plane& plane)
        : implementations(std::move(expression)),
          unconverted(get_pointers(implementations)),
          converted(unconverted) {
        fit_window::chunkify(plane, xs);
        unconverted.set_data(xs);
    }

    nonlinfit::AbstractFunction<double>* abstract_function() OVERRIDE {
        return &converted;
    }

    double highest_residue(Spot& spot) const {
        double current_highest = std::numeric_limits<double>::min();
        for (const typename Tag::Data::DataRow& row : xs.data) {
            for (int i = 0; i < row.residues.rows(); ++i) {
                if (current_highest < row.residues[i]) {
                    current_highest = row.residues[i];
                    spot = xs.get_coordinate(row, i).template cast<double>();
                }
            }
        }
        return current_highest;
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename ComputationWay>
std::auto_ptr<FitFunction>
PlaneFunction<ComputationWay>::create( Evaluators e, const fit_window::Plane& data, bool mle)
{
    if (mle) {
        return std::auto_ptr<FitFunction>( 
            new PlaneFunctionImplementation<ComputationWay, nonlinfit::plane::negative_poisson_likelihood>(
                std::move(e), data) );
    } else {
        return std::auto_ptr<FitFunction>( 
            new PlaneFunctionImplementation<ComputationWay, nonlinfit::plane::squared_deviations>(
                std::move(e), data) );
    }
}

}
}

#endif
