/** \file debug.h 
 *  This file contains the debug output switch common to all modules */
/** Debug levels:
 *    1 prints the AndorCamera control state transitions and the initialization/cleanup phase
 *    2 prints information about the STORM progress
 *    3 prints entries and exits from Andor functions
 **/
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

/** \def STATUS(x) The status macro prints control state transitions
 *                 and basic initialization/cleanup stuff. It should
 *                 not be used to print out information during engine
 *                 runs. */
#if DEBUG_LEVEL >= 1
#define PRINT(x) { ost::DebugStream *dbg = ost::DebugStream::get(); \
                   if(dbg) { dbg->begin(__FILE__, __LINE__); \
                             (std::ostream&)(*dbg) << x << "\n"; \
                             dbg->end(); }}
#define STATUS(x) PRINT(x)
#define DEBUG(x) PRINT(x)
#else
#define STATUS(x)
#define DEBUG(x)
#endif

/** \def PROGRESS(x) The progress macro prints information about the
 *                   general progress of engine runs. It should output
 *                   a small amount of lines (<10) per engine run. */
#if DEBUG_LEVEL >= 2
#define PROGRESS(x) PRINT(x)
#else
#define PROGRESS(x)
#endif

/** \def LOCKING(x) The locking macro prints very verbose information
 *                  about almost all steps and many mutex locks to
 *                  help debugging mutex problems. */
#if DEBUG_LEVEL >= 3
#define LOCKING(x) PRINT(x)
#else
#define LOCKING(x)
#endif

#if DEBUG_LEVEL == 0
#define GSL_RANGE_CHECK_OFF
#endif

#ifdef WITH_DMALLOC
# include <stdlib.h>
# include <string.h>
# include <dmalloc.h>
#endif
