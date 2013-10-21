#include "YMirror.h"

#include <boost/iterator/transform_iterator.hpp>
#include <boost/smart_ptr/shared_array.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <dStorm/Image.h>
#include <dStorm/image/mirror.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/Method.hpp>
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
    typedef Type Input;
    typedef input::Source<Input> Base;
    typedef Localization::PositionY::Traits::RangeType Range;
    Range range;
    struct iterator;
    void modify_traits( input::Traits<Type>& );
    void attach_local_ui_( simparm::NodeHandle ) {}

  public:
    Source( std::auto_ptr< Base > base ) : input::AdapterSource<Type>(base) {}
    typename Base::iterator begin();
    typename Base::iterator end();
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

struct Mirrorer : public boost::static_visitor<void> {
    typedef Localization::PositionY::Traits::RangeType Range;
    const Range& range;
    Mirrorer(const Range& range) : range(range) {}
    template <typename Type>
    inline void operator()( Type& type );
};

template <> 
void Mirrorer::operator()<Localization>( Localization& l )
{
    l.position_y() = *range.second - l.position().y() + *range.first;
}

template <> 
void Mirrorer::operator()<const dStorm::Localization>( const dStorm::Localization& l )
{
    (*this)( const_cast<Localization&>(l) );
}

template <> 
void Mirrorer::operator()<output::LocalizedImage>( output::LocalizedImage& img )
{
    std::for_each( img.begin(), img.end(), *this );
}

template <> 
void Mirrorer::operator()<dStorm::engine::ImageStack>( dStorm::engine::ImageStack& l )
{
    for (int i = 0; i < l.plane_count(); ++i)
    {
        l.plane(i) = image::mirror(l.plane(i), 1);
    }
}

template <typename Type>
struct Source<Type>::iterator
: public boost::iterator_adaptor< iterator, typename Base::iterator >
{
    iterator( Range r, const typename Base::iterator& o ) ;
  private:
    friend class boost::iterator_core_access;
    mutable Type converted;
    mutable bool initialized;
    Range range;

    inline void copy() const;
    inline bool need_range() const;

    void increment() { ++this->base_reference(); initialized = false; }
    Type& dereference() const {
        if ( ! initialized ) {
            copy();
            Mirrorer m(range);
            m( converted );
            initialized = true;
        }
        return converted;
    }
};

template <typename Type>
void Source<Type>::iterator::copy() const {
    converted = *this->base();
}

template <typename Type>
bool Source<Type>::iterator::need_range() const { return true; }
template <>
bool Source<dStorm::engine::ImageStack>::iterator::need_range() const { return false; }

template <typename Type>
Source< Type >::iterator::iterator( Range r, const typename Base::iterator& o )
        : iterator::iterator_adaptor_(o), initialized(false), range(r) 
    {
        if ( need_range() && (! range.first.is_initialized() || ! range.second.is_initialized()) )
            throw std::runtime_error("Range for Y coordinate unknown, cannot mirror results");
    }


template <typename Type>
typename Source< Type>::Base::iterator
Source< Type>::begin() {
    return typename Base::iterator( iterator( range, this->base().begin() ) );
}

template <typename Type>
typename Source< Type >::Base::iterator
Source< Type >::end() {
    return typename Base::iterator( iterator( range, this->base().end() ) );
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

