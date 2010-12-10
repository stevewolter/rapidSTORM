#include <dStorm/Image.h>

using namespace std;
using namespace dStormEngine;

#include <gsl/gsl_rng.h>

auto_ptr<Image> makeNoiseImage(int w, int h) {
   auto_ptr<Image> i(new Image(w, h));
   return i;
}
