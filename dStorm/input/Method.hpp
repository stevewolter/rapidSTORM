#ifndef DSTORM_INPUT_METHOD_H
#define DSTORM_INPUT_METHOD_H

#include <dStorm/input/chain/Forwarder.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/chain/DefaultFilterTypes.h>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace input {

template <typename CRTP>
struct Method
: public chain::Forwarder
{
    chain::Forwarder* clone() const { return new CRTP( static_cast<const CRTP&>(*this) ); }
    BaseSource* makeSource();
    AtEnd traits_changed( TraitsRef, Link* );

  protected:
    typedef chain::DefaultTypes SupportedTypes;
    bool ignore_unknown_type() const { return false; }
    template <typename Type>
    Source<Type>* make_source( std::auto_ptr< Source<Type> > ) {
        throw std::logic_error("Source creation not implemented");
    }
    template <typename Type>
    bool changes_traits( const chain::MetaInfo&, const Traits<Type>& )
        { return true; }
    template <typename Type>
    void notice_traits( const chain::MetaInfo&, const Traits<Type>& ) {}
    template <typename Type>
    inline void update_traits( chain::MetaInfo&, Traits<Type>& ) {
        throw std::logic_error("Trait update not implemented");
    }

    void republish_traits() { 
        if ( upstream_traits().get() )
            traits_changed( chain::Forwarder::upstream_traits(), NULL ); 
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
                boost::shared_ptr< chain::MetaInfo > my_info( new chain::MetaInfo(*orig) );
                boost::shared_ptr< Traits<Type> > my_traits( new Traits<Type>(*orig_traits) );
                static_cast<CRTP&>(me).update_traits( *my_info, *my_traits );
                my_info->set_traits( my_traits );
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
    Src orig( Forwarder::makeSource() ), result;
    boost::mpl::for_each< typename CRTP::SupportedTypes >(
        boost::bind( source_maker(), boost::ref(*this), _1, boost::ref(orig),
                     boost::ref(result) ) );
    if ( result.get() )
        return result.release();
    else if ( static_cast<CRTP&>(*this).ignore_unknown_type() )
        return orig.release();
    else
        throw std::runtime_error(getNode().getName() + " cannot process input "
            "of the current type");
}

template <typename CRTP>
chain::Link::AtEnd Method<CRTP>::traits_changed( TraitsRef orig, Link* ) {
    TraitsRef result;
    if ( orig.get() )
        boost::mpl::for_each< typename CRTP::SupportedTypes >(
            boost::bind( make_traits(), boost::ref(*this), _1, orig,
                        boost::ref(result) ) );
    if ( result.get() )
        return this->notify_of_trait_change(result);
    else if ( static_cast<CRTP&>(*this).ignore_unknown_type() )
        return this->notify_of_trait_change(orig);
    else
        return this->notify_of_trait_change( TraitsRef() );
}

}
}

#endif
