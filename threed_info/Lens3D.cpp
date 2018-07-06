#include "simparm/BoostUnits.h"
#include "simparm/Entry.h"
#include "threed_info/Lens3D.h"
#include "threed_info/Config.h"
#include "units/nanolength.h"
#include "LengthUnit.h"
#include <boost/units/io.hpp>

namespace dStorm {
namespace threed_info {

using namespace boost::units;

std::ostream& Lens3D::print_( std::ostream& o ) const {
    return o << "lens-determined 3D at " << z_position_;
}

class Lens3DConfig : public Config {
    simparm::Entry< quantity<si::nanolength> > z_position;

    boost::shared_ptr<DepthInfo> make_traits( Direction ) const { 
        boost::shared_ptr<Lens3D> rv( new Lens3D() );
        rv->z_position_ = ToLengthUnit(z_position());
        return rv;
    }
    void read_traits( const DepthInfo& dx, const DepthInfo& dy ) 
        { throw std::logic_error("Not implemented"); }
    void set_context() {}
    void attach_ui( simparm::NodeHandle at ) {
        simparm::NodeHandle r = attach_parent(at);
        z_position.attach_ui( r ); 
    }
  public:
    Lens3DConfig() 
        : Config("Lens3D", "Theoretical 3D"),
          z_position("ZPosition", "Z position", 0.0 * boost::units::si::nanometre) 
    {}
    Lens3DConfig* clone() const { return new Lens3DConfig(*this); }
};

std::auto_ptr< Config > make_lens_3d_config() 
    { return std::auto_ptr< Config >( new Lens3DConfig() ); }

}
}
