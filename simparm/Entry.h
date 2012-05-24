#if !defined(CONFIGENTRY_HH)
#define CONFIGENTRY_HH

#include "BasicEntry.h"
#include "BoostOptional.h"
#include "Object.h"
#include <list>
#include <iostream>
#include <boost/utility.hpp>
#include <boost/utility/base_from_member.hpp>
#include "default_value.h"
#include "MinMaxWatcher.h"
#include "Attributes.h"

namespace simparm {
using std::string;
using std::istream;
using std::ostream;

template <typename Type> 
const char *typeName();

template<typename ValueField>
struct Attributes<bool,ValueField,void> : private boost::noncopyable {
    Attributes( Attribute<ValueField>& ) {}
    Attributes( const Attributes&, Attribute<ValueField>& ) {}
    Attributes& operator=( const Attributes& ) { return *this; }
    void registerNamedEntries( simparm::NodeHandle ) {}
};

template<typename ValueField>
struct Attributes<std::string,ValueField,void> : private boost::noncopyable {
    Attributes( Attribute<ValueField>& ) {}
    Attributes( const Attributes&, Attribute<ValueField>& ) {}
    Attributes& operator=( const Attributes& ) { return *this; }
    void registerNamedEntries( simparm::NodeHandle ) {}
};

template <typename Type, typename ValueField>
struct Attributes<Type, ValueField, typename boost::enable_if< boost::is_fundamental<Type> >::type >
: private boost::noncopyable
{
    typedef typename add_boost_optional<ValueField>::type bound_type;
  public:
    Attribute<ValueField> increment;
    Attribute< bound_type > min, max;

    Attributes( Attribute<ValueField>& value );
    Attributes( const Attributes&, Attribute<ValueField>& value );
    Attributes& operator=( const Attributes& o ) {
        increment = o.increment();
        min = o.min();
        max = o.max();
        return *this;
    }
    void registerNamedEntries( simparm::NodeHandle );

 private:
    BoundWatcher< bound_type,ValueField,true> value_above_min;
    BoundWatcher< ValueField,bound_type,true> max_above_value;
    BoundWatcher< bound_type,ValueField,false> value_below_max;
    BoundWatcher< ValueField,bound_type,false> min_below_value;
};

template <typename TypeOfEntry>
class Entry 
: public BasicEntry, public boost::base_from_member< Attribute<TypeOfEntry> >,
  public Attributes<TypeOfEntry>
{
  protected:
    NodeHandle create_hidden_node( simparm::NodeHandle );
    NodeHandle make_naked_node( simparm::NodeHandle node ) { 
        if ( boost::is_same< TypeOfEntry, bool >() )
            return create_checkbox( node, getName(), getDesc() );
        else
            return create_textfield( node, getName(), getDesc(), typeName( TypeOfEntry() ) ); 
    }

  public:
    typedef TypeOfEntry value_type;
    typedef TypeOfEntry result_type;

    Attribute<TypeOfEntry>& value;

    Entry(string name, string desc = "",
                const TypeOfEntry& value = TypeOfEntry());
    Entry(const Entry<TypeOfEntry> &entry);
    ~Entry() ;
    Entry<TypeOfEntry>* clone() const
        { return new Entry<TypeOfEntry>(*this); }

    inline const TypeOfEntry &operator()() const { return value(); }
    void setValue(const TypeOfEntry &value)
        { this->value = value; }

    Entry<TypeOfEntry> &
        operator=(const Entry<TypeOfEntry> &entry);
    Entry<TypeOfEntry> &operator=(const TypeOfEntry &entry) 
 { this->value = entry; return *this; }

    bool operator==(const Entry<TypeOfEntry> &o) const;
    inline bool operator!=(const Entry<TypeOfEntry> &entry) const;
    inline bool operator==(const TypeOfEntry &value) const;
    inline bool operator!=(const TypeOfEntry &value) const;
};

typedef Entry<bool> BoolEntry;
typedef Entry<string> StringEntry;

}

#endif

