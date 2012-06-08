#ifndef SIMPARM_TEXT_STREAM_LAUNCHER_H
#define SIMPARM_TEXT_STREAM_LAUNCHER_H

#include "job/Config.h"
#include <simparm/TriggerEntry.h>

namespace simparm {
namespace text_stream {

class Launcher
{
    simparm::TriggerEntry trigger;
    dStorm::job::Config &config;
    simparm::BaseAttribute::ConnectionStore listening;
    void run_twiddler();
    bool wxWidgets;
  public:
    Launcher(dStorm::job::Config&, bool wxWidgets);
    ~Launcher();
    void attach_ui( simparm::NodeHandle );
    void launch() { run_twiddler(); }
};

}
}

#endif
