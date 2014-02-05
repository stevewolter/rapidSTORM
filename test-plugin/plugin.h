#ifndef TESTPLUGIN_PLUGIN_H
#define TESTPLUGIN_PLUGIN_H

#include "core/Config.h"
#include "display/Manager.h"

namespace dStorm {
namespace test {

void input_modules ( dStorm::Config* config );
void output_modules ( dStorm::Config* config );

}
}

#endif
