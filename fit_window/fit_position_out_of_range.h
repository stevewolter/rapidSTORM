#ifndef DSTORM_GUF_FITPOSITION_OUT_OF_RANGE_H
#define DSTORM_GUF_FITPOSITION_OUT_OF_RANGE_H

#include <stdexcept>

namespace dStorm {
namespace fit_window {

struct fit_position_out_of_range : public std::runtime_error
{
    fit_position_out_of_range();
};

}
}

#endif
