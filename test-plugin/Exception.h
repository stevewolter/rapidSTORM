#ifndef TESTPLUGIN_EXCEPTION_H
#define TESTPLUGIN_EXCEPTION_H

#include "simparm/Entry.h"
#include "simparm/Entry.h"
#include "output/Output.h"
#include "output/OutputBuilder.h"
#include <iostream>
#include <stdexcept>

struct Exception
: public dStorm::output::Output
{
    struct Config;
    bool onAnnouncement;
    int onImageNumber;

    void segfault() {
        std::cerr << "Throwing test exception\n";
        throw std::runtime_error("Test Exception"); 
    }

    Exception(const Config& config) ;
    Exception* clone() const;

    void announceStormSize(const Announcement&) OVERRIDE
        { if ( onAnnouncement ) segfault(); }
    void receiveLocalizations(const EngineResult& er) OVERRIDE {
        std::cerr << "Got " << er.group << " " << onImageNumber << "\n";
        if ( er.group == onImageNumber )
            segfault();
    }

};

struct Exception::Config
{
    simparm::BoolEntry onConstruction, onAnnouncement;
    simparm::Entry<long> onImageNumber;

    Config();
    void attach_ui( simparm::NodeHandle at ) {
        onConstruction.attach_ui( at );
        onAnnouncement.attach_ui( at );
        onImageNumber.attach_ui( at );
    }
    static std::string get_name() { return "Exception"; }
    static std::string get_description() { return "Exception"; }
    static simparm::UserLevel get_user_level() { return simparm::Debug; }
};

Exception::Config::Config()
 : onConstruction("OnConstruction", "Throw Exception on output construction", false), 
   onAnnouncement("OnAnnouncement", "Throw Exception on announcement", false), 
   onImageNumber("OnImageNumber", "Throw Exception on given image number", -1)
{
}

Exception* Exception::clone() const
        { return new Exception(*this); }

Exception::Exception( const Config& config )
        : onAnnouncement( config.onAnnouncement() ),
          onImageNumber( config.onImageNumber() )
        { if ( config.onConstruction() ) segfault(); }

#endif
