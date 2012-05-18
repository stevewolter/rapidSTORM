#ifndef TESTPLUGIN_REPEATTRIGGER_H
#define TESTPLUGIN_REPEATTRIGGER_H

#include <simparm/TriggerEntry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/Engine.h>

struct Repeat
: public dStorm::output::OutputObject, public simparm::Listener
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

    Repeat(const Config&) 
        : OutputObject("Repeater", "Result repeater"), 
          simparm::Listener( simparm::Event::ValueChanged ),
            r(NULL), repeat("Repeat", "Repeat results")
        { repeat.viewable = false; push_back( repeat ); receive_changes_from( repeat.value ); }
    Repeat(const Repeat& a) : OutputObject(a),
          simparm::Listener( simparm::Event::ValueChanged ),
          r(a.r), repeat(a.repeat)
        { push_back( repeat ); receive_changes_from( repeat.value );  }
    ~Repeat() {}
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
