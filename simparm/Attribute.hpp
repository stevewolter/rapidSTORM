#ifndef SIMPARM_ATTRIBUTE_HPP
#define SIMPARM_ATTRIBUTE_HPP

#include "Attribute.h"
#include "iostream.h"

namespace simparm {

template <typename Type>
boost::optional< std::string > Attribute<Type>::get_value() const { 
    if ( value_is_given( value ) ) {
        std::stringstream result; to_config_stream( result, value ); return result.str(); 
    } else
        return boost::optional< std::string >();
}

template <typename Type>
void Attribute<Type>::set_value(const std::string& i ) { 
    std::stringstream stream(i);
    Type temp;
    from_config_stream( stream, temp );
    valueChange( temp ); 
}

template <typename Type>
void Attribute<Type>::valueChange(const Type &to)
{
    if ( change_is_OK == NULL || (*change_is_OK)( value, to ) ) {
        if ( to == value ) {
            /* Do nothing. This is worded with == to avoid NaN
                * trouble. */
        } else {
            value = to;
            value_changed();
        }
    } else {
        /* Change is NOT ok. Print the correct value to underline this. */
        value_changed();
    }
}

}

#endif
