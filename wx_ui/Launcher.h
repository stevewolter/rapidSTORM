#ifndef DSTORM_WX_UI_LAUNCHER_H
#define DSTORM_WX_UI_LAUNCHER_H

#include <simparm/TriggerEntry.h>
#include "job/Config.h"
#include "MainThread.h"

namespace simparm {
namespace wx_ui {

class Launcher
: public simparm::TriggerEntry
{
    dStorm::job::Config &config;
    dStorm::MainThread& main_thread;
    simparm::BaseAttribute::ConnectionStore listening;
    void launch();
  public:
    Launcher(dStorm::job::Config&, dStorm::MainThread& main_thread);
    ~Launcher();
    void attach_ui( simparm::NodeHandle );
};

}
}

#endif
