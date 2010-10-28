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

    input::BaseSource* makeSource() 
        { return dispatch_makeSource(Invalid); }
    input::Source<output::LocalizedImage>*
        makeSource( std::auto_ptr< input::Source<Image> > );

    AtEnd traits_changed( TraitsRef r, Link* l ) 
        { return dispatch_trait_change(r, l, Invalid); }
    AtEnd traits_changed( TraitsRef, Link*, ObjectTraitsPtr );
    AtEnd context_changed(ContextRef, Link*);

    void add_spot_finder( SpotFinderFactory& finder) { config.spotFindingMethod.addChoice(finder); }
    void add_spot_fitter( SpotFitterFactory& fitter) { config.spotFittingMethod.addChoice(fitter); }

    void modify_context( input::Traits<Image>& ) { assert(false); }
    void notice_context( const input::Traits<Image>& ) { assert(false); }
};

}
}

#endif
