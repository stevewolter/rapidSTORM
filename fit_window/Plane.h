#ifndef DSTORM_GUF_DATAPLANE_H
#define DSTORM_GUF_DATAPLANE_H

#include <vector>

#include "fit_window/DataPoint.h"
#include "fit_window/Spot.h"

namespace dStorm {
namespace fit_window {

class Optics;

struct Plane {
    const Optics* optics;
    float integral, peak_intensity, background_estimate;
    double pixel_size;
    double min_coordinate[2], max_coordinate[2];
    double standard_deviation[2];
    int window_width;
    std::vector<DataPoint> points;
    int highest_pixel_index;
    bool has_per_pixel_background;
};

typedef std::vector<Plane> PlaneStack;

}
}

#endif
