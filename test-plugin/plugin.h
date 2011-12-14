#ifndef TESTPLUGIN_PLUGIN_H
#define TESTPLUGIN_PLUGIN_H

#include <dStorm/Config.h>
#include <dStorm/helpers/DisplayManager.h>

namespace dStorm {
namespace test {

void make_config ( dStorm::Config* config );
dStorm::Display::Manager* make_display (dStorm::Display::Manager *old);

}
}

#endif
