#ifndef DSTORM_TRAITS_DEPTHINFOCONFIG_H
#define DSTORM_TRAITS_DEPTHINFOCONFIG_H

#include "threed_info/DepthInfo.h"
#include <boost/smart_ptr/shared_ptr.hpp>
#include "Direction.h"
#include <simparm/ObjectChoice.h>
#include "make_clone_allocator.hpp"

namespace dStorm {
namespace traits { class PlaneConfig; }
namespace threed_info {

struct Config : public simparm::ObjectChoice {
    Config( std::string name, std::string desc ) : ObjectChoice(name,desc) {}
    Config( std::string name ) : ObjectChoice(name) {}
    virtual boost::shared_ptr<DepthInfo> make_traits( Direction ) const = 0;
    virtual void read_traits( const DepthInfo&, const DepthInfo& ) = 0;
    virtual void set_context( ) = 0;
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

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::threed_info::Config)

#endif
