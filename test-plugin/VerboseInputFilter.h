#ifndef VERBOSE_INPUT_FILTER_H
#define VERBOSE_INPUT_FILTER_H

#include <dStorm/log.h>
#include <dStorm/input/AdapterSource.h>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <simparm/Object.hh>
#include <boost/iterator/iterator_adaptor.hpp>

namespace VerboseInputFilter {

struct Config : public simparm::Object {
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
            inline void increment() {
                LOG("Advancing from " << this->base()->frame_number());
                ++this->base_reference();
            }
    };
    void attach_local_ui_( simparm::Node& ) {}
  public:
    Source(const Config& c, std::auto_ptr< dStorm::input::Source<Type> > base) 
        : Config(c), dStorm::input::AdapterSource<Type>(base) {}
    iterator begin() { return iterator(_iterator(this->base().begin())); }
    iterator end() { return iterator(_iterator(this->base().end())); }
};

}

#endif
