#include "inputs/YMirror.h"

#include <boost/smart_ptr/shared_array.hpp>
#include <boost/variant/apply_visitor.hpp>
#include "image/Image.h"
#include "image/mirror.h"
#include "input/AdapterSource.h"
#include "input/Method.hpp"
#include <simparm/Entry.h>
#include <simparm/Object.h>

namespace dStorm {
namespace YMirror {

struct Config
{
    typedef input::DefaultTypes SupportedTypes;

    simparm::Object name_object;
    simparm::BoolEntry mirror_y;
    Config();
    void attach_ui( simparm::NodeHandle at ) { mirror_y.attach_ui( name_object.attach_ui(at) ); }
};

template <typename Type>
class Source
: public input::AdapterSource< Type >,
  boost::noncopyable
{
    typedef input::Source<Type> Base;
    localization::MetaInfo<localization::PositionY::ValueType>::RangeType range;

    void modify_traits( input::Traits<Type>& );
    void attach_local_ui_( simparm::NodeHandle ) {}
    bool GetNext(int thread, Type* target) OVERRIDE;

  public:
    Source( std::auto_ptr< Base > base ) : input::AdapterSource<Type>(base) {}
};

class ChainLink
: public input::Method< ChainLink >
{
    friend class input::Method< ChainLink >;

    template <typename Type>
    input::Source<Type>* make_source( std::auto_ptr< input::Source<Type> > p ) 
    {
        if ( config.mirror_y() )
            return new Source<Type>(p);
        else
            return p.release();
    }
    template <typename Type>
    bool changes_traits( const input::MetaInfo&, const input::Traits<Type>& ) 
        { return false; }

    Config config;
  public:
    ChainLink() {}

    void attach_ui( simparm::NodeHandle at ) { config.attach_ui( at ); }
    static std::string getName() { return "Mirror"; }
};

bool need_range(output::LocalizedImage*) { return true; }
void mirror(const localization::MetaInfo<localization::PositionY::ValueType>::RangeType& range,
            output::LocalizedImage& localizations) {
    for (auto& l : localizations) {
        l.position_y() = *range.second - l.position().y() + *range.first;
    }
}

bool need_range(engine::ImageStack*) { return false; }
void mirror(const localization::MetaInfo<localization::PositionY::ValueType>::RangeType& range,
            dStorm::engine::ImageStack& image) {
    for (int i = 0; i < image.plane_count(); ++i) {
        image.plane(i) = image::mirror(image.plane(i), 1);
    }
}

template <typename Type>
bool Source<Type>::GetNext(int thread, Type* target) {
    if ( need_range(target) &&
            (! range.first.is_initialized() || ! range.second.is_initialized()) ) {
        throw std::runtime_error("Range for Y coordinate unknown, cannot mirror results");
    }

    if (!input::AdapterSource<Type>::GetNext(thread, target)) {
        return false;
    }

    mirror(range, *target);
    return true;
}


template <typename Type>
void Source< Type >::modify_traits( input::Traits<Type>& t ) {
    range = t.position_y().range();
}

template <>
void Source< engine::ImageStack >::modify_traits( input::Traits<engine::ImageStack>& ) {}

Config::Config() 
: name_object( ChainLink::getName(), "Mirror input data along Y axis"),
  mirror_y("MirrorY", false)
{
    mirror_y.set_user_level( simparm::Expert );
}

std::auto_ptr<input::Link> makeLink() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

}
}
