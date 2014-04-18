#ifndef DSTORM_OUTPUTS_CALIBRATE_3D_H
#define DSTORM_OUTPUTS_CALIBRATE_3D_H

#include "output/OutputSource.h"
#include <memory>

namespace dStorm {
namespace calibrate_3d {

std::auto_ptr<output::OutputSource> make_output_source();

namespace sigma_curve {
std::auto_ptr<output::OutputSource> make_output_source();
}

}

}

#endif

