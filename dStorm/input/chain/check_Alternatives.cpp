#include "debug.h"

#include <dStorm/helpers/thread.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/Image.h>
#include <dStorm/image/constructors.h>
#include "Alternatives.h"
#include "Filter.h"
#include "MetaInfo.h"
#include "Context.h"
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/input/chain/Filter_impl.h>

using namespace dStorm::input;
using namespace dStorm::input::chain;

struct Input : public Terminus {
    bool declared;
    bool current;
    simparm::Object node;
    Input() : Terminus(), declared(false), current(false), node("Input", "Input") {}
    Input *clone() const { return new Input(*this); }
    simparm::Node& getNode() { return node; }

    AtEnd context_changed( ContextRef ctx, Link* link ) {
        DEBUG("Context change with " << ctx.get());
        if ( ctx.get() && (!declared || ctx->need_multiple_concurrent_iterators != current) ) {
            current = ctx->need_multiple_concurrent_iterators;
            MetaInfo::Ptr mi( new MetaInfo() );
            if ( current )
                mi->set_traits( new Traits<dStorm::Localization>() );
            else
                mi->set_traits( new Traits<dStorm::engine::Image>() );
            declared = true;
            return notify_of_trait_change( mi );
        } else
            return AtEnd();
    }

    BaseSource* makeSource() { assert( false ); throw std::logic_error(""); }
};

struct ImageAlternative : public dStorm::input::chain::Filter
{
    simparm::Object node;

    ImageAlternative() : node("AIm", "AIm") {}
    ImageAlternative *clone() const { return new ImageAlternative(*this); }
    simparm::Node& getNode() { return node; }

    AtEnd traits_changed( TraitsRef c, Link* l ) {
        if ( c->provides<dStorm::engine::Image>() ) {
            DEBUG("Traits say it's an image");
            return notify_of_trait_change( c );
        } else {
            DEBUG("Traits say it's no image");
            return notify_of_trait_change( TraitsRef() );
        }
    }

    AtEnd context_changed( ContextRef c, Link *l ) {
        DEBUG("Context changed for image alternative");
        return notify_of_context_change(c);
    }

    BaseSource* makeSource() { assert( false ); throw std::logic_error(""); }
};

struct LocalizationAlternative : public dStorm::input::chain::Filter
{
    simparm::Object node;

    LocalizationAlternative() : node("AL", "AL") {}
    LocalizationAlternative *clone() const { return new LocalizationAlternative(*this); }
    simparm::Node& getNode() { return node; }

    AtEnd traits_changed( TraitsRef c, Link* l ) {
        if ( c->provides<dStorm::Localization>() ) {
            DEBUG("Traits " << c.get() << " provide localizations");
            return notify_of_trait_change( c );
        } else {
            DEBUG("Traits " << c.get() << " provide no localizations");
            return notify_of_trait_change( TraitsRef() );
        }
    }

    AtEnd context_changed( ContextRef c, Link *l ) {
        DEBUG("Context changed for localization alternative");
        return notify_of_context_change(c);
    }

    BaseSource* makeSource() {  assert( false ); throw std::logic_error(""); }
};

struct ChainBase : public Forwarder
{
    simparm::Object node;
    ChainBase() : node("Chain", "Chain") {}
    ChainBase* clone() const { return new ChainBase(*this); }
    simparm::Node& getNode() { return node; }

    TraitsRef current_traits;

    AtEnd traits_changed( TraitsRef t, Link* )  { current_traits = t; return AtEnd(); }
    AtEnd context_changed( ContextRef, Link* ) { return AtEnd(); }
    void set_context( ContextRef c ) { notify_of_context_change(c); }
    BaseSource* makeSource() { return Forwarder::makeSource(); }
};

std::string getName( const Alternatives& a )
{
    if ( a.isValid() ) return a.value().getNode().getName(); else return "";
}

int main() {
    ost::DebugStream::set(std::cerr);

    Input input;
    Alternatives alternatives("Alternatives", "Alternatives");
    ChainBase chain_base;

    ImageAlternative image_alternative;
    LocalizationAlternative loc_alternative;

    alternatives.set_more_specialized_link_element( &input );
    chain_base.set_more_specialized_link_element( &alternatives );
    alternatives.push_back_choice( image_alternative );
    alternatives.push_back_choice( loc_alternative );

    if ( getName(alternatives) != "" ) return 1;

    Context::Ptr c( new Context() );
    chain_base.set_context( c );

    if ( getName(alternatives) != "AIm" ) return 1;

    c.reset( new Context() );
    c->need_multiple_concurrent_iterators = true;
    chain_base.set_context( c );

    if ( getName(alternatives) != "AL" ) return 1;

    return 0;
}
