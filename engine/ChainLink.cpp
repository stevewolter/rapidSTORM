#include "engine/ChainLink.h"
#include "debug.h"

#include <boost/lexical_cast.hpp>
#include <boost/units/io.hpp>

#include "dejagnu.h"
#include "engine/ChainLink.h"
#include "engine/Engine.h"
#include "guf/Factory.h"
#include "helpers/make_unique.hpp"
#include "input/InputMutex.h"
#include "input/FilterFactory.h"
#include "input/FilterFactoryLink.h"
#include "output/LocalizedImage_traits.h"
#include "spotFinders/spotFinders.h"

namespace dStorm {
namespace engine {

using namespace input;
using boost::signals2::scoped_connection;

class ChainLink
: public input::FilterFactory<engine::ImageStack, output::LocalizedImage>
{
    Config config;
    simparm::Object engine_node;
    simparm::BaseAttribute::ConnectionStore listening[4];

    boost::shared_ptr<const Traits<output::LocalizedImage>> make_meta_info(
        MetaInfo& meta_info,
        boost::shared_ptr<const Traits<engine::ImageStack>> traits)
        OVERRIDE {
        config.separate_plane_fitting.set_visibility(traits->plane_count() > 1);
        return Engine::convert_traits(config, *traits);
    }

    std::unique_ptr<input::Source<output::LocalizedImage>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> input) OVERRIDE {
        return make_unique<Engine>( config, std::move(input) );
    }

  public:
    ChainLink();
    ChainLink* clone() const { return new ChainLink(*this); }

    void attach_ui(simparm::NodeHandle at,
                   std::function<void()> traits_change_callback) OVERRIDE { 
        listening[0] = config.fit_judging_method.value.notify_on_value_change(
                traits_change_callback);
        listening[1] = config.spotFittingMethod.value.notify_on_value_change( 
                traits_change_callback);
        listening[2] = config.spotFindingMethod.value.notify_on_value_change( 
                traits_change_callback);
        listening[3] = config.separate_plane_fitting.value.notify_on_value_change( 
                traits_change_callback);
        for (auto& spot_fitter_factory : config.spotFittingMethod) {
            spot_fitter_factory.register_trait_changing_nodes(
                    traits_change_callback);
        }
        config.attach_ui(engine_node.attach_ui(at)); 
    }
};

ChainLink::ChainLink() 
: engine_node("Engine", "")
{
    config.spotFindingMethod.addChoice(
            spalttiefpass_smoother::make_spot_finder_factory() );
    config.spotFindingMethod.addChoice(
            median_smoother::make_spot_finder_factory() );
    config.spotFindingMethod.addChoice(
            erosion_smoother::make_spot_finder_factory() );
    config.spotFindingMethod.addChoice(
            gauss_smoother::make_spot_finder_factory() );
    config.spotFindingMethod.addChoice(
            spaltbandpass_smoother::make_spot_finder_factory() );
    config.spotFittingMethod.addChoice(make_unique<guf::Factory>());
}

std::unique_ptr<input::FilterFactory<engine::ImageStack, output::LocalizedImage>> make_rapidSTORM_engine_link() {
    return make_unique<ChainLink>();
}

void unit_test( TestState& state ) {
    auto rv = make_rapidSTORM_engine_link();
    rv.reset();
    state.pass("Destruction of engine works");
    auto cloner = make_rapidSTORM_engine_link();
    rv.reset( cloner->clone() );
    cloner.reset();
    rv.reset();
    state.pass("Destruction of engine works");
}

}
}
