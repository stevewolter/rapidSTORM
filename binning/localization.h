#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_H

#include "binning.h"
#include <boost/fusion/include/sequence.hpp>

#include <boost/units/io.hpp>
#include <bitset>
#include <dStorm/display/DataSource.h>

namespace dStorm {
namespace binning {

template <int Index, int Mode>
class Localization;

template <int Index>
class Localization<Index, IsUnscaled> {
  public:
    typedef typename boost::fusion::result_of::value_at<dStorm::Localization, boost::mpl::int_<Index> >::type::Traits TraitsType;
    typedef typename TraitsType::ValueType value;

    Localization();
    Localization* clone() const { return new Localization(*this); }

    void announce(const output::Output::Announcement& a);
    traits::ImageResolution resolution() const;

    static bool can_work_with( const input::Traits<dStorm::Localization>& t);

    int field_number() const { return Index; }
    int bin_points( const output::LocalizedImage& l, float *target, int stride ) const;
    boost::optional<float> bin_point( const dStorm::Localization& ) const;
    boost::optional<float> get_uncertainty( const dStorm::Localization& ) const;

  protected:
    value bin_naively( const dStorm::Localization& ) const;
};

template <int Index>
struct Localization<Index, Bounded> : public Localization<Index,IsUnscaled> {
    typedef Localization<Index,IsUnscaled> Base;
    typedef typename Base::TraitsType TraitsType;
    typedef typename Base::value value;

    Localization();
    Localization* clone() const { return new Localization(*this); }
    static bool can_work_with( const input::Traits<dStorm::Localization>& t );

    dStorm::traits::ImageResolution resolution() const { return Base::resolution(); }
    void announce(const output::Output::Announcement& a);
    float get_size() const;
    int bin_points( const output::LocalizedImage& l, float *target, int stride ) const;
    boost::optional<float> bin_point( const dStorm::Localization& ) const;
    std::pair< float, float > get_minmax() const;
    int field_number() const { return Index; }
    double reverse_mapping( float ) const;
    void set_clipping( bool discard_outliers ) { discard = discard_outliers; }

  protected:
    typedef typename divide_typeof_helper< quantity<si::dimensionless, float>, value >::type Scale;
    typedef quantity<typename value::unit_type, float> InvScale;
    value range[2];
    bool discard;

    bool in_range( value ) const;
    float scale( value ) const;
    value clip( value ) const;
};

template <int Index>
class Localization<Index, ScaledByResolution> 
: public Localization<Index,Bounded> {
  public:
    typedef Localization<Index,Bounded> Base;
    typedef typename Base::TraitsType TraitsType;
    typedef typename Base::value value;

    Localization(value res);
    Localization* clone() const { return new Localization(*this); }
    static bool can_work_with( const input::Traits<dStorm::Localization>& t);

    traits::ImageResolution resolution() const;
    int bin_points( const output::LocalizedImage& l, float *target, int stride ) const;
    boost::optional<float> bin_point( const dStorm::Localization& ) const;
    boost::optional<float> get_uncertainty( const dStorm::Localization& ) const;
    double reverse_mapping( float ) const;
    float get_size() const;
    std::pair< float, float > get_minmax() const;

  protected:
    Localization() : Base() {}
    typename Base::Scale scale;
};

template <int Index>
class Localization<Index, ScaledToInterval> 
: public Localization<Index, ScaledByResolution> {
  public:
    typedef Localization<Index,ScaledByResolution> Base;
    typedef typename Base::TraitsType TraitsType;
    typedef typename Base::value value;

    Localization(float interval_width);
    Localization* clone() const { return new Localization(*this); }
    static bool can_work_with( const input::Traits<dStorm::Localization>& t);

    void announce(const output::Output::Announcement& a);
    float get_size() const;

  private:
    float desired_range;
  protected:
    void recompute_scale();
};

template <int Index>
class Localization<Index, InteractivelyScaledToInterval> : public Localization<Index, ScaledToInterval> {
  public:
    typedef Localization<Index,ScaledToInterval> Base;
    typedef typename Base::TraitsType TraitsType;
    typedef typename Base::value value;

    Localization(float interval_width);
    Localization* clone() const { return new Localization(*this); }

    void announce(const output::Output::Announcement& a);
    traits::ImageResolution resolution() const { return Base::resolution(); }
    int bin_points( const output::LocalizedImage& l, float *target, int stride ) const;
    boost::optional<float> bin_point( const dStorm::Localization& ) const;
    boost::optional<float> get_uncertainty( const dStorm::Localization& ) const;
    float get_size() const { return Base::get_size(); }

    static bool can_work_with( const input::Traits<dStorm::Localization>& t);
    int field_number() const { return Index; }

    double reverse_mapping( float f ) const { return Base::reverse_mapping(f); }
    void set_user_limit( bool lower_limit, const std::string& s );
    bool is_bounded() const;
    display::KeyDeclaration key_declaration() const;
    std::pair< float, float > get_minmax() const { return Base::get_minmax(); }

  private:
    std::bitset<2> not_given, user;
    typename TraitsType::RangeType orig_range;
};

}
}

#endif
