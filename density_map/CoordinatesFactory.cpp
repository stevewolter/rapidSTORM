#include "CoordinatesFactory.h"
#include "Coordinates.h"

#include <dStorm/image/MetaInfo.h>

#include <dStorm/output/binning/config.h>
#include <dStorm/output/binning/inversion.h>
#include <dStorm/output/binning/dummy_binner.h>
#include <dStorm/output/binning/constant_binner.h>

namespace dStorm {
namespace density_map {

static const char *axis_idents[] = { "X", "Y", "Z" };
static const char *axis_names[] = { "X axis", "Y axis", "Z axis" };

template <int Dim>
CoordinatesFactory<Dim>::CoordinatesFactory()
: name_object("DimensionSelector", "Select dimensions to bin"),
  invert_y_axis("InvertYAxis", "Y zero at bottom"),
  use_z_axis("ThreeDImage", "Make 3D image")
{
    use_z_axis.set_user_level( simparm::Intermediate );
    for (int i = 0; i < Dim; ++i) {
        components.replace( i, new output::binning::FieldChoice(std::string(axis_idents[i]) + "Dimension", axis_names[i], 
            output::binning::ScaledByResolution, std::string(axis_idents[i])) );
    }
    components.replace( Dim, new output::binning::FieldChoice("IDimension", "Intensity", output::binning::IsUnscaled, "I") );
    components[Dim].addChoice( dStorm::output::binning::make_constant_binner_config() );
    if ( Dim > 2 ) {
        components[2].choose( "PositionZ" );
        components[2].set_user_level( simparm::Intermediate );
    }

    components[0].choose( "PositionX" );
    components[1].choose( "PositionY" );
    components[Dim].choose( "Amplitude" );

    invert_y_axis.setHelp( "Invert the Y axis so that the point with "
        "coordinates (0,0) is in the lower left corner of the image. Naturally, "
        "you will be burned at the stake for activating this option since it is "
        "unnatural in image processing." );
}

template <int Dim>
CoordinatesFactory<Dim>::~CoordinatesFactory() {}

template <int Dim>
void CoordinatesFactory<Dim>::attach_ui( simparm::NodeHandle at ) {
    listening[0] = invert_y_axis.value.notify_on_value_change( value_change );
    listening[1] = use_z_axis.value.notify_on_value_change( value_change );

    simparm::NodeHandle r = name_object.attach_ui( at );
    for (int i = 0; i < Dim+1; ++i) {
        components[i].attach_ui( r );
    }
    invert_y_axis.attach_ui( r );
    if ( Dim > 2 ) 
        use_z_axis.attach_ui( r );
}

template <int Dim>
void CoordinatesFactory<Dim>::init() {
}

template <int Dim>
void CoordinatesFactory<Dim>::set_visibility(const input::Traits<Localization>& t) {
    for (int i = 0; i < Dim+1; ++i)
        components[i].set_visibility(t, (i == Dim));
}

template <int Dim>
std::auto_ptr< output::binning::Scaled > CoordinatesFactory<Dim>::make_x() const
    { return components[0]().make_scaled_binner(); }

template <int Dim>
std::auto_ptr< output::binning::Scaled > CoordinatesFactory<Dim>::make_y() const {
    std::auto_ptr<output::binning::Scaled> y( components[1]().make_scaled_binner() );
    if ( invert_y_axis() ) {
        boost::shared_ptr<output::binning::Scaled> base_y( y.release() );
        y.reset( new output::binning::Inversion<output::binning::Scaled>(base_y) );
    }
    return y;
}

template <int Dim>
std::auto_ptr< output::binning::Unscaled > CoordinatesFactory<Dim>::make_i() const 
    { return components[Dim]().make_unscaled_binner(); }

template <int Dim>
std::auto_ptr< Coordinates<Dim> > CoordinatesFactory<Dim>::make() const
{
    boost::ptr_array< output::binning::Scaled, Dim> spatial;
    for (int i = 0; i < Dim; ++i)
        if ( i == 1 && invert_y_axis() ) {
            boost::shared_ptr<output::binning::Scaled> base_y( 
                components[i]().make_scaled_binner().release() );
            spatial.replace( i, new output::binning::Inversion<output::binning::Scaled>(base_y) );
        } else if ( i == 2 && ! use_z_axis() ) {
            spatial.replace( i, new output::binning::Zero() );
        } else
            spatial.replace( i, components[i]().make_scaled_binner() );
    return std::auto_ptr< Coordinates<Dim> >(
        new Coordinates<Dim>( spatial, make_i() ) );
}

template <int Dim>
std::auto_ptr< output::binning::Unscaled > CoordinatesFactory<Dim>::make_unscaled(int field) const
{
    return components[field]().make_unscaled_binner();
}

template <int Dim>
void CoordinatesFactory<Dim>::add_listener( simparm::BaseAttribute::Listener& l ) {
    for (int i = 0; i < Dim+1; ++i)
        components[i].add_listener(l);
    value_change.connect(l);
}

template class CoordinatesFactory<2>;
template class CoordinatesFactory<3>;

}
}
