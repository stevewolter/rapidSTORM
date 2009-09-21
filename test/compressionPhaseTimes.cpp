#include "../src/Garage.h"
#include <dStorm/engine/EngineDebug.h>

using namespace dStorm;

int main(int argc, const char *argv[]) {
    Garage garage(argc, argv);
    ost::Thread::joinDetached();
    std::cout << "Driver spent " << smooth_time / 10000 << " cs in smoothing, " << search_time / 10000 << " cs in "
            "searching and " << fit_time / 10000 << " cs in fitting." 
         << "\n";
}
