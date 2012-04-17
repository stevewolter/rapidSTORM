#ifndef TEST_PLUGIN_DISPLAY_MANAGER_H
#define TEST_PLUGIN_DISPLAY_MANAGER_H

#include <memory>

namespace dStorm { namespace display { class Manager; } }

std::auto_ptr< dStorm::display::Manager > make_test_plugin_manager( dStorm::display::Manager* old );

#endif
