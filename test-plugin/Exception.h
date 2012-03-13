#ifndef TESTPLUGIN_EXCEPTION_H
#define TESTPLUGIN_EXCEPTION_H

#include <simparm/Entry.hh>
#include <simparm/Entry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>
#include <stdexcept>

struct Exception
: public dStorm::output::OutputObject
{
    struct _Config;
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<Exception> Source;
    bool onAnnouncement;
    int onImageNumber;

    void segfault() {
        std::cerr << "Throwing test exception\n";
        throw std::runtime_error("Test Exception"); 
    }

    Exception(const Config& config) ;
    Exception* clone() const;

    AdditionalData announceStormSize(const Announcement&)
        { if ( onAnnouncement ) segfault(); return AdditionalData(); }
    void receiveLocalizations(const EngineResult& er) {
        std::cerr << "Got " << er.forImage.value() << " " << onImageNumber << "\n";
        if ( er.forImage.value() == onImageNumber )
            segfault();
    }

};

struct Exception::_Config
 : public simparm::Object 
{
    simparm::BoolEntry onConstruction, onAnnouncement;
    simparm::Entry<long> onImageNumber;

    _Config();
    void registerNamedEntries() {
        push_back( onConstruction );
        push_back( onAnnouncement );
        push_back( onImageNumber );
    }
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
};

Exception::_Config::_Config()
 : simparm::Object("Exception", "Exception"),
   onConstruction("OnConstruction", "Throw Exception on output construction", false), 
   onAnnouncement("OnAnnouncement", "Throw Exception on announcement", false), 
   onImageNumber("OnImageNumber", "Throw Exception on given image number", -1)
{
}

Exception* Exception::clone() const
        { return new Exception(*this); }

Exception::Exception( const Config& config )
        : OutputObject("SegFault", "SegFault") ,
          onAnnouncement( config.onAnnouncement() ),
          onImageNumber( config.onImageNumber() )
        { if ( config.onConstruction() ) segfault(); }

#endif
