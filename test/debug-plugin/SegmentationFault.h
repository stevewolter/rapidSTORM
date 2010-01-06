#ifndef TESTPLUGIN_SEGFAULT_H
#define TESTPLUGIN_SEGFAULT_H

#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <iostream>

struct SegmentationFault
: public dStorm::output::OutputObject
{
    struct _Config;
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<SegmentationFault> Source;

    SegmentationFault(const Config&) 
        : OutputObject("SegFault", "SegFault") {}
    SegmentationFault* clone() const;

    AdditionalData announceStormSize(const Announcement&)
        { return AdditionalData(); }
    Result receiveLocalizations(const EngineResult& er) {
        if ( er.forImage.value() % 99 == 96 ) {
            std::cerr << "Provoking segfault\n";
            *(int*)(0x23) = 0;
        }
        return KeepRunning;
    }
    void propagate_signal(ProgressSignal) {}

};

struct SegmentationFault::_Config
 : public simparm::Object 
{
    simparm::BoolEntry onConstruction, onAnnouncement;
    simparm::LongEntry onImageNumber;

    _Config();
    void registerNamedEntries() {
        push_back( onConstruction );
        push_back( onAnnouncement );
        push_back( onImageNumber );
    }
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
};

SegmentationFault::_Config::_Config()
 : simparm::Object("SegFault", "SegFault"),
   onConstruction("OnConstruction", "Throw segfault on output construction", false), 
   onAnnouncement("OnAnnouncement", "Throw segfault on announcement", false), 
   onImageNumber("OnImageNumber", "Throw segfault on given image number", -1)
{
}

SegmentationFault* SegmentationFault::clone() const
        { return new SegmentationFault(*this); }

#endif
