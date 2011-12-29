#include "BinnedLocalizations_strategies.h"
#include "BinnedLocalizations_strategies_config.h"
#include "BinnedLocalizations_strategies_impl.h"
#include "BinnedLocalizations.h"
#include <simparm/ChoiceEntry_Impl.hh>

#include <simparm/Entry_Impl.hh>
#include <dStorm/ImageTraits_impl.h>

#include <dStorm/output/binning/config.h>
#include <dStorm/output/binning/inversion.h>
#include <dStorm/output/binning/dummy_binner.h>

namespace dStorm {
namespace outputs {

static const char *axis_idents[] = { "X", "Y", "Z" };
static const char *axis_names[] = { "X axis", "Y axis", "Z axis" };

template <int Dim>
DimensionSelector<Dim>::DimensionSelector()
: simparm::Object("DimensionSelector", "Select dimensions to bin"),
  invert_y_axis("InvertYAxis", "Y zero at bottom"),
  use_z_axis("ThreeDImage", "Make 3D image")
{
    use_z_axis.userLevel = simparm::Object::Intermediate;
    for (int i = 0; i < Dim; ++i) {
        components.replace( i, new output::binning::FieldChoice(std::string(axis_idents[i]) + "Dimension", axis_names[i], 
            output::binning::ScaledByResolution, std::string(axis_idents[i])) );
        push_back( components[i] );
    }
    components.replace( Dim, new output::binning::FieldChoice("IDimension", "Intensity", output::binning::IsUnscaled, "I") );
    push_back( components[Dim] );
    push_back( invert_y_axis );
    if ( Dim > 2 ) {
        components[2].choose( "PositionZ" );
        components[2].userLevel = simparm::Object::Intermediate;
        push_back( use_z_axis );
    }

    components[0].choose( "PositionX" );
    components[1].choose( "PositionY" );
    components[Dim].choose( "Amplitude" );

    invert_y_axis.help = "Invert the Y axis so that the point with "
        "coordinates (0,0) is in the lower left corner of the image. Naturally, "
        "you will be burned at the stake for activating this option since it is "
        "unnatural in image processing.";
}

template <int Dim>
DimensionSelector<Dim>::DimensionSelector(const DimensionSelector& o) 
: simparm::Object(o), invert_y_axis(o.invert_y_axis), use_z_axis(o.use_z_axis)
{
    for (int i = 0; i < Dim+1; ++i) {
        components.replace(i,  o.components[i].clone() );
        push_back( components[i] );
    }
    push_back( invert_y_axis );
    push_back( use_z_axis );
}

template <int Dim>
DimensionSelector<Dim>::~DimensionSelector() {}

template <int Dim>
void DimensionSelector<Dim>::init() {
}

template <int Dim>
void DimensionSelector<Dim>::set_visibility(const input::Traits<Localization>& t) {
    for (int i = 0; i < Dim+1; ++i)
        components[i].set_visibility(t, (i == Dim));
}

template <int Dim>
std::auto_ptr< output::binning::Scaled > DimensionSelector<Dim>::make_x() const
    { return components[0].value().make_scaled_binner(); }

template <int Dim>
std::auto_ptr< output::binning::Scaled > DimensionSelector<Dim>::make_y() const {
    std::auto_ptr<output::binning::Scaled> y( components[1].value().make_scaled_binner() );
    if ( invert_y_axis() ) {
        boost::shared_ptr<output::binning::Scaled> base_y( y.release() );
        y.reset( new output::binning::Inversion<output::binning::Scaled>(base_y) );
    }
    return y;
}

template <int Dim>
std::auto_ptr< output::binning::Unscaled > DimensionSelector<Dim>::make_i() const 
    { return components[Dim].value().make_unscaled_binner(); }

template <int Dim>
std::auto_ptr<BinningStrategy<Dim> > DimensionSelector<Dim>::make() const
{
    boost::ptr_array< output::binning::Scaled, Dim> spatial;
    for (int i = 0; i < Dim; ++i)
        if ( i == 1 && invert_y_axis() ) {
            boost::shared_ptr<output::binning::Scaled> base_y( 
                components[i].value().make_scaled_binner().release() );
            spatial.replace( i, new output::binning::Inversion<output::binning::Scaled>(base_y) );
        } else if ( i == 2 && ! use_z_axis() ) {
            spatial.replace( i, new output::binning::Zero() );
        } else
            spatial.replace( i, components[i].value().make_scaled_binner() );
    return std::auto_ptr<BinningStrategy<Dim> >(
        new binning_strategy::ComponentWise<Dim>( spatial, make_i() ) );
}

template <int Dim>
std::auto_ptr< output::binning::Unscaled > DimensionSelector<Dim>::make_unscaled(int field) const
{
    return components[field].value().make_unscaled_binner();
}

template class DimensionSelector<2>;
template class DimensionSelector<3>;

}
}
