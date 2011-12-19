#ifndef DSTORM_NOOP_ENGINE_CHAINLINK_H
#define DSTORM_NOOP_ENGINE_CHAINLINK_H

#include "ChainLink_decl.h"
#include <dStorm/engine/Image_decl.h>
#include <dStorm/input/chain/Filter.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/output/LocalizedImage.h>

namespace dStorm {
namespace noop_engine {

class ChainLink
: public simparm::Object,
  public input::chain::Filter
{
    struct Config;
    class Visitor;
    friend class input::chain::DelegateToVisitor;
    Config get_config();

    void make_new_requirements();

  public:
    ChainLink();
    ChainLink* clone() const { return new ChainLink(*this); }

    simparm::Node& getNode() { return *this; }

    input::BaseSource* makeSource() ;

    AtEnd traits_changed( TraitsRef r, Link* l );
};

}
}

#endif
