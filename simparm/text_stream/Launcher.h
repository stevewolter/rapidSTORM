#ifndef SIMPARM_TEXT_STREAM_LAUNCHER_H
#define SIMPARM_TEXT_STREAM_LAUNCHER_H

#include "job/Config.h"
#include "MainThread.h"
#include <simparm/TriggerEntry.h>

namespace simparm {
namespace text_stream {

class Launcher
{
    simparm::TriggerEntry trigger;
    dStorm::job::Config &config;
    dStorm::MainThread& main_thread;
    simparm::BaseAttribute::ConnectionStore listening;
    void run_twiddler();
  public:
    Launcher(dStorm::job::Config&, dStorm::MainThread& main_thread);
    ~Launcher();
    void attach_ui( simparm::NodeHandle );
    void launch() { run_twiddler(); }
};

}
}

#endif
