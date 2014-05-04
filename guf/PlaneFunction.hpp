#ifndef DSTORM_GUF_PLANEFUNCTION_HPP
#define DSTORM_GUF_PLANEFUNCTION_HPP

#include "guf/PlaneFunction.h"
#include "nonlinfit/plane/Distance.hpp"
#include "nonlinfit/FunctionConverter.h"
#include "fit_window/chunkify.hpp"

#include <iostream>

namespace dStorm {
namespace guf {

template <class Tag, class DistanceMetric, int VariableCount>
struct PlaneFunctionImplementation 
: public FitFunction 
{
    typedef std::vector<std::unique_ptr<nonlinfit::plane::Term<Tag>>> Evaluators;
    Evaluators implementations;
    nonlinfit::plane::Distance< Tag, DistanceMetric, VariableCount > unconverted;
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
        fit_window::chunkify<DistanceMetric::need_logoutput>(plane, xs);
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

    double r_value(Spot center, Spot width, double constant_background) const {
        double sum_of_squared_residues = 0, sum_of_squared_values = 0;
        for (const typename Tag::Data::DataRow& row : xs.data) {
            for (int i = 0; i < row.residues.rows(); ++i) {
                if (((xs.get_coordinate(row, i).template cast<double>() - center).array().abs() < width.array()).all()) {
                    sum_of_squared_residues += row.residues[i] * row.residues[i];
                    sum_of_squared_values += pow(row.output[i] - row.background[i] - constant_background, 2);
                }
            }
        }
        return 1.0 - sum_of_squared_residues / sum_of_squared_values;
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename ComputationWay>
template <typename DistanceMetric, int VariableCount>
std::unique_ptr<FitFunction>
PlaneFunction<ComputationWay>::create2(Evaluators e, const fit_window::Plane& data) {
    return std::unique_ptr<FitFunction>(
            new PlaneFunctionImplementation<ComputationWay, DistanceMetric, VariableCount>(
                std::move(e), data));
}

template <typename ComputationWay>
template <typename DistanceMetric>
std::unique_ptr<FitFunction>
PlaneFunction<ComputationWay>::create1(Evaluators e, const fit_window::Plane& data) {
    int variable_count = 0;
    for (const auto& evaluator : e) {
        variable_count += evaluator->term_variable_count;
    }

    if (variable_count == 3) {
        return create2<DistanceMetric, 3>(std::move(e), data);
    } else if (variable_count == 4) {
        return create2<DistanceMetric, 4>(std::move(e), data);
    } else if (variable_count == 5) {
        return create2<DistanceMetric, 5>(std::move(e), data);
    } else if (variable_count == 6) {
        return create2<DistanceMetric, 6>(std::move(e), data);
    } else {
        std::cerr << "Creating variable-sized distance for variable count " << variable_count << std::endl;
        return create2<DistanceMetric, Eigen::Dynamic>(std::move(e), data);
    }
}

template <typename ComputationWay>
std::unique_ptr<FitFunction>
PlaneFunction<ComputationWay>::create( Evaluators e, const fit_window::Plane& data, bool mle)
{
    if (mle) {
        return create1<nonlinfit::plane::negative_poisson_likelihood>(std::move(e), data);
    } else {
        return create1<nonlinfit::plane::squared_deviations>(std::move(e), data);
    }
}

}
}

#endif
