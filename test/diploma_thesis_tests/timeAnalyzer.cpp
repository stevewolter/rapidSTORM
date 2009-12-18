#include <dStorm/Engine.h>
#include <dStorm/Fits.h>
#include <dStorm/Commander.h>
#include <SIFStorm/SIFLoader.h>
#include <time.h>

using namespace std;
using namespace dStorm;
using namespace SIFStorm;

#ifdef DSTORMENGINE_AFTER_FIT_HOOK
extern int candidates;
#endif
extern unsigned long fitOperations;

int main(int argc, char *argv[]) {
   Commander::Config config;
   config.readConfig(argc, argv);
   Commander commander(config);
   clock_t time = clock();
   commander.trigger();
   clock_t time_after = clock();

   SIFVector input(config.inputFile().c_str());
   cout 
         << (time_after - time) << "\t" 
         << input.imageCount() * input.imageSize().w * 
                                 input.imageSize().h << "\t"
#ifdef DSTORMENGINE_AFTER_FIT_HOOK
         << candidates << "\t"
         << Engine::nGoodSpots << "\t"
         << Engine::nBadSpots << "\t"
#endif
#ifdef DSTORMENGINE_FIT_ITERATION_HOOK
         << fitOperations << "\t"
#endif
         << endl;
}
