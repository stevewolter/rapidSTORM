#ifndef VERBOSE_INPUT_FILTER_H
#define VERBOSE_INPUT_FILTER_H

#include <dStorm/log.h>
#include <dStorm/input/AdapterSource.h>
#include <simparm/Entry.hh>
#include <simparm/Object.hh>
#include <boost/iterator/iterator_adaptor.hpp>

namespace VerboseInputFilter {

struct Config {
    simparm::Object name_object;
    simparm::BoolEntry verbose;
    Config() : name_object("VerboseInput", "Verbose input filter"), 
               verbose("BeVerbose", "Be verbose") {}
    void attach_ui( simparm::NodeHandle at ) {
        verbose.attach_ui( name_object.attach_ui( at ) );
    }
};

template <typename Type>
class Source : public dStorm::input::AdapterSource<Type> {
    Config config;
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
    void attach_local_ui_( simparm::NodeHandle ) {}
  public:
    Source(const Config& c, std::auto_ptr< dStorm::input::Source<Type> > base) 
        : dStorm::input::AdapterSource<Type>(base), config(c) {}
    iterator begin() { return iterator(_iterator(this->base().begin())); }
    iterator end() { return iterator(_iterator(this->base().end())); }
};

}

#endif
