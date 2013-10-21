#include "localization_variable_impl.h"
#include <boost/mpl/for_each.hpp>
#include <dStorm/traits/scalar_iterator.h>
#include <dStorm/traits/range_impl.h>
#include "dejagnu.h"

namespace dStorm {
namespace expression {

class PositionVariable : public Variable {
 public:
  PositionVariable(std::string name, int index) : Variable(name), index(index) {}

  static DynamicUnit get_unit() {
    DynamicUnit unit;
    unit[BaseUnits::Meter] = 1;
  }

  Variable* clone() const { return new PositionVariable(*this); }

  bool is_static(const input::Traits<Localization>& traits) const {
    return traits.position().static_value;
  }

  DynamicQuantity get(const input::Traits<Localization>& traits) const {
    if (traits.position().is_given[index]) {
      return dynamizer.from_value( std::numeric_limits<double>::quiet_NaN() );
    } else {
      throw std::runtime_error("Tried to read variable " + this->Variable::name + ", but it is not defined.");
    }
  }

  DynamicQuantity get(const Localization& localization) const {
    return dynamizer(localization.position()[index]);
  }

  void set( input::Traits<Localization>& traits, const DynamicQuantity& value ) const {
    traits.position().is_given[index] = true;
  }

  bool set( const input::Traits<Localization>& traits, Localization& localization, const DynamicQuantity& value ) const {
    localization.position()[index] = dynamizer(value);
    return (!traits.position().range()[index].first ||
            *traits.position().range()[index].first <= localization.position()[index]) &&
           (!traits.position().range()[index].second ||
            *traits.position().range()[index].second >= localization.position()[index]);
  }

 private:
  int index;
  QuantityDynamizer< quantity<si::length> > dynamizer;
};

class PositionBoundVariable : public Variable {
 public:
  PositionBoundVariable(std::string name, int index, int bound) : Variable(name), index(index), bound(bound) {}

  template <typename Type>
  Type& GetBound(std::pair<Type,Type>& pair) const {
    return (bound == 0) ? pair.first : pair.second;
  }

  template <typename Type>
  const Type& GetBound(const std::pair<Type,Type>& pair) const {
    return (bound == 0) ? pair.first : pair.second;
  }

  static DynamicUnit get_unit() {
    DynamicUnit unit;
    unit[BaseUnits::Meter] = 1;
  }

  Variable* clone() const { return new PositionBoundVariable(*this); }

  bool is_static(const input::Traits<Localization>& traits) const { return true; }

  DynamicQuantity get(const input::Traits<Localization>& traits) const {
    if (GetBound(traits.position().range()[index])) {
      return dynamizer(*GetBound(traits.position().range()[index]));
    } else {
      throw std::runtime_error("Tried to read variable " + name + ", but it is not defined.");
    }
  }

  DynamicQuantity get(const Localization& localization) const {
    throw std::logic_error("Tried to read dynamic value of static variable " + name);
  }

  void set( input::Traits<Localization>& traits, const DynamicQuantity& value ) const {
    GetBound(traits.position().range()[index]) = samplepos::Scalar(dynamizer(value));
  }

  bool set( const input::Traits<Localization>& traits, Localization& localization, const DynamicQuantity& value ) const {
    if (bound == 0) {
      return *traits.position().range()[index].first <= localization.position()[index];
    } else {
      return *traits.position().range()[index].second >= localization.position()[index];
    }
  }

 private:
  int index;
  int bound;
  QuantityDynamizer< quantity<si::length> > dynamizer;
};


/** \cond */
template <int Field> struct FieldAdder;
template <> struct FieldAdder<Localization::Fields::Position>;
template <> struct FieldAdder<Localization::Fields::Count>;

template <>
struct FieldAdder<Localization::Fields::Count> {
    FieldAdder( boost::ptr_vector<Variable>& ) {}
    void add_variables_for_field() const {}
};

template <int Field>
struct FieldAdder : public FieldAdder<Field+1> {
    boost::ptr_vector<Variable>& target;
    FieldAdder( boost::ptr_vector<Variable>& target )
        : FieldAdder<Field+1>(target), target(target) {}

    template <typename Tag>
    void operator()(const Tag&) const {
        typedef LocalizationVariable<Field,Tag> Target;
        typedef typename Target::Scalar Scalar;
        for ( typename Scalar::Iterator i = Scalar::begin(); i != Scalar::end(); ++i )
            target.push_back( new Target(*i) );
    }

    void add_variables_for_field() const {
        boost::mpl::for_each<traits::tags>( *this );
        FieldAdder<Field+1>::add_variables_for_field();
    }
};

template <>
struct FieldAdder<Localization::Fields::Position>
: public FieldAdder<Localization::Fields::Position+1> {
    boost::ptr_vector<Variable>& target;
    FieldAdder( boost::ptr_vector<Variable>& target )
        : FieldAdder<Localization::Fields::Position+1>(target), target(target) {}

    void add_variables_for_field() const {
      for (int i = 0; i < 3; ++i) {
        std::string dim = (i == 0) ? "x" : (i == 1) ? "y" : "z";
        target.push_back( new PositionVariable("pos" + dim, i) );
        target.push_back( new PositionBoundVariable("posmin" + dim, i, 0) );
        target.push_back( new PositionBoundVariable("posmax" + dim, i, 1) );
      }
    }
};

/** \endcond */

std::auto_ptr< boost::ptr_vector<Variable> >
variables_for_localization_fields() {
    boost::ptr_vector<Variable> rv;
    FieldAdder<0>(rv).add_variables_for_field();
    return rv.release();
}

void check_localization_variable( TestState& state ) {
    dStorm::input::Traits<dStorm::Localization> traits;
    dStorm::Localization loc;

    loc.position().x() = 15 * boost::units::si::meter;

    LocalizationVariable<0,dStorm::traits::value_tag> v( dStorm::traits::Scalar< dStorm::traits::Position >(0,0) );

    try {
        v.get(traits);
        state.fail( "No error thrown when traits with X coordinate unset were provided to get()" );
    } catch (...) {}

    DynamicUnit meter_only;
    meter_only[ *UnitTable().find("m") ] = 1;
    DynamicQuantity fifteen_meters( 15, meter_only );

    traits.position().is_given(0,0) = true;
    state( v.is_static(traits) == false );
    state( v.get(traits) != fifteen_meters );
    state( v.get(loc) == fifteen_meters );
}

}
}
