#include "debug.h"
#include "optics_config.h"
#include <boost/lexical_cast.hpp>
#include <dStorm/localization/Traits.h>
#include "ProjectionFactory.h"

namespace dStorm {
namespace traits {

using namespace boost::units;

PlaneConfig::PlaneConfig(int number, Purpose purpose)
: simparm::Set("InputLayer" + boost::lexical_cast<std::string>(number), 
                  "Input layer " + boost::lexical_cast<std::string>(number+1)),
  purpose(purpose),
  three_d("ThreeD", "3D PSF model"),
  counts_per_photon( "CountsPerPhoton", "Camera response to photon" ),
  dark_current( "DarkCurrent", "Dark intensity" ),
  alignment( "Alignment", "Plane alignment" ),
  pixel_size("PixelSizeInNM", "Size of one input pixel",
                   PixelSize::Constant(107.0f * si::nanometre / camera::pixel))
{
    three_d.helpID = "3DType";
    if ( purpose == InputSimulation ) { 
        three_d.addChoice( threed_info::make_lens_3d_config() );
    } else {
        three_d.addChoice( threed_info::make_no_3d_config() );
        three_d.addChoice( threed_info::make_spline_3d_config() );
        three_d.addChoice( threed_info::make_polynomial_3d_config() );
    }

    alignment.addChoice( make_scaling_projection_config() );
    alignment.addChoice( make_affine_projection_config() );
    alignment.addChoice( make_support_point_projection_config() );

    transmissions.push_back( new simparm::Entry<double>("Transmission0", "Transmission of fluorophore 0", 1) );

    counts_per_photon.userLevel = Object::Intermediate;
    dark_current.userLevel = Object::Intermediate;
}

PlaneConfig::PlaneConfig( const PlaneConfig& o )
: simparm::Set(o), 
  purpose(o.purpose),
  three_d(o.three_d),
  counts_per_photon(o.counts_per_photon), 
  dark_current(o.dark_current), alignment( o.alignment ),
  pixel_size(o.pixel_size)
{
    for (Transmissions::const_iterator i = o.transmissions.begin(), e = o.transmissions.end(); i != e; ++i)
    {
        transmissions.push_back( i->clone() );
    }
}

void PlaneConfig::registerNamedEntries()
{
    if ( purpose != PSFDisplay )
        push_back( pixel_size );
    push_back( three_d );
    if ( purpose != PSFDisplay ) {
        push_back( counts_per_photon );
        push_back( dark_current );
        push_back( alignment );
    }

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
void PlaneConfig::set_context( const input::Traits<Localization>& t, int fluorophore_count ) {
    set_fluorophore_count( fluorophore_count, false );
    three_d().set_context();
}

void PlaneConfig::set_context( const traits::Optics& o, int fluorophore_count, bool multilayer)
{
    set_fluorophore_count( fluorophore_count, multilayer );
    three_d().set_context();
}

void PlaneConfig::write_traits( traits::Optics& rv) const
{
    rv.projection_factory_ = alignment().get_projection_factory();
    rv.photon_response = counts_per_photon();
    rv.dark_current = dark_current();
    rv.tmc.clear();
    for ( Transmissions::const_iterator i = transmissions.begin(); i != transmissions.end(); ++i) {
        DEBUG("Set transmission of " << rv.tmc.size() << " to " << i->value() << " at " << &rv);
        rv.tmc.push_back( i->value() );
    }
    for (Direction d = Direction_First; d != Direction_2D; ++d)
        rv.set_depth_info( d, three_d().make_traits( d ) );
}

void PlaneConfig::read_traits( const traits::Optics& t )
{
    for (int i = 0; i < int(t.tmc.size()); ++i) {
        transmissions[i] = t.tmc[i];
    }
    if ( t.photon_response ) counts_per_photon = *t.photon_response;
    if ( t.dark_current ) dark_current = *t.dark_current;
    if ( t.depth_info(Direction_X) && t.depth_info(Direction_Y) ) {
        three_d.choose( t.depth_info(Direction_X)->config_name() );
        three_d().read_traits( *t.depth_info(Direction_X), *t.depth_info(Direction_Y) );
    }
}

void PlaneConfig::write_traits( input::Traits<Localization>& t ) const
{
}


MultiPlaneConfig::MultiPlaneConfig( PlaneConfig::Purpose purpose ) 
: simparm::Object("Optics", "Optical pathway properties"),
  purpose(purpose)
{
    DEBUG("Constructing " << this);
    layers.push_back( new PlaneConfig(0, purpose) );
    set_number_of_planes( 1 );
}

MultiPlaneConfig::~MultiPlaneConfig()
{
    DEBUG("Destructing " << this);
}

void MultiPlaneConfig::registerNamedEntries()
{
    for ( Layers::iterator i = layers.begin(); i != layers.end(); ++i) {
        i->registerNamedEntries();
        push_back( *i );
    }
}

void MultiPlaneConfig::set_number_of_planes(int number)
{
    while ( number > int( layers.size() ) ) {
        layers.push_back( new PlaneConfig( layers.size(), purpose ) );
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
MultiPlaneConfig::get_resolution() const {
    return layers[0].get_resolution();
}

void MultiPlaneConfig::write_traits( input::Traits<engine::ImageStack>& t ) const
{
    for (int i = 0; i < int( layers.size() ) && i < t.plane_count(); ++i) {
        layers[i].write_traits( t.optics(i) );
        t.image(i).set_resolution( layers[i].get_resolution() );
    }
}

void MultiPlaneConfig::write_traits( input::Traits<Localization>& t ) const
{
    layers[0].write_traits( t );
}

void MultiPlaneConfig::read_traits( const input::Traits<engine::ImageStack>& t )
{
    set_context(t);
    for (int i = 0; i < t.plane_count(); ++i) {
        layers[i].read_traits( t.optics(i) );
    }
}

void MultiPlaneConfig::set_context( const input::Traits<engine::ImageStack>& t ) 
{
    DEBUG( "Setting context on " << this );
    set_number_of_planes( t.plane_count() );
    for (int i = 0; i < t.plane_count(); ++i) {
        layers[i].set_context( t.optics(i), t.fluorophores.size(), (t.plane_count() > 1) );
    }
}

void MultiPlaneConfig::set_context( const input::Traits<Localization>& t ) 
{
    set_number_of_planes( 1 );
    layers[0].set_context( t, t.fluorophores.size() );
}


}
}
