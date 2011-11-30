#include "BinnedLocalizations_strategies.h"
#include "BinnedLocalizations_strategies_config.h"
#include "BinnedLocalizations_strategies_impl.h"
#include "BinnedLocalizations.h"
#include <simparm/ChoiceEntry_Impl.hh>

#include <simparm/Entry_Impl.hh>
#include <dStorm/ImageTraits_impl.h>

#include <dStorm/output/binning/config.h>
#include <dStorm/output/binning/inversion.h>

namespace dStorm {
namespace outputs {

static const char *axis_idents[] = { "X", "Y", "I" };
static const char *axis_names[] = { "X axis", "Y axis", "Intensity" };

DimensionSelector::DimensionSelector()
: simparm::Object("DimensionSelector", "Select dimensions to bin"),
  invert_y_axis("InvertYAxis", "Y zero at bottom")
{
    for (int i = 0; i < 3; ++i) {
        components.replace( i, new output::binning::FieldChoice(std::string(axis_idents[i]) + "Dimension", axis_names[i], 
            (i == 2) ? output::binning::IsUnscaled : output::binning::ScaledByResolution, std::string(axis_idents[i])) );
        push_back( components[i] );
        if ( i == 2 )
            push_back( invert_y_axis );
    }
    components[0].choose( "PositionX" );
    components[1].choose( "PositionY" );
    components[2].choose( "Amplitude" );

    invert_y_axis.help = "Invert the Y axis so that the point with "
        "coordinates (0,0) is in the lower left corner of the image. Naturally, "
        "you will be burned at the stake for activating this option since it is "
        "unnatural in image processing.";
}

DimensionSelector::DimensionSelector(const DimensionSelector& o) 
: simparm::Object(o), invert_y_axis(o.invert_y_axis)
{
    for (int i = 0; i < 3; ++i) {
        components.replace(i,  o.components[i].clone() );
        push_back( components[i] );
        if ( i == 2 )
            push_back( invert_y_axis );
    }
}

DimensionSelector::~DimensionSelector() {}

void DimensionSelector::init() {
}

void DimensionSelector::set_visibility(const input::Traits<Localization>& t) {
    for (int i = 0; i < 3; ++i)
        components[i].set_visibility(t, (i == 2));
}
std::auto_ptr< output::binning::Scaled > DimensionSelector::make_x() const
    { return components[0].value().make_scaled_binner(); }
std::auto_ptr< output::binning::Scaled > DimensionSelector::make_y() const {
    std::auto_ptr<output::binning::Scaled> y( components[1].value().make_scaled_binner() );
    if ( invert_y_axis() ) {
        boost::shared_ptr<output::binning::Scaled> base_y( y.release() );
        y.reset( new output::binning::Inversion<output::binning::Scaled>(base_y) );
    }
    return y;
}
std::auto_ptr< output::binning::Unscaled > DimensionSelector::make_i() const 
    { return components[2].value().make_unscaled_binner(); }
std::auto_ptr<BinningStrategy> DimensionSelector::make() const
{
    return std::auto_ptr<BinningStrategy>(
        new binning_strategy::ThreeComponent(
            make_x(), make_y(), make_i() ) );
}
std::auto_ptr< output::binning::Unscaled > DimensionSelector::make_unscaled(int field) const
{
    return components[field].value().make_unscaled_binner();
}

}
}
