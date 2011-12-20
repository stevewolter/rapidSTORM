#ifndef DEBUGPLUGIN_VERBOSE_INPUT_FILTER_DECL_H
#define DEBUGPLUGIN_VERBOSE_INPUT_FILTER_DECL_H

#include <dStorm/input/chain/Link.h>

std::auto_ptr<dStorm::input::chain::Link>
make_verbose_input_filter();

#endif
