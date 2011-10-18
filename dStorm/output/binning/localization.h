#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_H

#include "binning.h"
#include "../../traits/scalar.h"
#include <boost/fusion/include/sequence.hpp>

#include <boost/units/io.hpp>
#include <bitset>
#include "../../helpers/DisplayDataSource.h"

namespace dStorm {
namespace output {
namespace binning {

template <int Mode> struct LocalizationInterfaceLevel;
template <> struct LocalizationInterfaceLevel<IsUnscaled> { typedef Unscaled type; };
template <> struct LocalizationInterfaceLevel<Bounded> { typedef Scaled type; };
template <> struct LocalizationInterfaceLevel<ScaledByResolution> { typedef Scaled type; };
template <> struct LocalizationInterfaceLevel<ScaledToInterval> { typedef Scaled type; };
template <> struct LocalizationInterfaceLevel<InteractivelyScaledToInterval> { typedef UserScaled type; };

template <int Index, int Mode, bool VirtualTable = true>
class Localization;

template <int Index>
class Localization<Index, IsUnscaled, false> {
  public:
    typedef typename boost::fusion::result_of::value_at<dStorm::Localization, boost::mpl::int_<Index> >::type::Traits TraitsType;
    typedef typename traits::Scalar<TraitsType>::value_type value;

    Localization(int row, int column);
    Localization* clone() const { return new Localization(*this); }

    void announce(const output::Output::Announcement& a);
    traits::ImageResolution resolution() const;

    static bool can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column );

    int field_number() const { return Index; }
    void bin_points( const output::LocalizedImage& l, float *target, int stride ) const;
    float bin_point( const dStorm::Localization& ) const;

  protected:
    traits::Scalar<TraitsType> scalar;
};

template <int Index>
struct Localization<Index, Bounded, false> : public Localization<Index,IsUnscaled, false> {
    typedef Localization<Index,IsUnscaled,false> Base;
    typedef typename Base::TraitsType TraitsType;
    typedef typename Base::value value;

    Localization(int row, int column);
    Localization* clone() const { return new Localization(*this); }
    static bool can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column );

    dStorm::traits::ImageResolution resolution() const { return Base::resolution(); }
    void announce(const output::Output::Announcement& a);
    float get_size() const;
    void bin_points( const output::LocalizedImage& l, float *target, int stride ) const;
    float bin_point( const dStorm::Localization& ) const;
    std::pair< float, float > get_minmax() const;
    int field_number() const { return Index; }
    double reverse_mapping( float ) const;

  protected:
    typedef typename divide_typeof_helper< quantity<si::dimensionless, float>, value >::type Scale;
    typedef quantity<typename value::unit_type, float> InvScale;
    value range[2];
};

template <int Index>
class Localization<Index, ScaledByResolution, false> 
: public Localization<Index,Bounded, false> {
  public:
    typedef Localization<Index,Bounded,false> Base;
    typedef typename Base::TraitsType TraitsType;
    typedef typename Base::value value;

    Localization(value res, int row, int column);
    Localization* clone() const { return new Localization(*this); }
    static bool can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column );

    traits::ImageResolution resolution() const;
    void bin_points( const output::LocalizedImage& l, float *target, int stride ) const;
    float bin_point( const dStorm::Localization& ) const;
    double reverse_mapping( float ) const;
    float get_size() const;
    std::pair< float, float > get_minmax() const;

  protected:
    Localization(int row, int column) : Base(row, column) {}
    typename Base::Scale scale;
};

template <int Index>
class Localization<Index, ScaledToInterval, false> 
: public Localization<Index, ScaledByResolution, false> {
  public:
    typedef Localization<Index,ScaledByResolution,false> Base;
    typedef typename Base::TraitsType TraitsType;
    typedef typename Base::value value;

    Localization(float interval_width, int row, int column);
    Localization* clone() const { return new Localization(*this); }
    static bool can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column );

    void announce(const output::Output::Announcement& a);
    float get_size() const;

  private:
    float desired_range;
  protected:
    void recompute_scale();
};

template <int Index>
class Localization<Index, InteractivelyScaledToInterval, false> : public Localization<Index, ScaledToInterval, false> {
  public:
    typedef Localization<Index,ScaledToInterval,false> Base;
    typedef typename Base::TraitsType TraitsType;
    typedef typename Base::value value;

    Localization(float interval_width, int row, int column);
    Localization* clone() const { return new Localization(*this); }

    void announce(const output::Output::Announcement& a);
    traits::ImageResolution resolution() const { return Base::resolution(); }
    void bin_points( const output::LocalizedImage& l, float *target, int stride ) const;
    float bin_point( const dStorm::Localization& ) const;
    float get_size() const { return Base::get_size(); }

    static bool can_work_with( const input::Traits<dStorm::Localization>& t, int row, int column );
    int field_number() const { return Index; }

    double reverse_mapping( float f ) const { return Base::reverse_mapping(f); }
    void set_user_limit( bool lower_limit, const std::string& s );
    bool is_bounded() const;
    Display::KeyDeclaration key_declaration() const;
    std::pair< float, float > get_minmax() const { return Base::get_minmax(); }

  private:
    std::bitset<2> given, user;
    typename traits::Scalar< TraitsType >::range_type orig_range;
};

template <int Index, int Mode>
struct Localization<Index,Mode,true>
: public Localization<Index,Mode,false>,
  public LocalizationInterfaceLevel<Mode>::type
{
    typedef Localization<Index,Mode,false> Base;
    typedef typename LocalizationInterfaceLevel<Mode>::type Interface;

    template <class T1, class T2>
    Localization(const T1& t1, const T2& t2) : Base(t1,t2) {}
    template <class T1, class T2, class T3>
    Localization(const T1& t1, const T2& t2, const T3& t3) : Base(t1,t2,t3) {}

    Localization* clone() const { return new Localization(*this); }
    void announce(const Output::Announcement& a) { Base::announce(a); }
    traits::ImageResolution resolution() const { return Base::resolution(); }
    void bin_points( const output::LocalizedImage& l, float* target, int stride ) const { Base::bin_points(l, target, stride); }
    int field_number() const { return Base::field_number(); }
    float bin_point( const dStorm::Localization& l ) const { return Base::bin_point(l); }

    inline float get_size() const { return Base::get_size(); }
    std::pair< float, float > get_minmax() const { return Base::get_minmax(); }
    double reverse_mapping( float f ) const { return Base::reverse_mapping(f); }

    void set_user_limit( bool lower_limit, const std::string& s ) { Base::set_user_limit(lower_limit, s); }
    bool is_bounded() const { return Base::is_bounded(); }
    Display::KeyDeclaration key_declaration() const { return Base::key_declaration(); }
};

}
}
}

#endif
