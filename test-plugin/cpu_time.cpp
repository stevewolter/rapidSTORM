#include "cpu_time.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "debug.h"
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#include <string.h>
#if HAVE_ERRNO_H
#include <errno.h>
#endif

boost::optional<double> get_cpu_time() {
#if HAVE_GETRUSAGE
    struct rusage usage;
    int rv = getrusage(RUSAGE_SELF, &usage);
    if ( rv == -1 ) {
        std::cout << "Getting resource usage failed: " 
                    << strerror(errno) << std::endl;
        return boost::optional<double>();
    } else {
        return usage.ru_utime.tv_sec + usage.ru_utime.tv_usec * 1E-6;
    }
#else
    return boost::optional<double>();
#endif
}
