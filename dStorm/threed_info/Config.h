#ifndef DSTORM_TRAITS_DEPTHINFOCONFIG_H
#define DSTORM_TRAITS_DEPTHINFOCONFIG_H

#include <dStorm/threed_info/DepthInfo.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <dStorm/Direction.h>
#include <simparm/Node.hh>

namespace dStorm {
namespace traits { class PlaneConfig; }
namespace threed_info {

struct Config {
    virtual ~Config() {}
    virtual boost::shared_ptr<DepthInfo> make_traits( Direction ) const = 0;
    virtual void read_traits( const DepthInfo&, const DepthInfo& ) = 0;
    virtual void set_context( ) = 0;
    virtual simparm::Node& getNode() = 0;
    operator simparm::Node&() { return getNode(); }
    operator const simparm::Node&() const { return const_cast<Config&>(*this).getNode(); }
    virtual Config* clone() const = 0;
};

class No3DConfig;
class Polynomial3DConfig;
class Spline3DConfig;
std::auto_ptr< Config > make_no_3d_config();
std::auto_ptr< Config > make_polynomial_3d_config();
std::auto_ptr< Config > make_spline_3d_config();
std::auto_ptr< Config > make_lens_3d_config();

}
}

#endif
