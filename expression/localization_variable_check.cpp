#include "localization_variable_impl.h"

using namespace dStorm::expression;

int main() {
    dStorm::input::Traits<dStorm::Localization> traits;
    dStorm::Localization loc;

    loc.position().x() = 15 * boost::units::si::meter;

    Variable<0,dStorm::traits::value_tag> v( dStorm::traits::Scalar< dStorm::traits::Position >(0,0) );

    try {
        v.get(traits);
        assert( false /* No error thrown when traits with X coordinate unset were provided to get() */ );
    } catch (...) {}

    DynamicUnit meter_only;
    meter_only[ *UnitTable().find("m") ] = 1;
    DynamicQuantity fifteen_meters( 15, meter_only );

    traits.position().is_given(0,0) = true;
    assert( v.is_static(traits) == false );
    assert( v.get(traits) != fifteen_meters );
    assert( v.get(loc) == fifteen_meters );
    return 0;
}
