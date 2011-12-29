#ifndef TESTPLUGIN_PLUGIN_H
#define TESTPLUGIN_PLUGIN_H

#include <dStorm/Config.h>
#include <dStorm/display/Manager.h>

namespace dStorm {
namespace test {

void make_config ( dStorm::Config* config );
dStorm::display::Manager* make_display (dStorm::display::Manager *old);

}
}

#endif
