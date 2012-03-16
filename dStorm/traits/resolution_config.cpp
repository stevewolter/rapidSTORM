#include "debug.h"
#include "resolution_config.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include <boost/variant/get.hpp>
#include <dStorm/units/permicrolength.h>

namespace dStorm {
namespace traits {
namespace resolution {

Config::Config()
: simparm::Object("Optics", "Optical parameters"),
  three_d("ThreeD", "3D PSF model")
{
    three_d.helpID = "3DType";
    three_d.addChoice( make_no_3d_config() );
    three_d.addChoice( make_polynomial_3d_config() );
}

Config::~Config() {}

void Config::registerNamedEntries() {
    push_back( three_d );
    cuboid_config.registerNamedEntries();
    push_back( cuboid_config );
}

void Config::write_traits( engine::InputTraits& t ) const
{
    DEBUG("Setting traits in ResolutionSetter, input is " << t.size.transpose());
    cuboid_config.write_traits(t, three_d());
}

traits::ImageResolution
Config::get( const FloatPixelSizeEntry::value_type& f ) {
    boost::units::quantity< boost::units::divide_typeof_helper<
        boost::units::si::length,camera::length>::type, float > q1;
    q1 = (f / (1E9 * boost::units::si::nanometre) * boost::units::si::metre);
    return q1;
}

void Config::write_traits( input::Traits<Localization>& t ) const {
    DEBUG("Setting resolution in Config");
    cuboid_config.write_traits( t, three_d() );
}

void Config::set_context( const engine::InputTraits& t ) {
    cuboid_config.set_context( t, three_d() );
}
void Config::set_context( const input::Traits<Localization>& t ) {
    cuboid_config.set_context( t, three_d() );
}

void Config::read_traits( const engine::InputTraits& t ) {
    set_context( t );
    const boost::optional<traits::DepthInfo> d = t.optics(0).depth_info();
    assert( d.is_initialized() );
    if ( boost::get< traits::No3D >( d.get_ptr() ) ) {
        three_d.choose( "No3D" );
    } else if ( boost::get< traits::Polynomial3D >( d.get_ptr() ) ) {
        three_d.choose( "Polynomial3D" );
    } else
        throw std::runtime_error("ThreeD info not recognized");

    cuboid_config.read_traits( t, three_d() );
}

void Config::read_traits( const input::Traits<dStorm::Localization>& t ) {
    set_context( t );
    /* TODO: Implementation of this function would enable displaying resolutions
     * from STM files. */
}

image::MetaInfo<2>::Resolutions Config::get_resolution() const
{
    return cuboid_config.get_resolution();
}


}
}
}

namespace boost {
namespace units {

}
}
