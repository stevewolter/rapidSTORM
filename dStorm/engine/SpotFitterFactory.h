#ifndef DSTORM_ENGINE_SPOTFITTERFACTORY_H
#define DSTORM_ENGINE_SPOTFITTERFACTORY_H

#include "JobInfo_decl.h"
#include "SpotFitter_decl.h"
#include "Input_decl.h"
#include "Config_decl.h"
#include <simparm/Node_decl.hh>
#include <memory>
#include <dStorm/output/Traits_decl.h>

namespace dStorm {
namespace engine {

struct SpotFitterFactory {
    simparm::Node& node;
  public:
    SpotFitterFactory(simparm::Node& node) : node(node) {}
    simparm::Node& getNode() { return node; }
    operator simparm::Node&() { return node; }
    const simparm::Node& getNode() const { return node; }
    operator const simparm::Node&() const { return node; }

    virtual SpotFitterFactory* clone() const = 0;
    virtual ~SpotFitterFactory();
    virtual std::auto_ptr<SpotFitter> make( const JobInfo& ) = 0;
    std::auto_ptr<SpotFitter> make_by_parts( const Config&, const InputTraits& );
    virtual void set_traits( output::Traits& ) = 0;
};

}
}

#endif
