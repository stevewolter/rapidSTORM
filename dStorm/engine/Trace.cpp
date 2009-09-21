#include "engine/Trace.h"
#include "statistics/Variance.h"

namespace dStorm {

void Trace::compute_SD() {
    Variance x, y;
    for (int i = 0; i < size(); i++) {
        x.addValue( (*this)[i].x() );
        y.addValue( (*this)[i].y() );
    }

    sd_x = sqrt( x.variance() );
    sd_y = sqrt( y.variance() );
    computed_variance = true;
}

}
