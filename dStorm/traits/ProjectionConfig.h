#ifndef DSTORM_TRAITS_PROJECTION_CONFIG_H
#define DSTORM_TRAITS_PROJECTION_CONFIG_H

#include <simparm/Node.hh>
#include <memory>

namespace dStorm {
namespace traits {

class ProjectionFactory;

class ProjectionConfig {
    virtual ProjectionConfig* clone_() const = 0;
    virtual simparm::Node& getNode_() = 0;
    virtual ProjectionFactory* get_projection_factory_() const = 0;
  public:
    virtual ~ProjectionConfig() {}
    ProjectionConfig* clone() const { return clone_(); }
    simparm::Node& getNode() { return getNode_(); }
    const simparm::Node& getNode() const { return const_cast<ProjectionConfig&>(*this).getNode_(); }
    operator simparm::Node&() { return getNode_(); }
    operator const simparm::Node&() const { return getNode(); }
    std::auto_ptr<ProjectionFactory> get_projection_factory() const 
        { return std::auto_ptr<ProjectionFactory>(get_projection_factory_()); }
};

std::auto_ptr<ProjectionConfig> make_scaling_projection_config();
std::auto_ptr<ProjectionConfig> make_affine_projection_config();
std::auto_ptr<ProjectionConfig> make_support_point_projection_config();

}
}

#endif
