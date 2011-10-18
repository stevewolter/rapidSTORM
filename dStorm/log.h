#include "debug_print.h"

#include "helpers/thread.h"
#include <iostream>
#define LOG(x) { ost::DebugStream *dbg = ost::DebugStream::get(); \
                   if(dbg) { dbg->begin(__FILE__, __LINE__); \
                             (std::ostream&)(*dbg) << x << "\n"; \
                             dbg->end(); }}
