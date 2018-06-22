#ifndef DSTORM_GUF_FITFUNCTIONCREATOR_H
#define DSTORM_GUF_FITFUNCTIONCREATOR_H

#include "guf/Config.h"
#include "engine/InputPlane.h"
#include "guf/MultiKernelModel.h"
#include "fit_window/Plane.h"
#include "nonlinfit/AbstractFunction.h"

namespace dStorm {
namespace guf {

class FitFunctionFactory {
  public:
    static std::unique_ptr<FitFunctionFactory> create(
        const Config&, const engine::InputPlane&, int kernel_count);

    virtual ~FitFunctionFactory() {}
    virtual std::vector<bool> reduction_bitset() const = 0;
    virtual MultiKernelModel fit_position() = 0;
    virtual std::unique_ptr<nonlinfit::AbstractFunction<double>> create_function(
        const fit_window::Plane& plane, bool mle) = 0;
};

}
}

#endif
