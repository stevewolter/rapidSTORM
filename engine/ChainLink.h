#ifndef DSTORM_ENGINE_CHAINLINK_H
#define DSTORM_ENGINE_CHAINLINK_H

#include "ChainLink_decl.h"
#include <dStorm/engine/Image_decl.h>
#include <dStorm/input/chain/Filter.h>
#include <dStorm/input/Source.h>
#include <dStorm/engine/Config.h>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/engine/ClassicEngine.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>

namespace dStorm {
namespace engine {

class ChainLink
: public input::chain::TypedFilter<engine::Image> ,
  public ClassicEngine,
  protected simparm::Listener
{
    input::chain::Context::Ptr my_context;
    Config config;

  protected:
    void operator()( const simparm::Event& );

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ChainLink* clone() const { return new ChainLink(*this); }

    simparm::Node& getNode() { return config; }

    input::Source<output::LocalizedImage>*
        makeSource( std::auto_ptr< input::Source<Image> > );

    void traits_changed( TraitsRef, Link*, ObjectTraitsPtr );
    void context_changed(ContextRef, Link*);

    void add_spot_finder( SpotFinderFactory& finder) { config.spotFindingMethod.addChoice(finder); }
    void add_spot_fitter( SpotFitterFactory& fitter) { config.spotFittingMethod.addChoice(fitter); }
};

}
}

#endif
