#include <simparm/BoostUnits.hh>
#include <simparm/Entry_Impl.hh>
#include "Lens3D.h"
#include "Config.h"
#include <dStorm/units/nanolength.h>

namespace dStorm {
namespace threed_info {

std::ostream& Lens3D::print_( std::ostream& o ) const {
    return o << "lens-determined 3D at " << z_position_;
}

class Lens3DConfig : public Config {
    simparm::Entry< quantity<si::nanolength> > z_position;

    boost::shared_ptr<DepthInfo> make_traits( Direction ) const { 
        boost::shared_ptr<Lens3D> rv( new Lens3D() );
        rv->z_position_ = ZPosition(z_position());
        return rv;
    }
    void read_traits( const DepthInfo& dx, const DepthInfo& dy ) 
        { throw std::logic_error("Not implemented"); }
    void set_context() {}
    void registerNamedEntries() { z_position.attach_ui( this->node ); }
  public:
    Lens3DConfig() 
        : Config("Lens3D", "Theoretical 3D"),
          z_position("ZPosition", "Z position", 0.0 * boost::units::si::nanometre) 
    { 
        registerNamedEntries(); 
    }
    Lens3DConfig* clone() const { 
        Lens3DConfig* p = new Lens3DConfig(*this); 
        p->registerNamedEntries();
        return p;
    }
};

std::auto_ptr< Config > make_lens_3d_config() 
    { return std::auto_ptr< Config >( new Lens3DConfig() ); }

}
}
