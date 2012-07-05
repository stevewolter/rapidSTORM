#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_IMPL_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_IMPL_H

#include <simparm/BoostUnits.h>
#include "localization_config.h"
#include "binning.hpp"
#include <sstream>

namespace dStorm {
namespace binning {

inline std::string dimen_name(int d) {
    switch (d) {
        case 0: return "X";
        case 1: return "Y";
        case 2: return "Z";
        case 3: return "A";
        default:
            throw std::logic_error("Ran out of dimension names");
    }
}

template <int Field>
std::string LocalizationConfig<Field>::make_ident(int r, int c) {
        std::stringstream result;
        result << Traits::get_ident();
        if ( Traits::Rows > 1 ) result << dimen_name(r);
        if ( Traits::Cols > 1 ) result << dimen_name(c);
        return result.str();
    }

template <int Field>
std::string LocalizationConfig<Field>::make_desc(int r, int c) {
        std::stringstream result;
        result << Traits::get_desc();
        if ( Traits::Rows > 1 && Traits::Cols > 1 )
            result << " " << dimen_name(r) << dimen_name(c);
        else if ( Traits::Rows > 1 )
            result << " " << dimen_name(r);
        else if ( Traits::Cols > 1 )
            result << " " << dimen_name(c);
        return result.str();
    }

template <int Field>
LocalizationConfig<Field>::LocalizationConfig(std::string axis, float range, int row, int column) 
: FieldConfig( make_ident(row,column), make_desc(row,column) ), 
  use_resolution( false ),
  row(row), column(column) ,
    resolution(axis + "Resolution", "Resolution in " + axis + " direction", Resolution::from_value(10)),
  range(range)
{
}

template <int Field>
LocalizationConfig<Field>::LocalizationConfig(std::string axis, bool use_resolution, int row, int column) 
: FieldConfig( make_ident(row,column), make_desc(row,column) ), 
  use_resolution( use_resolution ),
  row(row), column(column) ,
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
            new BinningAdapter<T,Scaled>( T(*range, row, column) ) );
    } else {
        typedef Localization<Field,ScaledByResolution> T;
        Resolution r1( resolution() );
        typename Traits::OutputType o( r1 * camera::pixel );
        typename traits::Scalar<Traits>::value_type r(o);
        return std::auto_ptr<Scaled>( 
            new BinningAdapter<T,Scaled>( T(r, row, column) ) );
    }
}

template <int Field>
std::auto_ptr<UserScaled>
LocalizationConfig<Field>::make_user_scaled_binner() const 
{
    if ( range.is_initialized() ) {
        std::auto_ptr< binning::Localization<Field,InteractivelyScaledToInterval> >
            o ( new binning::Localization<Field,InteractivelyScaledToInterval>(*range, row, column) );
        typedef binning::Localization<Field,InteractivelyScaledToInterval> T;
        std::auto_ptr<UserScaled> rv( new BinningAdapter< T, UserScaled >( 
            T(*range,row,column) ) );
        return rv;
    } else 
        throw std::logic_error("Range not set");
}

template <int Field>
std::auto_ptr<Unscaled>
LocalizationConfig<Field>::make_unscaled_binner() const 
{
    typedef binning::Localization<Field,IsUnscaled> T;
    return make_BinningAdapter<Unscaled>( T(row, column));
}

template <int Field>
void LocalizationConfig<Field>::set_visibility(
    const input::Traits<dStorm::Localization>& t, bool unscaled_suffices
) {
    bool v;
    if ( unscaled_suffices )
        v = binning::Localization<Field, IsUnscaled>::can_work_with(t, row, column);
    else if ( range.is_initialized() )
        v = binning::Localization<Field, ScaledToInterval>::can_work_with(t, row, column);
    else 
        v = binning::Localization<Field, ScaledByResolution>::can_work_with(t, row, column);

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