#ifndef DSTORM_TRAITS_PROJECTIONFACTORY_H
#define DSTORM_TRAITS_PROJECTIONFACTORY_H

#include <dStorm/image/fwd.h>
#include <memory>

namespace dStorm {
namespace traits {

class Projection;

class ProjectionFactory {
    virtual Projection* get_projection_( const image::MetaInfo<2>& ) const = 0;
  public:
    virtual ~ProjectionFactory() {}
    std::auto_ptr<Projection> get_projection( const image::MetaInfo<2>& i ) const
        { return std::auto_ptr<Projection>( get_projection_(i) ); }
};

}
}

#endif
