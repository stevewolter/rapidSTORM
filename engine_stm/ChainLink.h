#ifndef DSTORM_STM_ENGINE_CHAINLINK_H
#define DSTORM_STM_ENGINE_CHAINLINK_H

#include "ChainLink_decl.h"
#include "Config.h"
#include <dStorm/input/chain/Filter.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/Source.h>
#include <dStorm/output/LocalizedImage.h>
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace engine_stm {

class ChainLink : public input::chain::Filter
{
    class Visitor;
    friend class input::chain::DelegateToVisitor;
    input::chain::Context::Ptr my_context;
    Config config;
    Config& get_config() { return config; }

  public:
    ChainLink* clone() const { return new ChainLink(*this); }

    simparm::Node& getNode() { return config; }

    input::BaseSource* makeSource() ;
    AtEnd traits_changed( TraitsRef r, Link* l );
    AtEnd context_changed(ContextRef, Link*);
};

}
}

#endif
