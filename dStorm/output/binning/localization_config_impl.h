#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_IMPL_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_IMPL_H

#include <simparm/BoostUnits.hh>
#include "localization_config.h"
#include <sstream>
#include <simparm/Entry_Impl.hh>

namespace dStorm {
namespace output {
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
: simparm::Object( make_ident(row,column), make_desc(row,column) ), 
  row(row), column(column) ,
    resolution(axis + "Resolution", "Resolution in " + axis + " direction", Resolution::from_value(10)),
  range(range)
{
}

template <int Field>
LocalizationConfig<Field>::LocalizationConfig(std::string axis, int row, int column) 
: simparm::Object( make_ident(row,column), make_desc(row,column) ), 
  row(row), column(column) ,
    resolution(axis + "Resolution", "Resolution in " + axis + " direction", Resolution::from_value(10))
{
    resolution.helpID = "#Viewer_ResEnh";
    push_back(resolution); 
}

template <int Field>
LocalizationConfig<Field>::LocalizationConfig(const LocalizationConfig<Field>& o) 
: simparm::Object(o), row(o.row), column(o.column), resolution(o.resolution), range(o.range)
{ 
    if ( ! range.is_initialized() ) push_back(resolution); 
}

template <int Field>
std::auto_ptr<Scaled>
LocalizationConfig<Field>::make_scaled_binner() const 
{
    if ( range.is_initialized() ) {
        std::auto_ptr<output::binning::Scaled> rv( new output::binning::Localization<Field,ScaledToInterval>(*range, row, column) );
        return rv;
    } else {
        Resolution r1( resolution() );
        typename Traits::OutputType o( r1 * camera::pixel );
        typename traits::Scalar<Traits>::value_type r(o);
        std::auto_ptr<output::binning::Scaled> rv( 
            new output::binning::Localization<Field,ScaledByResolution>
                (r, row, column) );
        return rv;
    }
}

template <int Field>
std::auto_ptr<UserScaled>
LocalizationConfig<Field>::make_user_scaled_binner() const 
{
    if ( range.is_initialized() ) {
        std::auto_ptr< output::binning::Localization<Field,InteractivelyScaledToInterval> >
            o ( new output::binning::Localization<Field,InteractivelyScaledToInterval>(*range, row, column) );
        std::auto_ptr<output::binning::UserScaled> rv(o);
        return rv;
    } else 
        throw std::logic_error("Range not set");
}

template <int Field>
std::auto_ptr<Unscaled>
LocalizationConfig<Field>::make_unscaled_binner() const 
{
    return std::auto_ptr<output::binning::Unscaled>(new output::binning::Localization<Field, IsUnscaled>(row, column) );
}

template <int Field>
void LocalizationConfig<Field>::set_visibility(
    const input::Traits<dStorm::Localization>& t, bool unscaled_suffices
) {
    bool v;
    if ( unscaled_suffices )
        v = output::binning::Localization<Field, IsUnscaled>::can_work_with(t, row, column);
    else if ( range.is_initialized() )
        v = output::binning::Localization<Field, ScaledToInterval>::can_work_with(t, row, column);
    else 
        v = output::binning::Localization<Field, ScaledByResolution>::can_work_with(t, row, column);

    this->viewable = v;
}

}
}
}

#endif
