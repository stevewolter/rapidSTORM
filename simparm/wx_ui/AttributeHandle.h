#ifndef SIMPARM_WX_ATTRIBUTE_HANDLE_H
#define SIMPARM_WX_ATTRIBUTE_HANDLE_H

#include <wx/msgdlg.h>
#include <simparm/Attribute.h>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/lexical_cast.hpp>
#include "ProtocolNode.h"

namespace simparm {
namespace wx_ui {

class BaseAttributeHandle {
    mutable boost::recursive_mutex mutex;
    BaseAttribute *a;
    const ProtocolNode& protocol;
public:
    BaseAttributeHandle( BaseAttribute& a, const ProtocolNode& protocol ) : a(&a), protocol(protocol) {}
    boost::optional< std::string > get_value() const { 
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        return (a) ? a->get_value() : boost::optional< std::string >();
    }

    bool value_is_optional() const {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        return (a) ? a->value_is_optional() : true;
    }

    bool set_value( std::string value ) {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        if (a) {
            bool success = false;
            try {
                success = a->set_value(value); 
                if ( success ) protocol.protocol( "in value set " + value );
            } catch ( const std::exception& e ) {
                wxMessageBox( wxString( e.what(), wxConvUTF8 ) );
            }
            return success;
        } else {
            return true;
        }
    }

    void unset_value() {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        if (a) {
            try {
                a->unset_value();
                protocol.protocol( "in value unset" );
            } catch ( const std::exception& e ) {
                wxMessageBox( wxString( e.what(), wxConvUTF8 ) );
            }
        }
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
    const ProtocolNode& protocol;
public:
    AttributeHandle( Attribute<ValueType>& a, const ProtocolNode& p ) : a(&a), protocol(p) {}
    AttributeHandle( BaseAttribute& a, const ProtocolNode& p ) : a( dynamic_cast< Attribute<ValueType>* >(&a) ), protocol(p) {}
    boost::optional< std::string > get_value() const { 
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        return (a) ? a->get_value() : boost::optional< std::string >();
    }

    bool set_value( std::string value ) {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        if (a) {
            bool success = false;
            try {
                success = a->set_value(value); 
                if ( success ) protocol.protocol( "in value set " + value );
            } catch ( const std::exception& e ) {
                wxMessageBox( wxString( e.what(), wxConvUTF8 ) );
            }
            return success;
        } else
            return true;
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
        if ( a ) {
            try {
                a->set_value_from_GUI( o );
                protocol.protocol( "in value set " + boost::lexical_cast< std::string >(o) );
            } catch ( const std::exception& e ) {
                wxMessageBox( wxString( e.what(), wxConvUTF8 ) );
            }
        }
        return *this;
    }

    template <typename Type>
    AttributeHandle& operator+=( const Type& o ) {
        boost::lock_guard< boost::recursive_mutex > lock( mutex );
        if ( a ) {
            try {
                Type v = (*a)() + o;
                a->set_value_from_GUI( v );
                protocol.protocol( "in value set " + boost::lexical_cast< std::string >(v) );
            } catch ( const std::exception& e ) {
                wxMessageBox( wxString( e.what(), wxConvUTF8 ) );
            }
        }
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
