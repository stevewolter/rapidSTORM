#ifndef TESTPLUGIN_REPEATTRIGGER_H
#define TESTPLUGIN_REPEATTRIGGER_H

#include <simparm/TriggerEntry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/Engine.h>

struct Repeat
: public dStorm::output::OutputObject, public simparm::Listener
{
    struct _Config
        : public simparm::Object 
        {
            _Config() : simparm::Object("RepeatTrigger", "Repeat trigger") {}
            void registerNamedEntries() {}
            bool can_work_with(const dStorm::output::Capabilities&)
                {return true;}
        };
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<Repeat> Source;
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
    Result receiveLocalizations(const EngineResult& er) { return KeepRunning; }
    void propagate_signal(ProgressSignal) {}

    void operator()( const simparm::Event& ) {
        if ( r && repeat.triggered() ) { 
            repeat.untrigger();
            r->repeat_results(); 
            LOG("Repeating results"); 
        } 
    }
};

#endif
