#ifndef DSTORM_TRAITS_PROJECTION_CONFIG_H
#define DSTORM_TRAITS_PROJECTION_CONFIG_H

#include <simparm/Node.hh>
#include <simparm/ObjectChoice.hh>
#include <memory>
#include <dStorm/make_clone_allocator.hpp>

namespace dStorm {
namespace traits {

class ProjectionFactory;

class ProjectionConfig : public simparm::ObjectChoice {
    virtual ProjectionConfig* clone_() const = 0;
    virtual ProjectionFactory* get_projection_factory_() const = 0;
  public:
    ProjectionConfig( std::string name, std::string desc ) 
        : simparm::ObjectChoice(name,desc) {}
    virtual ~ProjectionConfig() {}
    ProjectionConfig* clone() const { return clone_(); }
    std::auto_ptr<ProjectionFactory> get_projection_factory() const 
        { return std::auto_ptr<ProjectionFactory>(get_projection_factory_()); }
};

std::auto_ptr<ProjectionConfig> make_scaling_projection_config();
std::auto_ptr<ProjectionConfig> make_affine_projection_config();
std::auto_ptr<ProjectionConfig> make_support_point_projection_config();

}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::traits::ProjectionConfig)

#endif
