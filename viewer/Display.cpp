#include "Display_impl.h"
#include "ColourScheme.h"

namespace dStorm {
namespace viewer {

template class Display< ColourScheme >;

void BaseDisplay::setSize(
    const Im::Size& size
) {
    int count = 1;
    for (int i = 0; i < size.rows(); ++i) {
        count *= size[i].value();
    }
    ps.resize( count, false );
    ps_step[0] = size[0].value();
    for (int i = 1; i < display::Image::Dim - 1; ++i)
        ps_step[i] = ps_step[i-1] * size[i].value();
}

void BaseDisplay::clear() {
    fill( ps.begin(), ps.end(), false );
}

}
}
