#ifndef DSTORM_FORMFITTER_TILE_H
#define DSTORM_FORMFITTER_TILE_H

#include <dStorm/engine/Input.h>
#include <dStorm/Localization.h>
#include <dStorm/display/DataSource.h>
#include "guf/guf/TransformedImage.h"

namespace dStorm {
namespace form_fitter {

struct Tile {
    dStorm::display::Image::Size region_start, region_end;
    engine::ImageStack image;
    Localization spot;
    int fluorophore;
    std::vector< guf::TransformedImage< si::length >::Bounds > bounds;
};

}
}

#endif
