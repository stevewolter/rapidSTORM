#ifndef SIMPARM_WX_ATTRIBUTE_HANDLE_H
#define SIMPARM_WX_ATTRIBUTE_HANDLE_H

#include <simparm/Attribute.h>
#include <boost/thread/recursive_mutex.hpp>

namespace simparm {
namespace wx_ui {

class BaseAttributeHandle {
    mutable boost::recursive_mutex mutex;
    BaseAttribute *a;
public:
    BaseAttributeHandle( BaseAttribute& a ) : a(&a) {}
    boost::optional< std::string > get_value() const { 
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        return (a) ? a->get_value() : boost::optional< std::string >();
    }

    void set_value( std::string value ) {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        if (a) a->set_value(value);
    }

    void detach() {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        a = NULL;
    }
};

template <typename ValueType>
class AttributeHandle {
    mutable boost::recursive_mutex mutex;
    Attribute<ValueType> *a;
public:
    AttributeHandle( Attribute<ValueType>& a ) : a(&a) {}
    AttributeHandle( BaseAttribute& a ) : a( dynamic_cast< Attribute<ValueType>* >(&a) ) {}
    boost::optional< std::string > get_value() const { 
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        return (a) ? a->get_value() : boost::optional< std::string >();
    }

    void set_value( std::string value ) {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        if (a) a->set_value(value);
    }

    boost::optional<ValueType> operator()() {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        if ( a )
            return (*a)();
        else
            return boost::optional<ValueType>();
    }

    template <typename Type>
    AttributeHandle& operator=( const Type& o ) {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        if ( a ) a->set_value_from_GUI( o );
        return *this;
    }

    template <typename Type>
    AttributeHandle& operator+=( const Type& o ) {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        if ( a ) a->set_value_from_GUI( (*a)() + o );
        return *this;
    }

    void detach() {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        a = NULL;
    }
};

}
}

#endif
