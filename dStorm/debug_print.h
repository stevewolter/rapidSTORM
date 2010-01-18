#ifdef VERBOSE
#include <dStorm/helpers/thread.h>
#include <iostream>
#define DEBUG(x) { ost::DebugStream *dbg = ost::DebugStream::get(); \
                   if(dbg) { dbg->begin(__FILE__, __LINE__); \
                             (std::ostream&)(*dbg) << x << "\n"; \
                             dbg->end(); }}
#else
#define DEBUG(x)
#endif
