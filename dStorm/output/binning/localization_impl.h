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
Localization<Index,IsUnscaled>::Localization(int row, int column)
: scalar(row, column)
{
}

template <int Index>
Localization<Index,Bounded>::Localization(int row, int column)
: Base(row, column), discard( true )
{
}

template <int Index>
Localization<Index,ScaledByResolution>::Localization(value res, int row, int column) 
: Base(row, column), scale(1.0f / quantity<typename value::unit_type,float>(res))
{
    DEBUG("Scale factor for field " << Index << ":" << row << ":" << column << " is " << scale);
}

template <int Index>
Localization<Index,ScaledToInterval>::Localization(float desired_range, int row, int column) 
: Base(row, column), desired_range(desired_range)
{
    DEBUG("Desired range for field " << Index << ":" << row << ":" << column << " is " << desired_range);
}

template <int Index>
Localization<Index,InteractivelyScaledToInterval>::Localization(float desired_range, int row, int column) 
: Base(desired_range, row, column)
{
    not_given.set();
}

template <int Index>
typename Localization<Index,IsUnscaled>::value
Localization<Index,IsUnscaled>::bin_naively( const dStorm::Localization& l ) const
{
    return scalar.value(boost::fusion::at_c<Index>(l).value());
}

template <int Index>
bool
Localization<Index,Bounded>::in_range( value v ) const
{
    return (v >= range[0] && v <= range[1]);
}

template <int Index>
float
Localization<Index,Bounded>::scale( value v ) const
{
    return (v - range[0]).value();
}

template <int Index>
typename Localization<Index,Bounded>::value
Localization<Index,Bounded>::clip( value v ) const
{
    return std::max( range[0], std::min( v, range[1] ) );
}


template <int Index>
boost::optional<float>
Localization<Index,IsUnscaled>::bin_point( const dStorm::Localization& l ) const
{
    return bin_naively(l).value(); 
}

template <int Index>
boost::optional<float>
Localization<Index,IsUnscaled>::get_uncertainty( const dStorm::Localization& l ) const
{
    return scalar.value(boost::fusion::at_c<Index>(l).uncertainty()).value();
}

template <int Index>
boost::optional<float>
Localization<Index,Bounded>::bin_point( const dStorm::Localization& l ) const
{
    value v = this->bin_naively(l);
    if ( discard ) {
        if ( ! in_range(v) )
            return boost::optional<float>();
        else
            return scale(v);
    } else {
        return scale( clip(v) );
    }
}

template <int Index>
boost::optional<float>
Localization<Index,ScaledByResolution>::bin_point( const dStorm::Localization& l ) const
{
    boost::optional<float> f = Base::bin_point( l );
    if ( f.is_initialized() )
        return *f * scale.value();
    else
        return f;
}

template <int Index>
boost::optional<float>
Localization<Index,ScaledByResolution>::get_uncertainty( const dStorm::Localization& l ) const
{
    boost::optional<float> f = Base::get_uncertainty( l );
    if ( f.is_initialized() ) return *f * scale.value(); else return f;
}

template <int Index>
boost::optional<float>
Localization<Index,InteractivelyScaledToInterval>::bin_point( const dStorm::Localization& l ) const
{
    if ( not_given.none() ) {
        return Base::bin_point(l);
    } else {
        return boost::optional<float>();
    }
}

template <int Index>
boost::optional<float>
Localization<Index,InteractivelyScaledToInterval>::get_uncertainty( const dStorm::Localization& l ) const
{
    if ( not_given.none() ) {
        return Base::get_uncertainty(l);
    } else {
        return boost::optional<float>();
    }
}

template <int Index>
int
Localization<Index,IsUnscaled>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    float *t = target;
    for (output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i, t += stride)
        *t = bin_naively(*i).value();
    return (t - target) / stride;
}

template <int Index>
int
Localization<Index,Bounded>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    float *t = target;
    for (output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i)
    {
        value v = this->bin_naively( *i );
        if ( ! discard || in_range( v ) ) {
            *target = scale( (!discard) ? clip(v) : v );
            target += stride;
        }
    }
    return (t - target) / stride;
}

template <int Index>
int
Localization<Index,ScaledByResolution>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    int c = Base::bin_points(l, target, stride);
    for (int i = 0; i < c; ++i)
        target[i * stride] *= scale.value();
    return c;
}

template <int Index>
int
Localization<Index,InteractivelyScaledToInterval>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    if ( not_given.none() )
        return Base::bin_points(l, target, stride);
    else {
        return 0;
    }
}

template <int Index>
void Localization<Index,IsUnscaled>::announce(const output::Output::Announcement& a) 
{ 
}

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

