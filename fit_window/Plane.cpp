#include "fit_window/Plane.h"

namespace dStorm {
namespace fit_window {

Spot residue_centroid(const PlaneStack& planes) {
    Spot location;
    double highest_residue = std::numeric_limits<double>::min();
    for (const Plane& plane : planes) {
        for ( const DataPoint& point : plane.points) {
            if (highest_residue < point.residue) {
                highest_residue = point.residue;
                location = point.position;
            }
        }
    }

    return location;
}

}
}
