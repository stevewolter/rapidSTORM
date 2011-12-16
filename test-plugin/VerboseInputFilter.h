#ifndef VERBOSE_INPUT_FILTER_H
#define VERBOSE_INPUT_FILTER_H

#include <dStorm/log.h>
#include <dStorm/input/AdapterSource.h>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <simparm/Object.hh>
#include <boost/iterator/iterator_adaptor.hpp>
#include <dStorm/input/chain/Filter.h>
#include <dStorm/input/chain/DefaultFilterTypes.h>

namespace VerboseInputFilter {

struct Config : public simparm::Object {
    typedef dStorm::input::chain::DefaultTypes SupportedTypes;
    simparm::BoolEntry verbose;
    Config() : Object("VerboseInput", "Verbose input filter"), 
               verbose("BeVerbose", "Be verbose") {}
    void registerNamedEntries() { push_back(verbose); }
};

template <typename Type>
class Source : public Config, public dStorm::input::AdapterSource<Type> {
    typedef typename dStorm::input::Source<Type>::iterator iterator;
    class _iterator 
      : public boost::iterator_adaptor<_iterator, iterator>
    {
        public:
            _iterator() : _iterator::iterator_adaptor_() {}
            _iterator(iterator i) : _iterator::iterator_adaptor_(i) {}
        
        private:
            friend class boost::iterator_core_access;
            inline void increment(); 
    };
  public:
    Source(const Config& c, std::auto_ptr< dStorm::input::Source<Type> > base) 
        : Config(c), dStorm::input::AdapterSource<Type>(base) {}
    iterator begin() { return iterator(_iterator(this->base().begin())); }
    iterator end() { return iterator(_iterator(this->base().end())); }
};

template <>
void Source<dStorm::Localization>::_iterator::increment()
{ LOG("Advancing from " << this->base()->frame_number()); ++this->base_reference(); }

struct OstreamVisitor : public boost::static_visitor<std::ostream&>
{
    std::ostream& o;
    OstreamVisitor(std::ostream& o) : o(o) {}
    std::ostream& operator()(const dStorm::Localization& l)
        { return (o << l); }
    std::ostream& operator()(const dStorm::localization::EmptyLine& i)
        { return (o << i); }
};

std::ostream& operator<<( std::ostream& o, dStorm::localization::Record& r )
{
    OstreamVisitor v(o);
    return boost::apply_visitor(v, r);
}

template <>
void Source<dStorm::localization::Record>::_iterator::increment()
{ 
    LOG("Advancing from " << *this->base());
    ++this->base_reference();
}

template <typename Type>
void Source<Type>::_iterator::increment()
{ ++this->base_reference(); }

class VerboseInputFilter 
: public dStorm::input::chain::Filter
{
  public:
    typedef dStorm::input::chain::DefaultVisitor<Config> Visitor;
    simparm::Structure<Config> config;
    const Config& get_config() { return config; }

    VerboseInputFilter() : dStorm::input::chain::Filter() {}
    ~VerboseInputFilter() {}
    VerboseInputFilter* clone() const { return new VerboseInputFilter(*this); }

    AtEnd traits_changed( TraitsRef, Link* );
    AtEnd context_changed( ContextRef, Link* );
    dStorm::input::BaseSource* makeSource();

    simparm::Node& getNode() { return static_cast<Config&>(config); }
};

}

#endif
