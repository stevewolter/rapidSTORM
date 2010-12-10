#include "SpotFitter.h"
#include "GaussFitter.h"

using namespace dStorm;
using namespace std;

auto_ptr<SpotFitter> SpotFitter::factory(const Config &c) {
   return auto_ptr<SpotFitter>(new GaussFitter(c));
}
