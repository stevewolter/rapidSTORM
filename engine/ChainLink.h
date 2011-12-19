#ifndef DSTORM_ENGINE_CHAINLINK_H
#define DSTORM_ENGINE_CHAINLINK_H

#include "ChainLink_decl.h"
#include <dStorm/engine/Image_decl.h>
#include <dStorm/input/chain/Filter.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/chain/MetaInfo.h>
#include "Config.h"
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/engine/ClassicEngine.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>

namespace dStorm {
namespace engine {

class ChainLink
: public ClassicEngine,
  protected simparm::Listener
{
    input::chain::Link::TraitsRef upstream_traits;
    input::chain::MetaInfo::Ptr my_traits;
    Config config;

    std::string amplitude_threshold_string() const;
    void make_new_traits();

  protected:
    void operator()( const simparm::Event& );

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ChainLink* clone() const { return new ChainLink(*this); }

    simparm::Node& getNode() { return config; }

    input::Source<output::LocalizedImage>* makeSource() ;

    AtEnd traits_changed( TraitsRef r, Link* l );

    void add_spot_finder( spot_finder::Factory& finder) { config.spotFindingMethod.addChoice(finder); }
    void add_spot_fitter( spot_fitter::Factory& fitter) { 
        fitter.register_trait_changing_nodes(*this);
        config.spotFittingMethod.addChoice(fitter); 
    }
};

}
}

#endif
