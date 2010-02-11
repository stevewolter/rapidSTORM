#ifndef TESTPLUGIN_DELAYER_H
#define TESTPLUGIN_DELAYER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>
#include <unistd.h>

struct Delayer
: public dStorm::output::OutputObject
{
    struct _Config;
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<Delayer> Source;

    Delayer(const Config& config) ;
    ~Delayer();
    Delayer* clone() const;

    AdditionalData announceStormSize(const Announcement& a) { 
        return AdditionalData(); 
    }
    Result receiveLocalizations(const EngineResult& er) {
#ifdef HAVE_USLEEP
        usleep( 1E6 );
#elif HAVE_WINDOWS_H
	Sleep( 1000 );
#endif
        return KeepRunning;
    }
    void propagate_signal(ProgressSignal s) {
    }

};

struct Delayer::_Config
 : public simparm::Object 
{
    _Config();
    void registerNamedEntries() {}
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
};

Delayer::_Config::_Config()
 : simparm::Object("Delayer", "Delayer")
{
}

Delayer* Delayer::clone() const { 
    return new Delayer(*this); 
}

Delayer::Delayer( const Config& config )
        : OutputObject("Delayer", "Delayer")
{
}

Delayer::~Delayer()
{
}

#endif
