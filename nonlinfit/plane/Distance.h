#ifndef NONLINFIT_EVALUATORS_PLANE_GENERIC_H
#define NONLINFIT_EVALUATORS_PLANE_GENERIC_H

#include "nonlinfit/plane/fwd.h"
#include "nonlinfit/DataChunk.h"
#include "nonlinfit/Evaluation.h"
#include "nonlinfit/Term.h"

namespace nonlinfit {
namespace plane {

template <typename Number, int ChunkSize>
class Distance : public AbstractFunction<Evaluation<Number>>
{
  public:
    enum DistanceMetric {
        kSquaredDeviations,
        kPoissonMaximumLikelihood,
    };

    Distance(const std::vector<Term<Number, ChunkSize>*>& terms,
             std::vector<DataChunk<Number, ChunkSize>>& data,
             DistanceMetric distance_metric);
    bool evaluate( Evaluation<Number>& p );
    int variable_count() const { return variable_count_; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  private:
    const std::vector<Term<Number, ChunkSize>*>& terms_;
    std::vector<DataChunk<Number, ChunkSize>>& data_;
    int variable_count_;
    Eigen::Matrix<Number, ChunkSize, Eigen::Dynamic> jacobian_;
    DistanceMetric distance_metric_;
};

}
}

#endif
