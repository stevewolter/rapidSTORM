#ifndef VERBOSE_INPUT_FILTER_H
#define VERBOSE_INPUT_FILTER_H

#include <dStorm/input/Source_impl.h>
#include <dStorm/input/chain/FullFilter.h>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <simparm/Object.hh>

struct Config : public simparm::Object {
    simparm::BoolEntry verbose;
    Config() : Object("VerboseInput", "Verbose input filter"), 
               verbose("BeVerbose", "Be verbose") {}
    void registerNamedEntries() { push_back(verbose); }
};

template <typename Type>
class TypedVerboseInputFilter
: public dStorm::input::chain::TypedFilter<Type>,
  public virtual simparm::Structure<Config>
{
    dStorm::input::BaseSource* makeSource
        ( std::auto_ptr< dStorm::input::Source<Type> > rv )
    {
        std::cerr << "Source of type " << typeid(*rv.get()).name() << " is passing" << std::endl;
        return rv.release();
    }

    dStorm::input::chain::Link::AtEnd traits_changed(
        dStorm::input::chain::Link::TraitsRef r, 
        dStorm::input::chain::Link*, 
        boost::shared_ptr< dStorm::input::Traits<Type> > ) 
    {
        return this->notify_of_trait_change(r);
    }

    void modify_context( dStorm::input::Traits<Type>& ) { assert(false); }
    void notice_context( const dStorm::input::Traits<Type>& ) { assert(false); }
};

class VerboseInputFilter 
: public dStorm::input::chain::FullFilter<TypedVerboseInputFilter>
{
  public:
    VerboseInputFilter() : dStorm::input::chain::Filter() {}
    ~VerboseInputFilter() {}
    VerboseInputFilter* clone() const { return new VerboseInputFilter(*this); }

    AtEnd traits_changed( TraitsRef, Link* );
    AtEnd context_changed( ContextRef, Link* );

    dStorm::input::BaseSource* makeSource() { return dispatch_makeSource(PassThrough); }

    simparm::Node& getNode() { return static_cast<Config&>(*this); }
};

#endif
