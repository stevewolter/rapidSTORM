#ifndef DSTORM_INPUT_METHOD_H
#define DSTORM_INPUT_METHOD_H

#include <dStorm/input/Forwarder.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/DefaultFilterTypes.h>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace input {

template <typename CRTP>
class Method
: public Forwarder
{
    typedef boost::shared_ptr< const MetaInfo > TraitsRef;
    BaseSource* makeSource();

  public:
    Forwarder* clone() const { return new CRTP( static_cast<const CRTP&>(*this) ); }
    void traits_changed( TraitsRef, Link* );
    void registerNamedEntries( simparm::Node& node ) { 
        Forwarder::registerNamedEntries( node );
        node.push_back( static_cast<CRTP&>(*this).getNode() ); 
    }
    std::string name() const
        { return const_cast<CRTP&>(static_cast<const CRTP&>(*this)).getNode().getName(); }
    std::string description() const 
        { return static_cast<simparm::Object&>(const_cast<CRTP&>(static_cast<const CRTP&>(*this)).getNode()).getDesc(); }

  protected:
    typedef DefaultTypes SupportedTypes;
    bool ignore_unknown_type() const { return false; }
    template <typename Type>
    BaseSource* make_source( std::auto_ptr< Source<Type> > ) {
        throw std::logic_error("Source creation not implemented");
    }
    template <typename Type>
    bool changes_traits( const MetaInfo&, const Traits<Type>& )
        { return true; }
    template <typename Type>
    void notice_traits( const MetaInfo&, const Traits<Type>& ) {}
    template <typename Type>
    void update_traits( MetaInfo&, Traits<Type>& ) {
        throw std::logic_error("Trait update not implemented");
    }
    void update_meta_info( MetaInfo& ) {}
    template <typename Type>
    BaseTraits* create_traits( MetaInfo& my_info,
                               const Traits<Type>& orig_traits ) 
    {
        std::auto_ptr< Traits<Type> > my_traits( new Traits<Type>(orig_traits) );
        static_cast<CRTP&>(*this).update_traits( my_info, *my_traits );
        return my_traits.release();
    }

    void republish_traits() { 
        if ( Forwarder::upstream_traits().get() )
            traits_changed( Forwarder::upstream_traits(), NULL ); 
    }
  private:
    typedef std::auto_ptr<BaseSource> Src;
    struct source_maker;
    struct make_traits;
};

template <typename CRTP>
struct Method<CRTP>::make_traits {
    typedef void result_type;
    template <typename Type>
    void operator()( Method<CRTP>& me, Type, TraitsRef orig, TraitsRef& result)
    {
        if ( result.get() ) return;
        if ( orig->provides<Type>() ) {
            boost::shared_ptr< const Traits<Type> > orig_traits
                = orig->traits<Type>();
            static_cast<CRTP&>(me).notice_traits( *orig, *orig_traits );
            if ( static_cast<CRTP&>(me).changes_traits( *orig, *orig_traits ) ) {
                boost::shared_ptr< MetaInfo > my_info( new MetaInfo(*orig) );
                my_info->set_traits( static_cast<CRTP&>(me).create_traits( *my_info, *orig_traits ) );
                result = my_info;
            } else {
                result = orig;
            }
        }
    }
};

template <typename CRTP>
struct Method<CRTP>::source_maker {
    typedef void result_type;
    template <typename Type>
    void operator()( Method<CRTP>& me, Type, Src& orig, Src& result) const
    {
        if ( result.get() ) return;
        Source<Type>* test = dynamic_cast< Source<Type>* >(orig.get());
        if ( test ) {
            std::auto_ptr< Source<Type> > typed = BaseSource::downcast<Type>(orig);
            result.reset( static_cast<CRTP&>(me).make_source(typed) );
        }
    }
};

template <typename CRTP>
BaseSource* Method<CRTP>::makeSource() {
    Src orig( Forwarder::upstream_source() ), result;
    boost::mpl::for_each< typename CRTP::SupportedTypes >(
        boost::bind( source_maker(), boost::ref(*this), _1, boost::ref(orig),
                     boost::ref(result) ) );
    if ( result.get() )
        return result.release();
    else if ( static_cast<CRTP&>(*this).ignore_unknown_type() )
        return orig.release();
    else
        throw std::runtime_error(Forwarder::name() + " cannot process input "
            "of the current type");
}

template <typename CRTP>
void Method<CRTP>::traits_changed( TraitsRef orig, Link* ) {
    TraitsRef result;
    if ( orig.get() )
        boost::mpl::for_each< typename CRTP::SupportedTypes >(
            boost::bind( make_traits(), boost::ref(*this), _1, orig,
                        boost::ref(result) ) );
    if ( result.get() )
        return this->update_current_meta_info(result);
    else if ( static_cast<CRTP&>(*this).ignore_unknown_type() )
        return this->update_current_meta_info(orig);
    else if ( orig.get() ) {
        boost::shared_ptr< MetaInfo > my_info( new MetaInfo(*orig) );
        my_info->set_traits( NULL );
        static_cast<CRTP&>(*this).update_meta_info( *my_info );
        return this->update_current_meta_info( my_info );
    } else {
        return this->update_current_meta_info( orig );
    }
}

}
}

#endif
