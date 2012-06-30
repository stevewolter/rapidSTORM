#ifdef VERBOSE
#include <dStorm/helpers/thread.h>
#include <iostream>
#include <boost/units/io.hpp>

#define PRINT(x) { ost::DebugStream *dbg = ost::DebugStream::get(); \
                   if(dbg) { dbg->begin(__FILE__, __LINE__); \
                             (std::ostream&)(*dbg) << x << "\n"; \
                             dbg->end(); }}
#define DEBUG(x) PRINT(x)
#else
#define DEBUG(x) 
#endif
