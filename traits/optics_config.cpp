#include "simparm/Node.h"
#include "debug.h"
#include "traits/optics_config.h"
#include <boost/lexical_cast.hpp>
#include "localization/Traits.h"
#include "traits/ProjectionFactory.h"

namespace dStorm {
namespace traits {

using namespace boost::units;

struct PlaneConfig::TransmissionEntry
: public simparm::Entry<double> {
    TransmissionEntry( int index ) 
        : simparm::Entry<double>(
            "Transmission" + boost::lexical_cast<std::string>(index),
            "Transmission of fluorophore " + boost::lexical_cast<std::string>(index),
            1 ) 
    {
        this->setHelpID( "TransmissionCoefficient" );
    }
    TransmissionEntry* clone() const { return new TransmissionEntry(*this); }
    simparm::BaseAttribute::ConnectionStore listener;
};

PlaneConfig::PlaneConfig(int number, Purpose purpose)
: name_object("InputLayer" + boost::lexical_cast<std::string>(number), 
                  "Input layer " + boost::lexical_cast<std::string>(number+1)),
  purpose(purpose),
  three_d("ThreeD"),
  counts_per_photon( "CountsPerPhoton", boost::optional< camera_response >() ),
  dark_current( "DarkCurrent", boost::optional< boost::units::quantity<boost::units::camera::intensity, int > >() ),
  alignment( "Alignment" ),
  pixel_size("PixelSizeInNM", PixelSize::Constant(107.0f * si::nanometre / camera::pixel))
{
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
    alignment.set_user_level(simparm::Expert);

    transmissions.push_back( new TransmissionEntry(0) );
    transmissions.back().hide();

    counts_per_photon.set_user_level( simparm::Intermediate );
    dark_current.set_user_level( simparm::Intermediate );
}

PlaneConfig::PlaneConfig( const PlaneConfig& o )
: name_object( o.name_object ),
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

PlaneConfig::~PlaneConfig() {}

void PlaneConfig::attach_ui( simparm::NodeHandle at )
{
    simparm::NodeHandle r = name_object.attach_ui( at );
    current_ui = r;
    if ( purpose != PSFDisplay ) {
        listening[0] = pixel_size.value.notify_on_value_change( ui_element_changed );
        pixel_size.attach_ui( r );
    }
    three_d.attach_ui( r );
    listening[4] = three_d.value.notify_on_value_change( ui_element_changed );
    if ( purpose != PSFDisplay ) {
        listening[1] = counts_per_photon.value.notify_on_value_change( ui_element_changed );
        listening[2] = dark_current.value.notify_on_value_change( ui_element_changed );
        listening[3] = alignment.value.notify_on_value_change( ui_element_changed );
        counts_per_photon.attach_ui( r );
        dark_current.attach_ui( r );
        alignment.attach_ui( r );
    }

    for (Transmissions::iterator i = transmissions.begin(), e = transmissions.end(); i != e; ++i) {
        i->attach_ui( r );
        i->listener = i->value.notify_on_value_change( ui_element_changed );
    }
}

void PlaneConfig::set_fluorophore_count( int fluorophore_count, bool multiplane )
{
    while ( int(transmissions.size()) < fluorophore_count ) {
        transmissions.push_back( new TransmissionEntry(transmissions.size()) );
        transmissions.back().attach_ui( current_ui );
    }

    Transmissions::iterator mark = transmissions.begin() + fluorophore_count;

    std::for_each( transmissions.begin(), mark,
        boost::bind( &simparm::Object::set_visibility, _1, fluorophore_count > 1 || multiplane ) );
    std::for_each( mark, transmissions.end(),
        boost::bind( &simparm::Object::set_visibility, _1, false ) );
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
        transmissions[i].value = t.tmc[i];
    }
    if ( t.photon_response ) counts_per_photon = *t.photon_response;
    if ( t.dark_current ) dark_current = *t.dark_current;
    if ( t.depth_info(Direction_X) && t.depth_info(Direction_Y) ) {
        three_d.choose( t.depth_info(Direction_X)->config_name() );
        three_d().read_traits( *t.depth_info(Direction_X), *t.depth_info(Direction_Y) );
    }
}

MultiPlaneConfig::MultiPlaneConfig( PlaneConfig::Purpose purpose ) 
: name_object("Optics", "Optical pathway properties"),
  purpose(purpose)
{
    DEBUG("Constructing " << this);
    layers.push_back( new PlaneConfig(0, purpose) );
    set_number_of_planes( 1 );
}

MultiPlaneConfig::MultiPlaneConfig(const MultiPlaneConfig& o) 
: name_object(o.name_object), layers(o.layers),
  purpose(o.purpose)
{
}

MultiPlaneConfig::~MultiPlaneConfig()
{
    DEBUG("Destructing " << this);
}

void MultiPlaneConfig::attach_ui( simparm::NodeHandle at )
{
    simparm::NodeHandle r = name_object.attach_ui( at );
    current_ui = r;
    for ( Layers::iterator i = layers.begin(); i != layers.end(); ++i) {
        i->notify_on_any_change( boost::ref(ui_element_listener) );
        i->attach_ui( r );
    }
}

void MultiPlaneConfig::set_number_of_planes(int number)
{
    while ( number > int( layers.size() ) ) {
        layers.push_back( new PlaneConfig( layers.size(), purpose ) );
        if ( current_ui ) {
            layers.back().notify_on_any_change( boost::ref(ui_element_listener) );
            layers.back().attach_ui( current_ui );
        }
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
        layers[i].set_context( t.optics(i), t.fluorophore_count, (t.plane_count() > 1) );
    }
}

bool MultiPlaneConfig::ui_is_attached() { return current_ui && current_ui->isActive(); }

}
}
