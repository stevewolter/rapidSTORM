#ifndef DSTORM_FIT_WINDOW_DATAPOINT_H
#define DSTORM_FIT_WINDOW_DATAPOINT_H

#include "fit_window/Spot.h"

namespace dStorm {
namespace fit_window {

struct DataPoint {
    Spot position;
    double value;
    double residue;
};

}
}

#endif
