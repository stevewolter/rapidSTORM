#include <dStorm/traits/optics_config.h>
#include "Config.h"
#include <simparm/Object.hh>
#include <boost/variant/get.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <dStorm/units/microlength.h>
#include <dStorm/threed_info/Spline3D.h>
#include <dStorm/threed_info/No3D.h>
#include <dStorm/threed_info/Polynomial3D.h>

namespace dStorm {
namespace threed_info {

using namespace boost::units;
using dStorm::traits::PlaneConfig;

class No3DConfig : public simparm::Object, public Config {
    boost::shared_ptr<DepthInfo> make_traits( const PlaneConfig& pc ) const { 
        boost::shared_ptr<No3D> rv( new No3D() );
        for (int i = 0; i < 2; ++i)
            rv->sigma[i] = Sigma(pc.psf_size()[i] / 2.35);
        return rv;
    }
    void read_traits( const DepthInfo& d, PlaneConfig& ) {
        assert( dynamic_cast<const No3D*>( &d ) );
    }
    void set_context( PlaneConfig& pc ) {
        pc.z_range.viewable = false;
        pc.z_position.viewable = false;
        pc.slopes.viewable = false;
        pc.z_calibration_file.viewable = false;
    }
    simparm::Node& getNode() { return *this; }
  public:
    No3DConfig() : simparm::Object("No3D", "No 3D") {}
    No3DConfig* clone() const { return new No3DConfig(); }
};

class Polynomial3DConfig : public simparm::Object, public Config {
    boost::shared_ptr<DepthInfo> make_traits( const PlaneConfig& ) const;
    void read_traits( const DepthInfo&, PlaneConfig& pc );
    void set_context( PlaneConfig& pc ) {
        pc.z_range.viewable = true;
        pc.z_position.viewable = true;
        pc.slopes.viewable = true;
        pc.z_calibration_file.viewable = false;
    }
    simparm::Node& getNode() { return *this; }
    void registerNamedEntries() {}
  public:
    Polynomial3DConfig();
    Polynomial3DConfig* clone() const { return new Polynomial3DConfig(*this); }
};

void Polynomial3DConfig::read_traits( const DepthInfo& d, PlaneConfig& pc )
{
    const Polynomial3D& p = static_cast< const Polynomial3D& >(d);
    if ( p.z_limit() ) pc.z_range = p.z_limit()->cast< quantity<si::nanolength> >();
    if ( p.focal_planes() ) pc.z_position = p.focal_planes()->cast< quantity<si::nanolength> >();
    PlaneConfig::SlopeEntry::value_type slopes;

    for ( Direction dir = Direction_First; dir < Direction_2D; ++dir ) {
        for ( int term = Polynomial3D::MinTerm; term <= Polynomial3D::Order; ++term ) {
            slopes(dir, term-Polynomial3D::MinTerm) = 
                PlaneConfig::SlopeEntry::value_type::Scalar(1.0 / p.get_slope(dir,term));
        }
    }
    pc.slopes = slopes;
}

boost::shared_ptr<DepthInfo> Polynomial3DConfig::make_traits(const PlaneConfig& pc) const {
    boost::shared_ptr<Polynomial3D> p( new Polynomial3D() );
    p->set_z_limit( pc.z_range().cast< ZPosition >() );
    p->focal_planes() = pc.z_position().cast< ZPosition >();
    for ( Direction dir = Direction_First; dir < Direction_2D; ++dir ) {
        p->set_base_width( dir, Sigma(pc.psf_size()[dir] / 2.35) );
        for ( int term = Polynomial3D::MinTerm; term <= Polynomial3D::Order; ++term ) {
            quantity< si::permicrolength > s = pc.slopes()( dir, term-Polynomial3D::MinTerm );
            if ( s < 1E-30 / si::micrometer )
                p->set_slope( dir, term, 1E24 * si::meter );
            else
                p->set_slope(dir, term, Polynomial3D::WidthSlope( pow<-1>(s) ) ) ;
        }
    }
    return p;
}


Polynomial3DConfig::Polynomial3DConfig()
: simparm::Object("Polynomial3D", "Polynomial 3D")
{
}

class Spline3DConfig : public simparm::Object, public Config {
    boost::shared_ptr<DepthInfo> make_traits( const PlaneConfig& c ) const {
        if ( c.z_calibration_file )
            return boost::shared_ptr<DepthInfo>(new Spline3D( SplineFactory( c.z_calibration_file() ) ));
        else
            return boost::shared_ptr<DepthInfo>();
    }
    void read_traits( const DepthInfo&, PlaneConfig& pc ) 
        { pc.z_calibration_file = ""; }
    void set_context( PlaneConfig& pc ) {
        pc.z_range.viewable = false;
        pc.z_position.viewable = false;
        pc.slopes.viewable = false;
        pc.z_calibration_file.viewable = true;
    }
    simparm::Node& getNode() { return *this; }
    void registerNamedEntries() {}
  public:
    Spline3DConfig() : simparm::Object("Spline3D", "Interpolated 3D") {}
    Spline3DConfig* clone() const { return new Spline3DConfig(*this); }
};


std::auto_ptr< Config > make_no_3d_config() 
    { return std::auto_ptr< Config >( new No3DConfig() ); }
std::auto_ptr< Config > make_polynomial_3d_config()
    { return std::auto_ptr< Config >( new Polynomial3DConfig() ); }
std::auto_ptr< Config > make_spline_3d_config()
    { return std::auto_ptr< Config >( new Spline3DConfig() ); }

}
}
