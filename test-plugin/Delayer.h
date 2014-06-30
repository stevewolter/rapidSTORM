#ifndef TESTPLUGIN_DELAYER_H
#define TESTPLUGIN_DELAYER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "simparm/Entry.h"
#include "simparm/Entry.h"
#include "output/Output.h"
#include "output/OutputBuilder.h"
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>
#include <unistd.h>

struct Delayer
: public dStorm::output::Output
{
    struct Config;

    Delayer(const Config& config) ;
    ~Delayer();
    Delayer* clone() const;

    void announceStormSize(const Announcement& a) OVERRIDE {}
    void receiveLocalizations(const EngineResult& er) OVERRIDE {
#ifdef HAVE_USLEEP
        usleep( 1E5 );
#elif HAVE_WINDOWS_H
	Sleep( 100 );
#endif
    }

};

struct Delayer::Config
{
    static std::string get_name() { return "Delayer"; }
    static std::string get_description() { return "Delayer"; }
    static simparm::UserLevel get_user_level() { return simparm::Debug; }
    void attach_ui( simparm::NodeHandle ) {}
};

Delayer* Delayer::clone() const { 
    return new Delayer(*this); 
}

Delayer::Delayer( const Config& )
{
}

Delayer::~Delayer()
{
}

#endif
