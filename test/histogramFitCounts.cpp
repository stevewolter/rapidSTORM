#include "dStorm/GaussFitter.h"
#include <dStormUtils/Driver.h>
#include <map>
#include <foreach.h>

using namespace dStorm;
using namespace dStormUtils;
using namespace std;

typedef map<int,int> Numbers;
Numbers numbers;

void fit_iteration_hook( int ni, const GaussFitter::GaussFit & )
{
   Numbers::iterator number = numbers.find(ni);
   if (number == numbers.end())
      number = numbers.insert(pair<int,int>(ni, 0)).first;

   number->second++;
}


int main(int argc, char *argv[]) {
   Driver::Config c;
   c.readConfig(argc, argv);

#ifdef DSTORMENGINE_FIT_ITERATION_HOOK
   GaussFitter::gaussfit_hook = fit_iteration_hook;
#endif
#ifndef DSTORMENGINE_FIT_ITERATION_HOOK
   return 1;
#endif

   Driver(c).drive();

   foreach_const( i, Numbers, numbers )
      cout << i->first << " " << i->second << endl;
   return 0;
}
