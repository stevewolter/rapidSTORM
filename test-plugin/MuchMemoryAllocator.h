#ifndef TESTPLUGIN_MEMALLOC_H
#define TESTPLUGIN_MEMALLOC_H

#include <simparm/Entry.hh>
#include <simparm/Entry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>
#include <boost/ptr_container/ptr_list.hpp>

struct Memalloc
: public dStorm::output::OutputObject
{
    struct _Config;
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<Memalloc> Source;

    struct large_struct { double array[100000]; };
    boost::ptr_list<large_struct> much_memory;

    bool destroy;

    Memalloc(const Config& config) ;
    Memalloc* clone() const;

    AdditionalData announceStormSize(const Announcement&)
        { return AdditionalData(); }
    Result receiveLocalizations(const EngineResult& er) {
        try {
            much_memory.push_back( new large_struct() );
            if ( er.forImage.value() % 999 == 998 ) {
                std::cerr << "Emptying memory\n";
                much_memory.clear();
            }
            return KeepRunning;
        } catch ( const std::bad_alloc& alloc ) {
            std::cerr << "Memalloc is out of memory\n";
            if ( destroy )
                return RemoveThisOutput;
            else
                return KeepRunning;
        }
    }
    void propagate_signal(ProgressSignal) {}

};

struct Memalloc::_Config
 : public simparm::Object 
{
    simparm::BoolEntry destroy_on_out_of_memory;

    _Config();
    void registerNamedEntries() {
        push_back( destroy_on_out_of_memory );
    }
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
};

Memalloc::_Config::_Config()
 : simparm::Object("MuchMem", "MuchMem"),
   destroy_on_out_of_memory("DestroyWhenNoMem", "Destroy output when out of memory", false)
{
}

Memalloc* Memalloc::clone() const
        { return new Memalloc(*this); }

Memalloc::Memalloc( const Config& c )
        : OutputObject("MuchMem", "MuchMem"),
          destroy(c.destroy_on_out_of_memory()) {}

#endif
