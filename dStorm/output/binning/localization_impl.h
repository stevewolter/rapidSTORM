#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_IMPL_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_IMPL_H

#include "../../debug.h"
#include "localization.h"
#include "../../display/DataSource.h"
#include <boost/units/cmath.hpp>

namespace dStorm {
namespace output {
namespace binning {

template <int Index>
Localization<Index,IsUnscaled,false>::Localization(int row, int column)
: scalar(row, column)
{
}

template <int Index>
Localization<Index,Bounded,false>::Localization(int row, int column)
: Base(row, column)
{
}

template <int Index>
Localization<Index,ScaledByResolution,false>::Localization(value res, int row, int column) 
: Base(row, column), scale(1.0f / quantity<typename value::unit_type,float>(res))
{
    DEBUG("Scale factor for field " << Index << ":" << row << ":" << column << " is " << scale);
}

template <int Index>
Localization<Index,ScaledToInterval,false>::Localization(float desired_range, int row, int column) 
: Base(row, column), desired_range(desired_range)
{
    DEBUG("Desired range for field " << Index << ":" << row << ":" << column << " is " << desired_range);
}

template <int Index>
Localization<Index,InteractivelyScaledToInterval,false>::Localization(float desired_range, int row, int column) 
: Base(desired_range, row, column)
{
    not_given.set();
}

template <int Index>
float Localization<Index,IsUnscaled,false>::bin_point( const dStorm::Localization& l ) const
{
    float rv = scalar.value(boost::fusion::at_c<Index>(l).value()).value();
    DEBUG("Coordinate index " << Index << " with unscaled binning returns " << rv);
    return rv;
}

template <int Index>
float Localization<Index,Bounded,false>::bin_point( const dStorm::Localization& l ) const
{
    value clipped = Base::scalar.value(boost::fusion::at_c<Index>(l).value());
    clipped = std::max( range[0], std::min( clipped, range[1] ) );
    float rv = (clipped - range[0]).value();
    DEBUG("Coordinate index " << Index << " with bounded binning returns " << rv);
    return rv;
}

template <int Index>
float
Localization<Index,ScaledByResolution,false>::bin_point( const dStorm::Localization& l ) const
{
    value clipped = Base::scalar.value(boost::fusion::at_c<Index>(l).value());
    clipped = std::max( Base::range[0], std::min( clipped, Base::range[1] ) );
    float rv = (clipped - Base::range[0]) * scale;
    DEBUG("Coordinate index " << Index << " with resolution-scaled binning returns " << rv << " with scale " << scale << " and base " << Base::range[0]);
    return rv;
}

template <int Index>
float
Localization<Index,InteractivelyScaledToInterval,false>::bin_point( const dStorm::Localization& l ) const
{
    if ( not_given.none() ) {
        DEBUG("Range given interactively, binning");
        return Base::bin_point(l);
    } else {
        DEBUG("Range not given, not binning");
        return 0;
    }
}

template <int Index>
void
Localization<Index,IsUnscaled,false>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    for (output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i, target += stride)
        *target = bin_point(*i);
}

template <int Index>
void
Localization<Index,Bounded,false>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    for (output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i, target += stride)
        *target = bin_point(*i);
}

template <int Index>
void
Localization<Index,ScaledByResolution,false>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    for (output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i, target += stride)
        *target = bin_point(*i);
}

template <int Index>
void
Localization<Index,InteractivelyScaledToInterval,false>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    if ( not_given.none() )
        return Base::bin_points(l, target, stride);
    else
        for (output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i, target += stride)
            *target = 0;
}

template <int Index>
void Localization<Index,IsUnscaled,false>::announce(const output::Output::Announcement& a) 
{ 
}

template <int Index>
void Localization<Index,Bounded,false>::announce(const output::Output::Announcement& a) 
{ 
    Base::announce(a);
    if ( ! Base::scalar.range(a).first.is_initialized() || ! Base::scalar.range(a).second.is_initialized() ) {
        std::stringstream message;
        if ( ! Base::scalar.range(a).first.is_initialized() ) message << "lower";
        if ( ! Base::scalar.range(a).first.is_initialized() && ! Base::scalar.range(a).second.is_initialized() )
            message << " and ";
        if ( ! Base::scalar.range(a).second.is_initialized() ) message << "upper";
        message << " bound not set for field " << Index;
        if ( TraitsType::Rows > 1 ) message << " in row " << Base::scalar.row();
        if ( TraitsType::Cols > 1 ) message << " in column " << Base::scalar.column();
        throw std::logic_error(message.str());
    }
    range[0] = *Base::scalar.range(a).first; 
    range[1] = *Base::scalar.range(a).second; 
}

template <int Index>
void Localization<Index,ScaledToInterval,false>::announce(const output::Output::Announcement& a) 
{ 
    Base::announce(a);

    recompute_scale();
}

