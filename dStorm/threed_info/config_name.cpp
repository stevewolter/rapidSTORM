#include "config_name.h"
#include <boost/variant/apply_visitor.hpp>
#include <stdexcept>

namespace dStorm {
namespace threed_info {

using namespace boost::units;

struct config_name_visitor
: public boost::static_visitor< std::string >
{
public:
    std::string operator()( const No3D& ) const { return "No3D"; }
    std::string operator()( const Polynomial3D& p ) const { return "Polynomial3D"; }
    std::string operator()( const Spline3D& p ) const { return "Spline3D"; }
};

std::string config_name( const DepthInfo& o ) {
    return boost::apply_visitor( config_name_visitor(), o );
}

}
}
