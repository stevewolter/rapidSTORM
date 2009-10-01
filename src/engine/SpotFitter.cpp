#include "engine/SpotFitter.h"
#include "engine/GaussFitter.h"

using namespace std;

namespace dStorm {

auto_ptr<SpotFitter> SpotFitter::factory(const Config &c) {
    return auto_ptr<SpotFitter>( select_gauss_fitter( c ) );
}

}
