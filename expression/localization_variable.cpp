#include "localization_variable_impl.h"
#include <boost/mpl/for_each.hpp>
#include <dStorm/traits/scalar_iterator.h>
#include <dStorm/traits/range_impl.h>
#include "dejagnu.h"

namespace dStorm {
namespace expression {

/** \cond */
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
struct FieldAdder<Localization::Fields::Count> {
    FieldAdder( boost::ptr_vector<Variable>& ) {}
    void add_variables_for_field() const {}
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
