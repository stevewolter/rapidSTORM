#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_IMPL_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_IMPL_H

#include "simparm/BoostUnits.h"
#include "binning/localization_config.h"
#include "binning/localization_impl.h"
#include "binning/binning.hpp"
#include "binning/always_failing_binner.h"
#include <sstream>

namespace dStorm {
namespace binning {

inline std::string get_label(dStorm::localization::PositionX) { return "PositionX"; }
inline std::string get_label(dStorm::localization::PositionY) { return "PositionY"; }
inline std::string get_label(dStorm::localization::PositionZ) { return "PositionZ"; }
inline std::string get_label(dStorm::localization::PositionUncertaintyX) { return "PositionUncertaintyX"; }
inline std::string get_label(dStorm::localization::PositionUncertaintyY) { return "PositionUncertaintyY"; }
inline std::string get_label(dStorm::localization::PositionUncertaintyZ) { return "PositionUncertaintyZ"; }
inline std::string get_label(dStorm::localization::PSFWidthX) { return "PSFWidthX"; }
inline std::string get_label(dStorm::localization::PSFWidthY) { return "PSFWidthY"; }
inline std::string get_label(dStorm::localization::ImageNumber) { return "ImageNumber"; }
inline std::string get_label(dStorm::localization::Amplitude) { return "Amplitude"; }
inline std::string get_label(dStorm::localization::TwoKernelImprovement) { return "TwoKernelImprovement"; }
inline std::string get_label(dStorm::localization::FitResidues) { return "FitResidues"; }
inline std::string get_label(dStorm::localization::Fluorophore) { return "Fluorophore"; }
inline std::string get_label(dStorm::localization::LocalBackground) { return "LocalBackground"; }
inline std::string get_label(dStorm::localization::CoefficientOfDetermination) { return "CoefficientOfDetermination"; }
inline std::string get_label(dStorm::localization::Molecule) { return "Molecule"; }

template <typename Tag>
LocalizationConfig<Tag>::LocalizationConfig(std::string axis, float range) 
: FieldConfig( get_label(Tag()), Tag::get_desc() ), 
  use_resolution( false ),
  resolution(axis + "Resolution", "Resolution in " + axis + " direction", Resolution::from_value(10)),
  range(range)
{
}

template <typename Tag>
LocalizationConfig<Tag>::LocalizationConfig(std::string axis, bool use_resolution) 
: FieldConfig( get_label(Tag()), Tag::get_desc() ), 
  use_resolution( use_resolution ),
  resolution(axis + "Resolution", "Resolution in " + axis + " direction", Resolution::from_value(10))
{
    resolution.setHelpID( "#Viewer_ResEnh" );
}

template <typename Tag>
std::auto_ptr<Scaled>
LocalizationConfig<Tag>::make_scaled_binner() const 
{
    if ( range.is_initialized() ) {
        typedef Localization<Tag,ScaledToInterval> T;
        return std::auto_ptr<Scaled>( 
            new BinningAdapter<T,Scaled>( T(*range) ) );
    } else {
        typedef Localization<Tag,ScaledByResolution> T;
        Resolution r1( resolution() );
        typename Tag::OutputType o( r1 * camera::pixel );
        typename Tag::ValueType r(o);
        return std::auto_ptr<Scaled>( 
            new BinningAdapter<T,Scaled>( T(r) ) );
    }
}

template <typename Tag>
std::auto_ptr<UserScaled>
LocalizationConfig<Tag>::make_user_scaled_binner() const 
{
    if ( range.is_initialized() ) {
        std::auto_ptr< binning::Localization<Tag,InteractivelyScaledToInterval> >
            o ( new binning::Localization<Tag,InteractivelyScaledToInterval>(*range) );
        typedef binning::Localization<Tag,InteractivelyScaledToInterval> T;
        std::auto_ptr<UserScaled> rv( new BinningAdapter< T, UserScaled >( 
            T(*range) ) );
        return rv;
    } else 
        throw std::logic_error("Range not set");
}

template <typename Tag>
std::auto_ptr<Unscaled>
LocalizationConfig<Tag>::make_unscaled_binner() const 
{
    typedef binning::Localization<Tag,IsUnscaled> T;
    return make_BinningAdapter<Unscaled>( T());
}

namespace {

template <int Dimension>
std::auto_ptr<Unscaled> make_uncertainty_binner(localization::Position<Dimension> tag) {
    return make_BinningAdapter<Unscaled>(
            binning::Localization<localization::PositionUncertainty<Dimension>, IsUnscaled>());
}

template <typename Tag>
std::auto_ptr<Unscaled> make_uncertainty_binner(Tag tag) {
    return std::auto_ptr<Unscaled>(new AlwaysFailingUnscaled());
}

}  // namespace

template <typename Tag>
std::auto_ptr<Unscaled>
LocalizationConfig<Tag>::make_uncertainty_binner() const 
{
    return binning::make_uncertainty_binner(Tag());
}

template <typename Tag>
void LocalizationConfig<Tag>::set_visibility(
    const input::Traits<dStorm::Localization>& t, bool unscaled_suffices
) {
    bool v;
    if ( unscaled_suffices )
        v = binning::Localization<Tag, IsUnscaled>::can_work_with(t);
    else if ( range.is_initialized() )
        v = binning::Localization<Tag, ScaledToInterval>::can_work_with(t);
    else 
        v = binning::Localization<Tag, ScaledByResolution>::can_work_with(t);

    set_viewability(v);
}

template <typename Tag>
void LocalizationConfig<Tag>::add_listener( simparm::BaseAttribute::Listener& l )
{
    change.connect( l );
}


}
}

#endif
