#ifndef DSTORM_INPUT_JOIN_SPATIAL_H
#define DSTORM_INPUT_JOIN_SPATIAL_H

#include <dStorm/input/Source.h>
#include <dStorm/engine/Image.h>
#include <dStorm/output/LocalizedImage.h>

namespace dStorm {
namespace spatial_join {

struct tag {
    static std::string get_name() { return "Spatialz"; }
    static std::string get_desc() { return "Spatially in z dimension"; }
};

std::unique_ptr< input::Traits<engine::ImageStack> > merge_traits(
        const std::vector< boost::shared_ptr< const input::Traits<engine::ImageStack> > >& traits,
        tag t);
inline std::unique_ptr< input::Traits<output::LocalizedImage> > merge_traits(
        const std::vector< boost::shared_ptr< const input::Traits<output::LocalizedImage> > >& traits,
        tag t) {
    throw std::logic_error("Sorry, merging localizations spatially is not supported yet.");
}


std::unique_ptr<input::Source<engine::ImageStack>> Create(
        std::vector<std::unique_ptr<input::Source<engine::ImageStack>>> sources);

}
}

#endif
