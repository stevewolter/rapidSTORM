#ifndef DSTORM_WX_UI_LAUNCHER_H
#define DSTORM_WX_UI_LAUNCHER_H

#include <simparm/TriggerEntry.h>
#include "job/Config.h"

namespace simparm {
namespace wx_ui {

class Launcher
{
    simparm::TriggerEntry trigger;
    dStorm::job::Config &config;
    simparm::BaseAttribute::ConnectionStore listening;
    void was_triggered();
  public:
    Launcher(dStorm::job::Config&);
    ~Launcher();
    void attach_ui( simparm::NodeHandle );
    void launch();
};

}
}

#endif
