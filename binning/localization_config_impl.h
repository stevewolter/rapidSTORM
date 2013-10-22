#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_IMPL_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_IMPL_H

#include <simparm/BoostUnits.h>
#include "localization_config.h"
#include "binning.hpp"
#include <sstream>

namespace dStorm {
namespace binning {

template <int Field> std::string get_label();

template <int Field>
LocalizationConfig<Field>::LocalizationConfig(std::string axis, float range) 
: FieldConfig( get_label<Field>(), Traits::get_desc() ), 
  use_resolution( false ),
  resolution(axis + "Resolution", "Resolution in " + axis + " direction", Resolution::from_value(10)),
  range(range)
{
}

template <int Field>
LocalizationConfig<Field>::LocalizationConfig(std::string axis, bool use_resolution) 
: FieldConfig( get_label<Field>(), Traits::get_desc() ), 
  use_resolution( use_resolution ),
  resolution(axis + "Resolution", "Resolution in " + axis + " direction", Resolution::from_value(10))
{
    resolution.setHelpID( "#Viewer_ResEnh" );
}

template <int Field>
std::auto_ptr<Scaled>
LocalizationConfig<Field>::make_scaled_binner() const 
{
    if ( range.is_initialized() ) {
        typedef Localization<Field,ScaledToInterval> T;
        return std::auto_ptr<Scaled>( 
            new BinningAdapter<T,Scaled>( T(*range) ) );
    } else {
        typedef Localization<Field,ScaledByResolution> T;
        Resolution r1( resolution() );
        typename Traits::OutputType o( r1 * camera::pixel );
        typename Traits::ValueType r(o);
        return std::auto_ptr<Scaled>( 
            new BinningAdapter<T,Scaled>( T(r) ) );
    }
}

template <int Field>
std::auto_ptr<UserScaled>
LocalizationConfig<Field>::make_user_scaled_binner() const 
{
    if ( range.is_initialized() ) {
        std::auto_ptr< binning::Localization<Field,InteractivelyScaledToInterval> >
            o ( new binning::Localization<Field,InteractivelyScaledToInterval>(*range) );
        typedef binning::Localization<Field,InteractivelyScaledToInterval> T;
        std::auto_ptr<UserScaled> rv( new BinningAdapter< T, UserScaled >( 
            T(*range) ) );
        return rv;
    } else 
        throw std::logic_error("Range not set");
}

template <int Field>
std::auto_ptr<Unscaled>
LocalizationConfig<Field>::make_unscaled_binner() const 
{
    typedef binning::Localization<Field,IsUnscaled> T;
    return make_BinningAdapter<Unscaled>( T());
}

template <int Field>
void LocalizationConfig<Field>::set_visibility(
    const input::Traits<dStorm::Localization>& t, bool unscaled_suffices
) {
    bool v;
    if ( unscaled_suffices )
        v = binning::Localization<Field, IsUnscaled>::can_work_with(t);
    else if ( range.is_initialized() )
        v = binning::Localization<Field, ScaledToInterval>::can_work_with(t);
    else 
        v = binning::Localization<Field, ScaledByResolution>::can_work_with(t);

    set_viewability(v);
}

template <int Field>
void LocalizationConfig<Field>::add_listener( simparm::BaseAttribute::Listener& l )
{
    change.connect( l );
}


}
}

#endif
