#include "debug.h"

#include <boost/make_shared.hpp>
#include <boost/mpl/vector.hpp>

#include "engine_stm/ChainLink.h"
#include "engine_stm/LocalizationBuncher.h"
#include "helpers/make_unique.hpp"
#include "input/FilterFactory.h"
#include "input/Source.h"
#include "localization/record.h"
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"

namespace dStorm {
namespace engine_stm {

using namespace input;

class ChainLink : public input::FilterFactory<localization::Record, output::LocalizedImage> {
  public:
    ChainLink* clone() const OVERRIDE { return new ChainLink(*this); }
    void attach_ui(simparm::NodeHandle at,
                   std::function<void()> traits_change_callback) OVERRIDE {}

    std::unique_ptr<input::Source<output::LocalizedImage>> make_source(
        std::unique_ptr<input::Source<localization::Record>> input) OVERRIDE {
        return make_unique<Source>(std::move(input));
    }

    boost::shared_ptr<const input::Traits<output::LocalizedImage>> make_meta_info(
        boost::shared_ptr<const input::Traits<localization::Record>> input_meta_info) OVERRIDE {
        return boost::make_shared<Traits<output::LocalizedImage>>(
                *input_meta_info, "STM", "Localizations file");
    }
};

std::unique_ptr<input::FilterFactory<localization::Record, output::LocalizedImage>> create() {
    return make_unique<ChainLink>();
}

}
}
