#ifndef DSTORM_STM_ENGINE_CHAINLINK_H
#define DSTORM_STM_ENGINE_CHAINLINK_H

#include "ChainLink_decl.h"
#include <dStorm/input/chain/Filter.h>
#include <dStorm/input/Source.h>
#include <dStorm/output/LocalizedImage.h>
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace engine_stm {

class ChainLink
: public input::chain::TypedFilter<Localization>,
  public input::chain::TypedFilter<output::LocalizedImage>
{
    input::chain::Context::Ptr my_context;
    Config config;

  public:
    ChainLink* clone() const { return new ChainLink(*this); }

    simparm::Node& getNode() { return config; }

    input::BaseSource* makeSource() 
        { return dispatch_makeSource(Invalid); }
    input::Source<output::LocalizedImage>*
        makeSource( std::auto_ptr< input::Source<Localization> > );
    input::Source<output::LocalizedImage>*
        makeSource( std::auto_ptr< input::Source<output::LocalizedImage> > i )
        { return i.release(); }

    AtEnd traits_changed( TraitsRef r, Link* l ) 
        { return dispatch_trait_change(r, l, Invalid); }
    AtEnd traits_changed( TraitsRef, Link*, 
        boost::shared_ptr< input::Traits<Localization> > );
    AtEnd traits_changed( TraitsRef, Link*, input::Traits<output::LocalizedImage>::Ptr );

    AtEnd context_changed(ContextRef, Link*);

    void modify_context( input::Traits<Image>& ) { assert(false); }
    void notice_context( const input::Traits<Image>& ) { assert(false); }
};

}
}

#endif
