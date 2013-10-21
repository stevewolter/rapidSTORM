#ifndef DSTORM_FORMFITTER_TILE_H
#define DSTORM_FORMFITTER_TILE_H

#include <dStorm/engine/Input.h>
#include <dStorm/Localization.h>
#include <dStorm/display/DataSource.h>
#include <dStorm/traits/Projection.h>

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
