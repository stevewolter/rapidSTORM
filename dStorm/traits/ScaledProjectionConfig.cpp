#include "ScaledProjection.h"
#include "ProjectionConfig.h"
#include <simparm/Object.hh>
#include <boost/make_shared.hpp>

namespace dStorm {
namespace traits {

class ScaledProjectionConfig
: public ProjectionConfig
{
    simparm::Object node;
    simparm::Node& getNode_() { return node; }
    boost::shared_ptr<Projection> get_projection_( const Optics<2>::Resolutions& r ) const { 
        return boost::make_shared<ScaledProjection>(r[0]->in_dpm(), r[0]->in_dpm() );
    }

    ProjectionConfig* clone_() const 
        { return new ScaledProjectionConfig(*this); }

  public:
    ScaledProjectionConfig() : node("ScaledProjection", "No alignment") {}
};

std::auto_ptr<ProjectionConfig> make_scaling_projection_config() {
    return std::auto_ptr<ProjectionConfig>( new ScaledProjectionConfig() );
}

}
}
