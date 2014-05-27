#ifndef TESTPLUGIN_REPEATTRIGGER_H
#define TESTPLUGIN_REPEATTRIGGER_H

#include "simparm/TriggerEntry.h"
#include "output/Output.h"
#include "output/OutputBuilder.h"
#include "base/Engine.h"

struct Repeat
: public dStorm::output::Output
{
    struct Config {
        static std::string get_name() { return "RepeatTrigger"; }
        static std::string get_description() { return "Repeat trigger"; }
        static simparm::UserLevel get_user_level() { return simparm::Debug; }
        void attach_ui( simparm::NodeHandle ) {}
    };
    dStorm::Engine *r;
    simparm::TriggerEntry repeat;
    simparm::BaseAttribute::ConnectionStore listening;

    void attach_ui_( simparm::NodeHandle at ) { 
        listening = repeat.value.notify_on_value_change( 
            boost::bind( &Repeat::repeat_results, this ) );
        repeat.attach_ui( at ); 
    }
    Repeat(const Config&) 
        : r(NULL), repeat("Repeat", "Repeat results")
        { repeat.hide();  }
    Repeat* clone() const { return new Repeat(*this); }

    AdditionalData announceStormSize(const Announcement& a) { 
        r = a.engine;
        repeat.set_visibility( r );
        return AdditionalData(); 
    }
    void receiveLocalizations(const EngineResult&) {}

private:
    void repeat_results() {
        if ( r && repeat.triggered() ) { 
            repeat.untrigger();
            r->repeat_results(); 
        } 
    }
};

#endif
