#ifndef DSTORM_ENGINE_SPOTFITTERFACTORY_H
#define DSTORM_ENGINE_SPOTFITTERFACTORY_H

#include "engine/JobInfo_decl.h"
#include "engine/SpotFitter_decl.h"
#include "engine/Image.h"
#include "simparm/NodeHandle.h"
#include <memory>
#include "output/Traits_decl.h"
#include "output/Basename_decl.h"
#include "simparm/BaseAttribute.h"
#include "simparm/Choice.h"

namespace dStorm {
namespace engine {
namespace spot_fitter {

struct Factory : public simparm::Choice {
  public:
    virtual Factory* clone() const = 0;
    virtual ~Factory();
    virtual std::auto_ptr<Implementation> make( const JobInfo& ) = 0;
    virtual void set_requirements( InputTraits& ) = 0;
    virtual void set_traits( output::Traits&, const JobInfo& ) = 0;
    virtual void set_variables( output::Basename& ) const {}

    virtual void register_trait_changing_nodes( simparm::BaseAttribute::Listener ) = 0;
    /** Check whether spot fitter objects could actually be built with the current configuration. */
    virtual void check_configuration( const InputTraits&, const output::Traits& ) {}
};

}
}
}

#include "make_clone_allocator.hpp"
DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::engine::spot_fitter::Factory)

#endif
