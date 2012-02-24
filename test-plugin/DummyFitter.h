#ifndef DSTORM_DUMMYFITTER_H
#define DSTORM_DUMMYFITTER_H

#include <dStorm/engine/SpotFitter.h>
#include <dStorm/engine/SpotFitterBuilder.h>
#include <simparm/Set.hh>
#include <dStorm/engine/InputTraits.h>

namespace dStorm {
namespace debugplugin {

struct DummyFitterConfig : public simparm::Set {
    DummyFitterConfig();
    ~DummyFitterConfig();

    void set_traits( output::Traits&, const engine::JobInfo& );
    void set_requirements( input::Traits<engine::ImageStack>& ) {}
};

class DummyFitter : public dStorm::engine::spot_fitter::Implementation {
    dStorm::input::Traits<dStorm::engine::ImageStack> traits;
    int counter, length;
  public:
    typedef DummyFitterConfig Config;
    typedef dStorm::engine::spot_fitter::Builder<DummyFitter> Source;
    DummyFitter(const Config& config, const dStorm::engine::JobInfo& info);
    int fitSpot( const dStorm::engine::FitPosition& spot, const dStorm::engine::ImageStack &im,
                        iterator target );
};

}
}

#endif
