#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_IMPL_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_IMPL_H

#include "debug.h"
#include "localization.h"
#include <dStorm/display/DataSource.h>
#include <boost/units/cmath.hpp>

namespace dStorm {
namespace binning {

template <typename Tag>
Localization<Tag,IsUnscaled>::Localization() {}

template <typename Tag>
Localization<Tag,Bounded>::Localization()
: Base(), discard( true ) {}

template <typename Tag>
Localization<Tag,ScaledByResolution>::Localization(value res) 
: Base(), scale(1.0f / quantity<typename value::unit_type,float>(res))
{
}

template <typename Tag>
Localization<Tag,ScaledToInterval>::Localization(float desired_range) 
: Base(), desired_range(desired_range)
{
}

template <typename Tag>
Localization<Tag,InteractivelyScaledToInterval>::Localization(float desired_range) 
: Base(desired_range)
{
    not_given.set();
}

template <typename Tag>
typename Localization<Tag,IsUnscaled>::value
Localization<Tag,IsUnscaled>::bin_naively( const dStorm::Localization& l ) const
{
    return l.field(Tag()).value();
}

template <typename Tag>
bool
Localization<Tag,Bounded>::in_range( value v ) const
{
    return (v >= range[0] && v <= range[1]);
}

template <typename Tag>
float
Localization<Tag,Bounded>::scale( value v ) const
{
    return (v - range[0]).value();
}

template <typename Tag>
typename Localization<Tag,Bounded>::value
Localization<Tag,Bounded>::clip( value v ) const
{
    return std::max( range[0], std::min( v, range[1] ) );
}


template <typename Tag>
boost::optional<float>
Localization<Tag,IsUnscaled>::bin_point( const dStorm::Localization& l ) const
{
    return bin_naively(l).value(); 
}

template <typename Tag>
boost::optional<float>
Localization<Tag,Bounded>::bin_point( const dStorm::Localization& l ) const
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

template <typename Tag>
boost::optional<float>
Localization<Tag,ScaledByResolution>::bin_point( const dStorm::Localization& l ) const
{
    boost::optional<float> f = Base::bin_point( l );
    if ( f.is_initialized() )
        return *f * scale.value();
    else
        return f;
}

template <typename Tag>
boost::optional<float>
Localization<Tag,InteractivelyScaledToInterval>::bin_point( const dStorm::Localization& l ) const
{
    if ( not_given.none() ) {
        return Base::bin_point(l);
    } else {
        return boost::optional<float>();
    }
}

template <typename Tag>
int
Localization<Tag,IsUnscaled>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    float *t = target;
    for (output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i, t += stride)
        *t = bin_naively(*i).value();
    return (t - target) / stride;
}

template <typename Tag>
int
Localization<Tag,Bounded>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
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

template <typename Tag>
int
Localization<Tag,ScaledByResolution>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    int c = Base::bin_points(l, target, stride);
    for (int i = 0; i < c; ++i)
        target[i * stride] *= scale.value();
    return c;
}

template <typename Tag>
int
Localization<Tag,InteractivelyScaledToInterval>::bin_points(const output::LocalizedImage& l, float *target, int stride) const
{
    if ( not_given.none() )
        return Base::bin_points(l, target, stride);
    else {
        return 0;
    }
}

template <typename Tag>
void Localization<Tag,IsUnscaled>::announce(const output::Output::Announcement& a) 
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

template <typename Tag>
void Localization<Tag,Bounded>::announce(const output::Output::Announcement& a) 
{ 
    Base::announce(a);
    const TraitsType& traits = a;
    if ( ! traits.range().first.is_initialized() || ! traits.range().second.is_initialized() ) {
        std::stringstream message;
        if ( ! traits.range().first.is_initialized() ) message << "lower";
        if ( ! traits.range().first.is_initialized() && ! traits.range().second.is_initialized() )
            message << " and ";
        if ( ! traits.range().second.is_initialized() ) message << "upper";
        message << " bound not set for field " << TraitsType::get_shorthand();
        throw std::runtime_error(message.str());
    }
    range[0] = *traits.range().first; 
    range[1] = *traits.range().second; 
}