template <int Index>
void Localization<Index,InteractivelyScaledToInterval,false>::announce(const output::Output::Announcement& a) 
{ 
    orig_range = Base::scalar.range(a);
    if ( orig_range.first.is_initialized() && ! user.test(0) ) 
        { Base::range[0] = * orig_range.first; not_given.reset(0); }
    if ( orig_range.second.is_initialized() && ! user.test(1) ) 
        { Base::range[1] = * orig_range.second; not_given.reset(1); }
    if ( not_given.none() ) Base::recompute_scale();
}

template <int Index>
float Localization<Index,Bounded,false>::get_size() const
{
    DEBUG("Interval for " << Index << " goes from " << range[0] << " " << range[1]);
    return (range[1] - range[0]).value();
}
template <int Index>
float Localization<Index,ScaledByResolution,false>::get_size() const
{
    float rv = Base::get_size() * scale.value();
    DEBUG( "Size for field " << Index << ":" << Base::scalar.row() << ":" << Base::scalar.column() << " is " << rv );
    return rv;
}
template <int Index>
float Localization<Index,ScaledToInterval,false>::get_size() const
{
    return desired_range;
}

template <int Index>
void Localization<Index,ScaledToInterval,false>::recompute_scale() 
{
    Base::scale = desired_range / typename Base::InvScale(Base::range[1] - Base::range[0]);
}

template <int Index>
traits::ImageResolution Localization<Index,IsUnscaled,false>::resolution() const
{
    traits::ImageResolution rv( value::from_value(1.0) / camera::pixel );
    assert( rv.unit_symbol == symbol_string(typename value::unit_type()) );
    return rv;
}

template <int Index>
traits::ImageResolution Localization<Index,ScaledByResolution,false>::resolution() const
{
    traits::ImageResolution rv( static_cast<typename Base::Scale::value_type>(1.0) / scale / camera::pixel );
    assert( rv.unit_symbol == symbol_string(typename value::unit_type()) );
    return rv;
}

template <int Index>
bool Localization<Index,IsUnscaled,false>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    traits::Scalar<TraitsType> s(row, column);
    return s.is_given(t);
} 

template <int Index>
bool Localization<Index,Bounded,false>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    traits::Scalar<TraitsType> s(row, column);
    return Base::can_work_with(t, row, column) && s.range(t).first.is_initialized() && s.range(t).second.is_initialized();
}

template <int Index>
bool Localization<Index,ScaledByResolution,false>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    return Base::can_work_with(t, row, column);
}

template <int Index>
bool Localization<Index,ScaledToInterval,false>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    return Base::can_work_with(t, row, column);
}

template <int Index>
bool Localization<Index,InteractivelyScaledToInterval,false>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    return traits::Scalar<TraitsType>(row, column).is_given(t);
}

template <int Index>
std::pair< float, float > Localization<Index,Bounded,false>::get_minmax() const
{
    return std::make_pair( range[0].value(), range[1].value() );
}

template <int Index>
std::pair< float, float > Localization<Index,ScaledByResolution,false>::get_minmax() const
{
    return std::make_pair( Base::range[0] * scale, Base::range[1] * scale );
}

template <int Index>
bool Localization<Index,InteractivelyScaledToInterval,false>::is_bounded() const
{
    return not_given.none();
}

template <int Index>
double Localization<Index,Bounded,false>::reverse_mapping( float mapped_value ) const
{
    return mapped_value + range[0].value();
}

template <int Index>
double Localization<Index,ScaledByResolution,false>::reverse_mapping( float mapped_value ) const
{
    return value(mapped_value / scale + Base::range[0]).value();
}

template <int Index>
void Localization<Index,InteractivelyScaledToInterval,false>::set_user_limit( bool lower_limit, const std::string& s )
{
    int i = (lower_limit) ? 0 : 1;
    if ( s == "" ) {
        boost::optional<value>& bound = (lower_limit) ? orig_range.first : orig_range.second;
        if ( bound.is_initialized() )
            Base::range[i] = *bound;
        else 
            not_given.set(i);
        user.reset(i);
    } else {
        typename value::value_type f;
        std::stringstream ss(s);
        ss >> f;
        Base::range[i] = value::from_value(f);
        user.set(i);
        not_given.reset(i);
    }
    if ( not_given.none() ) Base::recompute_scale();
}

template <int Index>
display::KeyDeclaration Localization<Index,InteractivelyScaledToInterval,false>::key_declaration() const
{
    traits::ImageResolution resolution = this->resolution();
    dStorm::display::KeyDeclaration rv(
        resolution.unit_symbol,
        resolution.unit_name, 1);
    rv.can_set_lower_limit = rv.can_set_upper_limit = true;
    std::stringstream s[2];
    for (int i = 0; i < 2; ++i)
        if ( ! not_given[i] ) s[i] << Base::range[i].value();
    rv.lower_limit = s[0].str();
    rv.upper_limit = s[1].str();
    return rv;
}

}
}
}

#endif
