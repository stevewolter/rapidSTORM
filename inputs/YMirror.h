#ifndef DSTORM_INPUT_YMIRROR_H
#define DSTORM_INPUT_YMIRROR_H

#include "YMirror_decl.h"
#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/Image.h>
#include <simparm/Entry.hh>
#include <dStorm/input/chain/DefaultFilterTypes.h>

namespace dStorm {
namespace YMirror {

struct Config : public simparm::Object
{
    typedef input::chain::DefaultTypes SupportedTypes;

    simparm::BoolEntry mirror_y;
    Config();
    void registerNamedEntries() { push_back(mirror_y); }
};

template <typename Type>
class Source
: public input::AdapterSource< Type >,
  boost::noncopyable
{
    typedef Type Input;
    typedef input::Source<Input> Base;
    typedef Localization::Position::Traits::RangeType::Scalar Range;
    Range range;
    struct iterator;
    void modify_traits( input::Traits<Type>& );

  public:
    Source( std::auto_ptr< Base > base ) : input::AdapterSource<Type>(base) {}
    typename Base::iterator begin();
    typename Base::iterator end();
};

}
}

#endif
