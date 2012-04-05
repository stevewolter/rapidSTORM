#include <dStorm/traits/optics_config.h>
#include "DepthInfoConfig.h"
#include <simparm/Object.hh>
#include <boost/variant/get.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <dStorm/units/microlength.h>
#include <dStorm/threed_info/Spline.h>

namespace dStorm {
namespace traits {

using namespace boost::units;

class NoThreeDConfig : public simparm::Object, public ThreeDConfig {
    DepthInfo make_traits( const PlaneConfig& ) const { return traits::No3D(); }
    void read_traits( const DepthInfo& d, PlaneConfig& ) {
        assert( boost::get< traits::No3D >( &d ) );
    }
    void set_context( PlaneConfig& pc ) {
        pc.z_position.viewable = false;
        pc.slopes.viewable = false;
        pc.z_calibration_file.viewable = false;
    }
    simparm::Node& getNode() { return *this; }
  public:
    NoThreeDConfig() : simparm::Object("No3D", "No 3D") {}
    NoThreeDConfig* clone() const { return new NoThreeDConfig(); }
};

class Polynomial3DConfig : public simparm::Object, public ThreeDConfig {
    DepthInfo make_traits( const PlaneConfig& ) const;
    void read_traits( const DepthInfo&, PlaneConfig& pc );
    void set_context( PlaneConfig& pc ) {
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
    const Polynomial3D& p = boost::get< traits::Polynomial3D >(d);
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

DepthInfo Polynomial3DConfig::make_traits(const PlaneConfig& pc) const {
    traits::Polynomial3D p;
    p.focal_planes() = pc.z_position().cast< quantity<si::length> >();
    for ( Direction dir = Direction_First; dir < Direction_2D; ++dir ) {
        for ( int term = Polynomial3D::MinTerm; term <= Polynomial3D::Order; ++term ) {
            quantity< si::permicrolength > s = pc.slopes()( dir, term-Polynomial3D::MinTerm );
            if ( s < 1E-30 / si::micrometer )
                p.set_slope( dir, term, 1E24 * si::meter );
            else
                p.set_slope(dir, term, traits::Polynomial3D::WidthSlope( pow<-1>(s) ) ) ;
        }
    }
    return p;
}


Polynomial3DConfig::Polynomial3DConfig()
: simparm::Object("Polynomial3D", "Polynomial 3D")
{
}

class Spline3DConfig : public simparm::Object, public ThreeDConfig {
    DepthInfo make_traits( const PlaneConfig& c ) const {
        if ( c.z_calibration_file )
            return traits::Spline3D( boost::make_shared<threed_info::Spline>(
                threed_info::SplineFactory( c.z_calibration_file() ) ) );
        else
            return traits::No3D();
    }
    void read_traits( const DepthInfo&, PlaneConfig& pc ) 
        { pc.z_calibration_file = ""; }
    void set_context( PlaneConfig& pc ) {
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


std::auto_ptr< ThreeDConfig > make_no_3d_config() 
    { return std::auto_ptr< ThreeDConfig >( new NoThreeDConfig() ); }
std::auto_ptr< ThreeDConfig > make_polynomial_3d_config()
    { return std::auto_ptr< ThreeDConfig >( new Polynomial3DConfig() ); }
std::auto_ptr< ThreeDConfig > make_spline_3d_config()
    { return std::auto_ptr< ThreeDConfig >( new Spline3DConfig() ); }

}
}
