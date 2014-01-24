#ifndef DSTORM_INPUT_JOIN_TEMPORAL_HPP
#define DSTORM_INPUT_JOIN_TEMPORAL_HPP

#include <memory>

#include <dStorm/engine/Image.h>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/Traits.h>

namespace dStorm {
namespace temporal_join {

struct tag {
    static std::string get_name() { return "Temporal"; }
    static std::string get_desc() { return "In time"; }
};

std::unique_ptr< input::Traits<engine::ImageStack> > merge_traits(
        const std::vector< boost::shared_ptr< const input::Traits<engine::ImageStack> > >& traits,
        tag t);
std::unique_ptr< input::Traits<output::LocalizedImage> > merge_traits(
        const std::vector< boost::shared_ptr< const input::Traits<output::LocalizedImage> > >& traits,
        tag t);

std::unique_ptr<input::Source<engine::ImageStack>> Create(
        std::vector<std::unique_ptr<input::Source<engine::ImageStack>>> sources);
std::unique_ptr<input::Source<output::LocalizedImage>> Create(
        std::vector<std::unique_ptr<input::Source<output::LocalizedImage>>> sources);

}
}

#endif
