#include "Display_impl.h"
#include "colour_schemes/impl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) template class Display< Hueing >

#include "colour_schemes/instantiate.h"

void BaseDisplay::setSize(
    const display::Image::Size& size
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
