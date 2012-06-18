#ifndef DSTORM_WX_UI_LAUNCHER_H
#define DSTORM_WX_UI_LAUNCHER_H

#include <simparm/TriggerEntry.h>
#include <dStorm/Config.h>

namespace simparm {
namespace wx_ui {

class Launcher
{
    simparm::TriggerEntry trigger;
    const dStorm::JobConfig& rapidstorm_job;
    simparm::BaseAttribute::ConnectionStore listening;
    void was_triggered();
  public:
    Launcher( const dStorm::JobConfig& rapidstorm_job );
    ~Launcher();
    void attach_ui( simparm::NodeHandle );
    void launch();
};

}
}

#endif
