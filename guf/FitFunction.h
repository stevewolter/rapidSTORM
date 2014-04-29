#ifndef DSTORM_GUF_FITFUNCTION_H
#define DSTORM_GUF_FITFUNCTION_H

#include "guf/Spot.h"
#include "nonlinfit/AbstractFunction.h"

namespace dStorm {
namespace guf {

class FitFunction {
  public:
    virtual ~FitFunction() {}
    virtual nonlinfit::AbstractFunction<double>* abstract_function() = 0;
    virtual double highest_residue(Spot& spot) const = 0;
};

}
}

#endif
