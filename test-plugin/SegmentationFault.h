#ifndef TESTPLUGIN_SEGFAULT_H
#define TESTPLUGIN_SEGFAULT_H

#include <simparm/Entry.hh>
#include <simparm/Entry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>

struct SegmentationFault
: public dStorm::output::OutputObject
{
    struct _Config;
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<SegmentationFault> Source;
    bool onAnnouncement;
    int onImageNumber;
    bool onDestruction;

    static void segfault() { 
        std::cerr << "Segfault at 0x23" << std::endl;
        *(int*)(0x23) = 0;
        std::cerr << "Segfault at ~0x23" << std::endl;
        *(int*)(~0x23) = 0;
        std::cerr << "Segfaults finished" << std::endl;
    }

    SegmentationFault(const Config& config) ;
    ~SegmentationFault() { if ( onDestruction) segfault();}
    SegmentationFault* clone() const;

    AdditionalData announceStormSize(const Announcement&)
        { if ( onAnnouncement ) segfault(); return AdditionalData(); }
    Result receiveLocalizations(const EngineResult& er) {
        if ( er.forImage.value() == onImageNumber )
            segfault();
        return KeepRunning;
    }
    void propagate_signal(ProgressSignal) {}

};

struct SegmentationFault::_Config
 : public simparm::Object 
{
    simparm::BoolEntry onConstruction, onAnnouncement,onDestruction;
    simparm::Entry<long> onImageNumber;
    simparm::BoolEntry myCopy, myDestruction;

    _Config();
    _Config(const _Config&);
    ~_Config();
    void registerNamedEntries() {
        push_back( onConstruction );
        push_back( onAnnouncement );
        push_back( onImageNumber );
        push_back( onDestruction );
        push_back( myCopy );
        push_back( myDestruction );
    }
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
};

SegmentationFault::_Config::_Config()
 : simparm::Object("SegFault", "SegFault"),
   onConstruction("OnConstruction", "Throw segfault on output construction", false), 
   onAnnouncement("OnAnnouncement", "Throw segfault on announcement", false), 
   onDestruction("OnDestruction", "Throw segfault on output destruction", false), 
   onImageNumber("OnImageNumber", "Throw segfault on given image number", -1),
   myCopy("OnMyCopy", "Throw segfault on source copy"),
   myDestruction("OnMyDestruction", "Throw segfault on source destruction")
{
}

SegmentationFault::_Config::_Config(const _Config& c)
:   simparm::Object(c),
    onConstruction(c.onConstruction),
    onAnnouncement(c.onAnnouncement),
    onDestruction(c.onDestruction),
    onImageNumber(c.onImageNumber),
    myCopy(c.myCopy),
    myDestruction(c.myDestruction)
{
    if ( myCopy() ) SegmentationFault::segfault(); 
}

SegmentationFault::_Config::~_Config() {
    if ( myDestruction() ) SegmentationFault::segfault(); 
}

SegmentationFault* SegmentationFault::clone() const
        { return new SegmentationFault(*this); }

SegmentationFault::SegmentationFault( const Config& config )
        : OutputObject("SegFault", "SegFault") ,
          onAnnouncement( config.onAnnouncement() ),
          onImageNumber( config.onImageNumber() )
        { if ( config.onConstruction() ) segfault(); }

#endif
