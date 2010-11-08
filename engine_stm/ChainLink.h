#ifndef DSTORM_STM_ENGINE_CHAINLINK_H
#define DSTORM_STM_ENGINE_CHAINLINK_H

#include "ChainLink_decl.h"
#include "Config.h"
#include <dStorm/input/chain/Filter.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/Source.h>
#include <dStorm/localization_file/record.h>
#include <dStorm/output/LocalizedImage.h>
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace engine_stm {

class ChainLink
: public input::chain::TypedFilter<LocalizationFile::Record>,
  public input::chain::TypedFilter<Localization>,
  public input::chain::TypedFilter<output::LocalizedImage>
{
    input::chain::Context::Ptr my_context;
    Config config;

    template <typename Type>
    AtEnd _traits_changed( TraitsRef r, Link*, 
        boost::shared_ptr< input::Traits<Type> > );

  public:
    ChainLink* clone() const { return new ChainLink(*this); }

    simparm::Node& getNode() { return config; }

    input::BaseSource* makeSource() 
        { return dispatch_makeSource( ( my_context.get() && my_context->throw_errors ) ? ThrowError : InvalidTraits); }
    input::Source<output::LocalizedImage>*
        makeSource( std::auto_ptr< input::Source<LocalizationFile::Record> > );
    input::Source<output::LocalizedImage>*
        makeSource( std::auto_ptr< input::Source<Localization> > );
    input::Source<output::LocalizedImage>*
        makeSource( std::auto_ptr< input::Source<output::LocalizedImage> > i )
        { return i.release(); }

    AtEnd traits_changed( TraitsRef r, Link* l ) 
        { return dispatch_trait_change(r, l, ( my_context.get() && my_context->throw_errors ) ? ThrowError : InvalidTraits); }
    AtEnd traits_changed( TraitsRef r, Link* l, 
        boost::shared_ptr< input::Traits<Localization> > t ) 
        { return _traits_changed(r,l,t); }
    AtEnd traits_changed( TraitsRef r, Link* l, boost::shared_ptr< input::Traits<output::LocalizedImage> > t)
        { return _traits_changed(r,l,t); }
    AtEnd traits_changed( TraitsRef r, Link* l, boost::shared_ptr< input::Traits<LocalizationFile::Record> > t)
        { return _traits_changed(r,l,t); }

    AtEnd context_changed(ContextRef, Link*);

    void modify_context( input::Traits<output::LocalizedImage>& ) { assert(false); }
    void notice_context( const input::Traits<output::LocalizedImage>& ) { assert(false); }
    void modify_context( input::Traits<LocalizationFile::Record>& ) { assert(false); }
    void notice_context( const input::Traits<LocalizationFile::Record>& ) { assert(false); }
    void modify_context( input::Traits<Localization>& ) { assert(false); }
    void notice_context( const input::Traits<Localization>& ) { assert(false); }
};

}
}

#endif
