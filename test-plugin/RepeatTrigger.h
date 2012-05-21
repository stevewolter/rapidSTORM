#ifndef TESTPLUGIN_REPEATTRIGGER_H
#define TESTPLUGIN_REPEATTRIGGER_H

#include <simparm/TriggerEntry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/Engine.h>

struct Repeat
: public dStorm::output::Output, public simparm::Listener
{
    struct Config {
        static std::string get_name() { return "RepeatTrigger"; }
        static std::string get_description() { return "Repeat trigger"; }
        void attach_ui( simparm::Node& ) {}
        bool can_work_with(const dStorm::output::Capabilities&)
            {return true;}
    };
    dStorm::Engine *r;
    simparm::TriggerEntry repeat;

    void attach_ui_( simparm::Node& at ) { receive_changes_from( repeat.value ); repeat.attach_ui( at ); }
    Repeat(const Config&) 
        : simparm::Listener( simparm::Event::ValueChanged ),
            r(NULL), repeat("Repeat", "Repeat results")
        { repeat.viewable = false;  }
    Repeat* clone() const { return new Repeat(*this); }

    AdditionalData announceStormSize(const Announcement& a) { 
        r = a.engine;
        repeat.viewable = r;
        return AdditionalData(); 
    }
    void receiveLocalizations(const EngineResult&) {}

    void operator()( const simparm::Event& ) {
        if ( r && repeat.triggered() ) { 
            repeat.untrigger();
            r->repeat_results(); 
            LOG("Repeating results"); 
        } 
    }
};

#endif
