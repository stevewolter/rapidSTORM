#include <dStormUtils/Driver.h>
#include <dStorm/GaussFitter.h>

using namespace dStorm;
using namespace dStormUtils;

static int state = 0, endStates[4] = {0,0,0,0};

void probe(int iteration, const GaussFitter::GaussFit& fit) {
   if (iteration == 1) {
      endStates[state-1]++;
      state = 0;
   }

   if (state == 0 && fit.isGood())
      state = 1;
   else if (state == 0 && ! fit.isGood())
      state = 2;
   else if (state == 1 && ! fit.isGood()) {
      state = 3;
   } else if (state == 2 && fit.isGood()) {
      state = 4;
   }
}

int main(int argc, char *argv[]) {
#ifdef DSTORMENGINE_FIT_ITERATION_HOOK
   GaussFitter::fit_iteration_hook = probe;
#endif
   Driver::Config c; 
   c.readConfig(argc, argv);
   Driver cs(c);
   cs.drive();

   for (int i = 0; i < 4; i++)
      cout << endStates[i] << " ";
   cout << endl;
   return 0;
}
