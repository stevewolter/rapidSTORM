#ifndef SIMPARM_ATTRIBUTE_HPP
#define SIMPARM_ATTRIBUTE_HPP

#include "Attribute.h"
#include "iostream.h"
#include <cstdio>

namespace simparm {

template <typename Type>
boost::optional< std::string > Attribute<Type>::get_value() const { 
    if ( value_is_given( value ) ) {
        std::stringstream result; to_config_stream( result, value ); return result.str(); 
    } else
        return boost::optional< std::string >();
}

template <typename Type>
bool Attribute<Type>::valueChange(const Type &to, bool from_gui)
{
    if ( change_is_OK == NULL || (*change_is_OK)( value, to ) ) {
        if ( to == value ) {
            /* Do nothing. This is worded with == to avoid NaN
                * trouble. */
        } else {
            value = to;
            value_changed();
            if ( ! from_gui )
                value_changed_non_GUI();
        }
        return true;
    } else {
        return false;
    }
}

template <typename Type>
bool Attribute<Type>::set_value(const std::string& i ) { 
    std::stringstream stream(i);
    Type temp;
    from_config_stream( stream, temp );
    if ( stream.peek() != EOF ) return false;
    return valueChange( temp, true ); 
}

}

#endif