template <int Index>
void Localization<Index,Bounded>::announce(const output::Output::Announcement& a) 
{ 
    Base::announce(a);
    if ( ! Base::scalar.range(a).first.is_initialized() || ! Base::scalar.range(a).second.is_initialized() ) {
        std::stringstream message;
        if ( ! Base::scalar.range(a).first.is_initialized() ) message << "lower";
        if ( ! Base::scalar.range(a).first.is_initialized() && ! Base::scalar.range(a).second.is_initialized() )
            message << " and ";
        if ( ! Base::scalar.range(a).second.is_initialized() ) message << "upper";
        message << " bound not set for field " << TraitsType::get_ident();
        if ( TraitsType::Rows > 1 ) message << " in " << dimen_name( Base::scalar.row() );
        throw std::runtime_error(message.str());
    }
    range[0] = *Base::scalar.range(a).first; 
    range[1] = *Base::scalar.range(a).second; 
}

template <int Index>
void Localization<Index,ScaledToInterval>::announce(const output::Output::Announcement& a) 
{ 
    Base::announce(a);

    recompute_scale();
}

template <int Index>
void Localization<Index,InteractivelyScaledToInterval>::announce(const output::Output::Announcement& a) 
{ 
    orig_range = Base::scalar.range(a);
    if ( orig_range.first.is_initialized() && ! user.test(0) ) 
        { Base::range[0] = * orig_range.first; not_given.reset(0); }
    if ( orig_range.second.is_initialized() && ! user.test(1) ) 
        { Base::range[1] = * orig_range.second; not_given.reset(1); }
    if ( not_given.none() ) Base::recompute_scale();
}

template <int Index>
float Localization<Index,Bounded>::get_size() const
{
    DEBUG("Interval for " << Index << " goes from " << range[0] << " " << range[1]);
    return (range[1] - range[0]).value();
}
template <int Index>
float Localization<Index,ScaledByResolution>::get_size() const
{
    float rv = Base::get_size() * scale.value();
    DEBUG( "Size for field " << Index << ":" << Base::scalar.row() << ":" << Base::scalar.column() << " is " << rv );
    return rv;
}
template <int Index>
float Localization<Index,ScaledToInterval>::get_size() const
{
    return desired_range;
}

template <int Index>
void Localization<Index,ScaledToInterval>::recompute_scale() 
{
    Base::scale = desired_range / typename Base::InvScale(Base::range[1] - Base::range[0]);
}

template <int Index>
traits::ImageResolution Localization<Index,IsUnscaled>::resolution() const
{
    traits::ImageResolution rv( value::from_value(1.0) / camera::pixel );
    assert( rv.unit_symbol == symbol_string(typename value::unit_type()) );
    return rv;
}

template <int Index>
traits::ImageResolution Localization<Index,ScaledByResolution>::resolution() const
{
    traits::ImageResolution rv( static_cast<typename Base::Scale::value_type>(1.0) / scale / camera::pixel );
    assert( rv.unit_symbol == symbol_string(typename value::unit_type()) );
    return rv;
}

template <int Index>
bool Localization<Index,IsUnscaled>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    traits::Scalar<TraitsType> s(row, column);
    return s.is_given(t);
} 

template <int Index>
bool Localization<Index,Bounded>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    traits::Scalar<TraitsType> s(row, column);
    return Base::can_work_with(t, row, column) && s.range(t).first.is_initialized() && s.range(t).second.is_initialized();
}

template <int Index>
bool Localization<Index,ScaledByResolution>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    return Base::can_work_with(t, row, column);
}

template <int Index>
bool Localization<Index,ScaledToInterval>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    return Base::can_work_with(t, row, column);
}

template <int Index>
bool Localization<Index,InteractivelyScaledToInterval>::can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column )
{
    return traits::Scalar<TraitsType>(row, column).is_given(t);
}

template <int Index>
std::pair< float, float > Localization<Index,Bounded>::get_minmax() const
{
    return std::make_pair( range[0].value(), range[1].value() );
}

template <int Index>
std::pair< float, float > Localization<Index,ScaledByResolution>::get_minmax() const
{
    return std::make_pair( Base::range[0] * scale, Base::range[1] * scale );
}

template <int Index>
bool Localization<Index,InteractivelyScaledToInterval>::is_bounded() const
{
    return not_given.none();
}

template <int Index>
double Localization<Index,Bounded>::reverse_mapping( float mapped_value ) const
{
    return mapped_value + range[0].value();
}

template <int Index>
double Localization<Index,ScaledByResolution>::reverse_mapping( float mapped_value ) const
{
    return value(mapped_value / scale + Base::range[0]).value();
}

template <int Index>
void Localization<Index,InteractivelyScaledToInterval>::set_user_limit( bool lower_limit, const std::string& s )
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
display::KeyDeclaration Localization<Index,InteractivelyScaledToInterval>::key_declaration() const
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
