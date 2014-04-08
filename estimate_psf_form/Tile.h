#ifndef DSTORM_FORMFITTER_TILE_H
#define DSTORM_FORMFITTER_TILE_H

#include "engine/Input.h"
#include "Localization.h"
#include "display/DataSource.h"
#include "traits/Projection.h"

namespace dStorm {
namespace estimate_psf_form {

struct Tile {
    dStorm::display::Image::Size region_start, region_end;
    engine::ImageStack image;
    Localization spot;
    int fluorophore;
    std::vector< traits::Projection::Bounds > bounds;
};

}
}

#endif
