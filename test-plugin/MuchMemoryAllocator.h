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

    Memalloc(const Config& config) ;
    Memalloc* clone() const;

    AdditionalData announceStormSize(const Announcement&)
        { return AdditionalData(); }
    void receiveLocalizations(const EngineResult& er) {
        try {
            much_memory.push_back( new large_struct() );
            if ( er.forImage.value() % 999 == 998 ) {
                std::cerr << "Emptying memory\n";
                much_memory.clear();
            }
        } catch ( const std::bad_alloc& alloc ) {
            std::cerr << "Memalloc is out of memory\n";
        }
    }

};

struct Memalloc::_Config
 : public simparm::Object 
{
    _Config();
    void registerNamedEntries() {}
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
};

Memalloc::_Config::_Config()
 : simparm::Object("MuchMem", "MuchMem")
{
}

Memalloc* Memalloc::clone() const
        { return new Memalloc(*this); }

Memalloc::Memalloc( const Config& c )
        : OutputObject("MuchMem", "MuchMem") {}

#endif