template <typename Tag>
void Localization<Tag,ScaledToInterval>::announce(const output::Output::Announcement& a) 
{ 
    Base::announce(a);

    recompute_scale();
}

template <typename Tag>
void Localization<Tag,InteractivelyScaledToInterval>::announce(const output::Output::Announcement& a) 
{ 
    orig_range = static_cast<const TraitsType&>(a).range();
    if ( orig_range.first.is_initialized() && ! user.test(0) ) 
        { Base::range[0] = * orig_range.first; not_given.reset(0); }
    if ( orig_range.second.is_initialized() && ! user.test(1) ) 
        { Base::range[1] = * orig_range.second; not_given.reset(1); }
    if ( not_given.none() ) Base::recompute_scale();
}

template <typename Tag>
float Localization<Tag,Bounded>::get_size() const
{
    return (range[1] - range[0]).value();
}
template <typename Tag>
float Localization<Tag,ScaledByResolution>::get_size() const
{
    float rv = Base::get_size() * scale.value();
    return rv;
}
template <typename Tag>
float Localization<Tag,ScaledToInterval>::get_size() const
{
    return desired_range;
}

template <typename Tag>
void Localization<Tag,ScaledToInterval>::recompute_scale() 
{
    Base::scale = desired_range / typename Base::InvScale(Base::range[1] - Base::range[0]);
}

template <typename Tag>
traits::ImageResolution Localization<Tag,IsUnscaled>::resolution() const
{
    traits::ImageResolution rv( value::from_value(1.0) / camera::pixel );
    assert( rv.unit_symbol == symbol_string(typename value::unit_type()) );
    return rv;
}

template <typename Tag>
traits::ImageResolution Localization<Tag,ScaledByResolution>::resolution() const
{
    traits::ImageResolution rv( static_cast<typename Base::Scale::value_type>(1.0) / scale / camera::pixel );
    assert( rv.unit_symbol == symbol_string(typename value::unit_type()) );
    return rv;
}

template <typename Tag>
bool Localization<Tag,IsUnscaled>::can_work_with( const input::Traits<dStorm::Localization>& t )
{
    return static_cast<const TraitsType&>(t).is_given;
} 

template <typename Tag>
bool Localization<Tag,Bounded>::can_work_with( const input::Traits<dStorm::Localization>& t )
{
  const TraitsType& traits = t;
  return Base::can_work_with(t) && traits.range().first.is_initialized() && traits.range().second.is_initialized();
}

template <typename Tag>
bool Localization<Tag,ScaledByResolution>::can_work_with( const input::Traits<dStorm::Localization>& t)
{
    return Base::can_work_with(t);
}

template <typename Tag>
bool Localization<Tag,ScaledToInterval>::can_work_with( const input::Traits<dStorm::Localization>& t)
{
    return Base::can_work_with(t);
}

template <typename Tag>
bool Localization<Tag,InteractivelyScaledToInterval>::can_work_with( const input::Traits<dStorm::Localization>& t )
{
    return static_cast<const TraitsType&>(t).is_given;
}

template <typename Tag>
std::pair< float, float > Localization<Tag,Bounded>::get_minmax() const
{
    return std::make_pair( range[0].value(), range[1].value() );
}

template <typename Tag>
std::pair< float, float > Localization<Tag,ScaledByResolution>::get_minmax() const
{
    return std::make_pair( Base::range[0] * scale, Base::range[1] * scale );
}

template <typename Tag>
bool Localization<Tag,InteractivelyScaledToInterval>::is_bounded() const
{
    return not_given.none();
}

template <typename Tag>
double Localization<Tag,Bounded>::reverse_mapping( float mapped_value ) const
{
    return mapped_value + range[0].value();
}

template <typename Tag>
double Localization<Tag,ScaledByResolution>::reverse_mapping( float mapped_value ) const
{
    return value(mapped_value / scale + Base::range[0]).value();
}

template <typename Tag>
void Localization<Tag,InteractivelyScaledToInterval>::set_user_limit( bool lower_limit, const std::string& s )
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

template <typename Tag>
display::KeyDeclaration Localization<Tag,InteractivelyScaledToInterval>::key_declaration() const
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

#endif
