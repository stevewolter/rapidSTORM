#ifndef DSTORM_GUF_PLANEFUNCTION_H
#define DSTORM_GUF_PLANEFUNCTION_H

#include <memory>

#include "guf/DistanceMetric.h"
#include "guf/FitFunction.h"
#include "nonlinfit/AbstractFunction.h"
#include "nonlinfit/plane/Term.h"

namespace dStorm {
namespace fit_window { class Plane; }
namespace guf {

template <typename ComputationWay>
struct PlaneFunction {
  public:
    typedef std::vector<std::unique_ptr<nonlinfit::plane::Term<ComputationWay>>> Evaluators;
    static std::unique_ptr<FitFunction> 
        create( Evaluators, const fit_window::Plane&, bool mle );

  private:
    template <typename DistanceMetric>
    static std::unique_ptr<FitFunction> create1(Evaluators, const fit_window::Plane&);
    template <typename DistanceMetric, int VariableCount>
    static std::unique_ptr<FitFunction> create2(Evaluators, const fit_window::Plane&);
};

}
}

#endif
