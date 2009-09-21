/** \file EngineDebug.h
 *  This file contains switches to enable debug-facilities for dStorm */

    /** If this macro is defined, the dStorm engine will export two
    *  variables of type clock_t: search_time and fit_time .
    *  These variables give the time (accurate to SYS_CLK) the system
    *  spent in the two compression phases. 
    */
//#define DSTORM_MEASURE_TIMES

#include <time.h>

#ifdef DSTORM_MEASURE_TIMES
#    ifndef DSTORM_ENGINE_CPP
         extern clock_t smooth_time, search_time, fit_time;
#    endif
#    define IF_DSTORM_MEASURE_TIMES(x) x
#else
#    define IF_DSTORM_MEASURE_TIMES(x)
#endif
