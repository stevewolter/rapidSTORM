#ifndef DSTORM_TRAITS_PROJECTION_CONFIG_H
#define DSTORM_TRAITS_PROJECTION_CONFIG_H

#include <simparm/Node.hh>
#include <boost/smart_ptr/shared_ptr.hpp>
#include "optics.h"

namespace dStorm {
namespace traits {

class Projection;

class ProjectionConfig {
    virtual ProjectionConfig* clone_() const = 0;
    virtual simparm::Node& getNode_() = 0;
    virtual boost::shared_ptr<Projection> get_projection_( const Optics<2>::Resolutions& ) const = 0;
  public:
    virtual ~ProjectionConfig() {}
    ProjectionConfig* clone() const { return clone_(); }
    simparm::Node& getNode() { return getNode_(); }
    const simparm::Node& getNode() const { return const_cast<ProjectionConfig&>(*this).getNode_(); }
    operator simparm::Node&() { return getNode_(); }
    operator const simparm::Node&() const { return getNode(); }
    boost::shared_ptr<Projection> get_projection( const Optics<2>::Resolutions& r ) const 
        { return get_projection_(r); }
};

std::auto_ptr<ProjectionConfig> make_scaling_projection_config();
std::auto_ptr<ProjectionConfig> make_affine_projection_config();
std::auto_ptr<ProjectionConfig> make_support_point_projection_config();

}
}

#endif
