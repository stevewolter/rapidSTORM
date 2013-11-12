#include "localization_variable_decl.h"
#include <boost/bind/bind.hpp>
#include <boost/mpl/bind.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/fusion/include/value_at.hpp>
#include "dStorm/localization/Fields.h"
#include "expression/QuantityDynamizer.hpp"
#include "dStorm/Localization.h"
#include "dejagnu.h"

namespace dStorm {
namespace expression {

template <typename Tag>
class ValueVariable : public Variable {
 public:
  ValueVariable() : Variable(Tag::get_shorthand()) {}

  Variable* clone() const { return new ValueVariable(*this); }

  bool is_static(const input::Traits<Localization>& traits) const {
    return traits.field(Tag()).static_value;
  }

  DynamicQuantity get(const input::Traits<Localization>& traits) const {
    if (traits.field(Tag()).is_given) {
      return dynamizer.from_value( std::numeric_limits<double>::quiet_NaN() );
    } else {
      throw std::runtime_error("Tried to read variable " + this->Variable::name + ", but it is not defined.");
    }
  }

  DynamicQuantity get(const Localization& localization) const {
    return dynamizer(localization.field(Tag()).value());
  }

  void set( input::Traits<Localization>& traits, const DynamicQuantity& value ) const {
    traits.field(Tag()).is_given = true;
  }

  bool set( const input::Traits<Localization>& localization_traits, Localization& localization, const DynamicQuantity& value ) const {
    const typename Tag::ValueType parsed_value( dynamizer(value) );
    localization.field(Tag()) = parsed_value;
    const localization::MetaInfo<Tag>& traits = localization_traits;
    return (!traits.range().first || *traits.range().first <= parsed_value) &&
           (!traits.range().second || *traits.range().second >= parsed_value);
  }

 private:
  QuantityDynamizer< typename Tag::ValueType > dynamizer;
};

template <typename Tag>
class MinVariable : public Variable {
 public:
  MinVariable() : Variable("min" + Tag::get_shorthand()) {}

  Variable* clone() const { return new MinVariable(*this); }

  bool is_static(const input::Traits<Localization>& traits) const { return true; }

  DynamicQuantity get(const input::Traits<Localization>& traits) const {
    if (traits.field(Tag()).range().first) {
      return dynamizer(*traits.field(Tag()).range().first);
    } else {
      throw std::runtime_error("Tried to read variable " + name + ", but it is not defined.");
    }
  }

  DynamicQuantity get(const Localization& localization) const {
    throw std::logic_error("Tried to read dynamic value of static variable " + name);
  }

  void set( input::Traits<Localization>& traits, const DynamicQuantity& value ) const {
    traits.field(Tag()).range().first = dynamizer(value);
  }

  bool set( const input::Traits<Localization>& traits, Localization& localization, const DynamicQuantity& value ) const {
    return *traits.field(Tag()).range().first <= localization.field(Tag()).value();
  }

 private:
  QuantityDynamizer<  typename Tag::ValueType > dynamizer;
};

template <typename Tag>
class MaxVariable : public Variable {
 public:
  MaxVariable() : Variable("max" + Tag::get_shorthand()) {}

  Variable* clone() const { return new MaxVariable(*this); }

  bool is_static(const input::Traits<Localization>& traits) const { return true; }

  DynamicQuantity get(const input::Traits<Localization>& traits) const {
    if (traits.field(Tag()).range().second) {
      return dynamizer(*traits.field(Tag()).range().second);
    } else {
      throw std::runtime_error("Tried to read variable " + name + ", but it is not defined.");
    }
  }

  DynamicQuantity get(const Localization& localization) const {
    throw std::logic_error("Tried to read dynamic value of static variable " + name);
  }

  void set( input::Traits<Localization>& traits, const DynamicQuantity& value ) const {
    traits.field(Tag()).range().second = dynamizer(value);
  }

  bool set( const input::Traits<Localization>& traits, Localization& localization, const DynamicQuantity& value ) const {
    return *traits.field(Tag()).range().second >= localization.field(Tag()).value();
  }

 private:
  QuantityDynamizer< typename Tag::ValueType > dynamizer;
};

struct FieldAdder {
    typedef void result_type;
    template <typename Tag>
    void operator()(boost::ptr_vector<Variable>& target, Tag tag) {
      target.push_back( new ValueVariable<Tag>() );
      if (Tag::has_range) {
        target.push_back( new MinVariable<Tag>() );
        target.push_back( new MaxVariable<Tag>() );
      }
    }
};

std::auto_ptr< boost::ptr_vector<Variable> >
variables_for_localization_fields() {
    boost::ptr_vector<Variable> rv;
    boost::mpl::for_each<localization::Fields>(boost::bind(
        FieldAdder(), boost::ref(rv), _1));
    return rv.release();
}

void check_localization_variable( TestState& state ) {
    dStorm::input::Traits<dStorm::Localization> traits;
    dStorm::Localization loc;

    loc.position_x() = 15 * boost::units::si::meter;

    ValueVariable<traits::PositionX> v;

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
