#include "localization_variable_decl.h"
#include <boost/mpl/for_each.hpp>
#include <boost/fusion/include/value_at.hpp>
#include "expression/QuantityDynamizer.hpp"
#include "dStorm/Localization.h"
#include "dejagnu.h"

namespace dStorm {
namespace expression {

template <int Field>
class ValueVariable : public Variable {
  typedef typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Field> >::type::Traits TraitsType;
 public:
  ValueVariable() : Variable(TraitsType::get_shorthand()) {}

  Variable* clone() const { return new ValueVariable(*this); }

  bool is_static(const input::Traits<Localization>& traits) const {
    return static_cast<const TraitsType&>(traits).static_value;
  }

  DynamicQuantity get(const input::Traits<Localization>& traits) const {
    if (static_cast<const TraitsType&>(traits).is_given) {
      return dynamizer.from_value( std::numeric_limits<double>::quiet_NaN() );
    } else {
      throw std::runtime_error("Tried to read variable " + this->Variable::name + ", but it is not defined.");
    }
  }

  DynamicQuantity get(const Localization& localization) const {
    return dynamizer(boost::fusion::at_c<Field>(localization).value());
  }

  void set( input::Traits<Localization>& traits, const DynamicQuantity& value ) const {
    static_cast<TraitsType&>(traits).is_given = true;
  }

  bool set( const input::Traits<Localization>& localization_traits, Localization& localization, const DynamicQuantity& value ) const {
    const typename TraitsType::ValueType parsed_value( dynamizer(value) );
    boost::fusion::at_c<Field>(localization) = parsed_value;
    const TraitsType& traits = localization_traits;
    return (!traits.range().first || *traits.range().first <= parsed_value) &&
           (!traits.range().second || *traits.range().second >= parsed_value);
  }

 private:
  QuantityDynamizer< typename TraitsType::ValueType > dynamizer;
};

template <int Field>
class MinVariable : public Variable {
  typedef typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Field> >::type::Traits TraitsType;
 public:
  MinVariable() : Variable("min" + TraitsType::get_shorthand()) {}

  Variable* clone() const { return new MinVariable(*this); }

  bool is_static(const input::Traits<Localization>& traits) const { return true; }

  DynamicQuantity get(const input::Traits<Localization>& localization_traits) const {
    const TraitsType& traits = localization_traits;
    if (traits.range().first) {
      return dynamizer(*traits.range().first);
    } else {
      throw std::runtime_error("Tried to read variable " + name + ", but it is not defined.");
    }
  }

  DynamicQuantity get(const Localization& localization) const {
    throw std::logic_error("Tried to read dynamic value of static variable " + name);
  }

  void set( input::Traits<Localization>& traits, const DynamicQuantity& value ) const {
    static_cast<TraitsType&>(traits).range().first = dynamizer(value);
  }

  bool set( const input::Traits<Localization>& traits, Localization& localization, const DynamicQuantity& value ) const {
    return *static_cast<const TraitsType&>(traits).range().first <= boost::fusion::at_c<Field>(localization).value();
  }

 private:
  QuantityDynamizer<  typename TraitsType::ValueType > dynamizer;
};

template <int Field>
class MaxVariable : public Variable {
  typedef typename boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Field> >::type::Traits TraitsType;
 public:
  MaxVariable() : Variable("max" + TraitsType::get_shorthand()) {}

  Variable* clone() const { return new MaxVariable(*this); }

  bool is_static(const input::Traits<Localization>& traits) const { return true; }

  DynamicQuantity get(const input::Traits<Localization>& localization_traits) const {
    const TraitsType& traits = localization_traits;
    if (traits.range().second) {
      return dynamizer(*traits.range().second);
    } else {
      throw std::runtime_error("Tried to read variable " + name + ", but it is not defined.");
    }
  }

  DynamicQuantity get(const Localization& localization) const {
    throw std::logic_error("Tried to read dynamic value of static variable " + name);
  }

  void set( input::Traits<Localization>& traits, const DynamicQuantity& value ) const {
    static_cast<TraitsType&>(traits).range().second = dynamizer(value);
  }

  bool set( const input::Traits<Localization>& traits, Localization& localization, const DynamicQuantity& value ) const {
    return *static_cast<const TraitsType&>(traits).range().second >= boost::fusion::at_c<Field>(localization).value();
  }

 private:
  QuantityDynamizer< typename TraitsType::ValueType > dynamizer;
};


/** \cond */
template <int Field> struct FieldAdder;
template <> struct FieldAdder<Localization::Fields::Count>;

template <int Field, bool has_range>
struct RangedFieldAdder {
  static void add_variables_for_field(boost::ptr_vector<Variable>& target) {
  }
};

template <int Field>
struct RangedFieldAdder<Field,false> {
  static void add_variables_for_field(boost::ptr_vector<Variable>& target) {
  }
};

template <>
struct FieldAdder<Localization::Fields::Count> {
    static void add_variables_for_field(boost::ptr_vector<Variable>& target) {}
};

template <int Field>
struct FieldAdder : public FieldAdder<Field+1> {
    static void add_variables_for_field(boost::ptr_vector<Variable>& target) {
      target.push_back( new ValueVariable<Field>() );
      if (boost::fusion::result_of::value_at<Localization, boost::mpl::int_<Field> >::type::Traits::has_range) {
        target.push_back( new MinVariable<Field>() );
        target.push_back( new MaxVariable<Field>() );
      }
      FieldAdder<Field+1>::add_variables_for_field(target);
    }
};

/** \endcond */

std::auto_ptr< boost::ptr_vector<Variable> >
variables_for_localization_fields() {
    boost::ptr_vector<Variable> rv;
    FieldAdder<0>::add_variables_for_field(rv);
    return rv.release();
}

void check_localization_variable( TestState& state ) {
    dStorm::input::Traits<dStorm::Localization> traits;
    dStorm::Localization loc;

    loc.position_x() = 15 * boost::units::si::meter;

    ValueVariable<Localization::Fields::PositionX> v;

    try {
        v.get(traits);
        state.fail( "No error thrown when traits with X coordinate unset were provided to get()" );
    } catch (...) {}

    DynamicUnit meter_only;
    meter_only[ *UnitTable().find("m") ] = 1;
    DynamicQuantity fifteen_meters( 15, meter_only );

    traits.position_x().is_given = true;
    state( v.is_static(traits) == false );
    state( v.get(traits) != fifteen_meters );
    state( v.get(loc) == fifteen_meters );
}

}
}
