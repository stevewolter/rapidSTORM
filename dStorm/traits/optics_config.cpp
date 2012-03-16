#include "debug.h"
#include "optics_config.h"
#include <boost/lexical_cast.hpp>
#include <dStorm/localization/Traits.h>
#include "ProjectionFactory.h"

namespace dStorm {
namespace traits {

using namespace boost::units;

PlaneConfig::PlaneConfig(int number)
: simparm::Set("InputLayer" + boost::lexical_cast<std::string>(number), 
                  "Input layer " + boost::lexical_cast<std::string>(number+1)),
  is_first_layer(number==0),
  z_position("ZPosition", "Point of sharpest Z", ZPosition::Constant(0 * si::nanometre)),
  counts_per_photon( "CountsPerPhoton", "Camera response to photon" ),
  dark_current( "DarkCurrent", "Dark intensity" ),
  alignment( "Alignment", "Plane alignment" ),
  psf_size("PSF", "PSF FWHM", PSFSize::Constant(500.0 * boost::units::si::nanometre)),
  pixel_size("PixelSizeInNM", "Size of one input pixel",
                   PixelSize::Constant(107.0f * si::nanometre / camera::pixel)),
  slopes("WideningConstants", "Widening slopes")
{
    slopes.helpID = "Polynomial3D.WideningSlopes";
    alignment.addChoice( make_scaling_projection_config() );
    if ( ! is_first_layer ) {
        alignment.addChoice( make_affine_projection_config() );
        alignment.addChoice( make_support_point_projection_config() );
    }

    psf_size.helpID = "PSF.FWHM";
    z_position.setHelp("Z position where this layer is sharpest in this dimension");
    transmissions.push_back( new simparm::Entry<double>("Transmission0", "Transmission of fluorophore 0", 1) );

    counts_per_photon.userLevel = Object::Intermediate;
    dark_current.userLevel = Object::Intermediate;

    alignment.viewable = ! is_first_layer;
}

PlaneConfig::PlaneConfig( const PlaneConfig& o )
: simparm::Set(o), is_first_layer(o.is_first_layer), 
  z_position(o.z_position), counts_per_photon(o.counts_per_photon), 
  dark_current(o.dark_current), alignment( o.alignment ),
  psf_size(o.psf_size),
  pixel_size(o.pixel_size),
  slopes(o.slopes)
{
    for (Transmissions::const_iterator i = o.transmissions.begin(), e = o.transmissions.end(); i != e; ++i)
    {
        transmissions.push_back( i->clone() );
    }
}

void PlaneConfig::registerNamedEntries()
{
    push_back( pixel_size );
    push_back( psf_size );
    push_back( z_position );
    push_back( slopes );
    push_back( counts_per_photon );
    push_back( dark_current );
    push_back( alignment );

    for (Transmissions::iterator i = transmissions.begin(), e = transmissions.end(); i != e; ++i)
        push_back( *i );
}

void PlaneConfig::set_fluorophore_count( int fluorophore_count, bool multiplane )
{
    while ( int(transmissions.size()) < fluorophore_count ) {
       std::string i = boost::lexical_cast<std::string>(transmissions.size());
       transmissions.push_back( new simparm::Entry<double>("Transmission" + i,
         	"Transmission of fluorophore " + i, 1) );
        push_back( transmissions.back() );
    }

    for (Transmissions::iterator i = transmissions.begin(); i != transmissions.end(); ++i)
	i->viewable = (i - transmissions.begin()) < fluorophore_count && (fluorophore_count > 1 || multiplane);
}
void PlaneConfig::set_context( const input::Traits<Localization>& t, int fluorophore_count, ThreeDConfig& t3 ) {
    set_fluorophore_count( fluorophore_count, false );
    t3.set_context( *this );
}

void PlaneConfig::set_context( const traits::Optics& o, int fluorophore_count, bool multilayer, ThreeDConfig& t3)
{
    set_fluorophore_count( fluorophore_count, multilayer );
    t3.set_context( *this );
}

void PlaneConfig::write_traits( traits::Optics& rv, const ThreeDConfig& t3) const
{
    rv.projection_factory_ = alignment().get_projection_factory();
    rv.photon_response = counts_per_photon();
    rv.dark_current = dark_current();
    rv.z_position = z_position().cast< quantity<si::length,float> >();
    rv.tmc.clear();
    for ( Transmissions::const_iterator i = transmissions.begin(); i != transmissions.end(); ++i) {
        DEBUG("Set transmission of " << rv.tmc.size() << " to " << i->value() << " at " << &rv);
        rv.tmc.push_back( i->value() );
    }
    rv.psf_size(0) = psf_size().cast< quantity<si::length,float> >();
    for (int i = 0; i < 2; ++i)
        (*rv.psf_size(0))[i] /= 2.35;
    rv.depth_info() = t3.make_traits( *this );
}

void PlaneConfig::read_traits( const traits::Optics& t, ThreeDConfig& t3 )
{
    for (int i = 0; i < int(t.tmc.size()); ++i) {
        transmissions[i] = t.tmc[i];
    }
    if ( t.psf_size(0).is_initialized() ) {
        PSFSize s = (*t.psf_size(0) ).cast< PSFSize::Scalar >();
        for (int i = 0; i < 2; ++i) s[i] *= 2.35;
        psf_size = s;
    }
    if ( t.z_position ) z_position = t.z_position->cast< quantity<si::nanolength> >();
    if ( t.photon_response ) counts_per_photon = *t.photon_response;
    if ( t.dark_current ) dark_current = *t.dark_current;
    if ( t.depth_info() ) t3.read_traits( *t.depth_info(), *this );
}

void PlaneConfig::write_traits( input::Traits<Localization>& t, const ThreeDConfig& t3 ) const
{
    for (int i = 0; i < 2; ++i)
        t.position().resolution()[i] = 1.0f / (pixel_size()[i] / (1E9f * si::nanometre) * si::metre) ;
}


CuboidConfig::CuboidConfig() 
: simparm::Object("Optics", "Optical pathway properties")
{
    layers.push_back( new PlaneConfig(0) );
    set_number_of_planes( 1 );
}

void CuboidConfig::registerNamedEntries()
{
    for ( Layers::iterator i = layers.begin(); i != layers.end(); ++i) {
        i->registerNamedEntries();
        push_back( *i );
    }
}

void CuboidConfig::set_number_of_planes(int number)
{
    while ( number > int( layers.size() ) ) {
        layers.push_back( new PlaneConfig( layers.size() ) );
        layers.back().registerNamedEntries();
        push_back( layers.back() );
    }
    while ( number < int( layers.size() ) )
        layers.pop_back();
}

image::MetaInfo<2>::Resolutions
PlaneConfig::get_resolution() const {
    const Eigen::Matrix< quantity< nanometer_pixel_size, float >, 2, 1, Eigen::DontAlign >
        f = pixel_size();
    quantity< divide_typeof_helper<
        si::length,camera::length>::type, float > q1;
    image::MetaInfo<2>::Resolutions r;
    for (int i = 0; i < 2; ++i) {
        q1 = (f[i] / (1E9 * si::nanometre) * si::metre);
        r[i] = q1;
    }
    return r;
}

image::MetaInfo<2>::Resolutions
CuboidConfig::get_resolution() const {
    return layers[0].get_resolution();
}

void CuboidConfig::write_traits( input::Traits<engine::ImageStack>& t, const ThreeDConfig& t3 ) const
{
    for (int i = 0; i < int( layers.size() ) && i < t.plane_count(); ++i) {
        layers[i].write_traits( t.optics(i), t3 );
        t.image(i).set_resolution( layers[i].get_resolution() );
    }
}

void CuboidConfig::write_traits( input::Traits<Localization>& t, const ThreeDConfig& t3 ) const
{
    layers[0].write_traits( t, t3 );
}

void CuboidConfig::read_traits( const input::Traits<engine::ImageStack>& t, ThreeDConfig& t3 )
{
    set_context(t, t3);
    for (int i = 0; i < t.plane_count(); ++i) {
        layers[i].read_traits( t.optics(i), t3 );
    }
}

void CuboidConfig::set_context( const input::Traits<engine::ImageStack>& t, ThreeDConfig& t3 ) 
{
    set_number_of_planes( t.plane_count() );
    for (int i = 0; i < t.plane_count(); ++i) {
        layers[i].set_context( t.optics(i), t.fluorophores.size(), (t.plane_count() > 1), t3 );
    }
}

void CuboidConfig::set_context( const input::Traits<Localization>& t, ThreeDConfig& t3 ) 
{
    set_number_of_planes( 1 );
    layers[0].set_context( t, t.fluorophores.size(), t3 );
}


}
}
