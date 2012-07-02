#if !defined(CONFIGENTRY_HH)
#define CONFIGENTRY_HH

#include "BasicEntry.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include "Attribute.h"
#include "BoostOptional.h"

namespace simparm {
using std::string;

template <typename Type> 
const char *typeName();

template <typename TypeOfEntry>
class Entry 
: public BasicEntry
{
    typedef typename add_boost_optional<TypeOfEntry>::type bound_type;
    boost::ptr_vector< BaseAttribute > additional_attributes;
  protected:
    NodeHandle create_hidden_node( simparm::NodeHandle );
    NodeHandle make_naked_node( simparm::NodeHandle node );
private:
    std::auto_ptr< typename Attribute<TypeOfEntry>::ChangeWatchFunction > 
        range_checker;

  public:
    typedef TypeOfEntry value_type;
    typedef TypeOfEntry result_type;

    Attribute<TypeOfEntry> value;
    Attribute< TypeOfEntry > increment;
    Attribute< bound_type > min, max;

    Entry(string name, string desc,
                const TypeOfEntry& value);
    Entry(string name, const TypeOfEntry& value);
    Entry(const Entry<TypeOfEntry> &entry);
    ~Entry() ;
    Entry<TypeOfEntry>* clone() const
        { return new Entry<TypeOfEntry>(*this); }

    inline const TypeOfEntry &operator()() const { return value(); }
    void setValue(const TypeOfEntry &value)
        { this->value = value; }

    Entry<TypeOfEntry> &operator=(const Entry<TypeOfEntry> &entry) ;
    Entry<TypeOfEntry> &operator=(const TypeOfEntry &entry) 
        { this->value = entry; return *this; }

    bool operator==(const Entry<TypeOfEntry> &o) const;
    bool operator!=(const Entry<TypeOfEntry> &entry) const;
    bool operator==(const TypeOfEntry &value) const;
    bool operator!=(const TypeOfEntry &value) const;
};

typedef Entry<bool> BoolEntry;
typedef Entry<string> StringEntry;

}

#endif

