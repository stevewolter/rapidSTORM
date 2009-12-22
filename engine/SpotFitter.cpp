#include "SpotFitter.h"
#include "GaussFitter.h"

using namespace std;

namespace dStorm {
namespace engine {

auto_ptr<SpotFitter> SpotFitter::factory(const Config &c) {
    return auto_ptr<SpotFitter>( select_gauss_fitter( c ) );
}

}
}
