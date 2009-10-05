#include "Localizations.h"
#include <cassert>
#include <string.h>
#include <fstream>
#include <foreach.h>

#include <dStorm/Output.h>
#include <dStorm/Image.h>
#include <data-c++/Vector.h>
#include <data-c++/VectorList.h>
#include <cc++/thread.h>
#include <vector>
#include <map>

using namespace std;
using namespace data_cpp;

namespace dStorm {

Localizations::Localizations(int w, int h, int n) : w(w), h(h), n(n) {
}

Localizations::Localizations(const Localizations& l)
: VectorList<Localization>(l), w(l.w), h(l.h), n(l.n) {}

Localizations::~Localizations() {
    STATUS("Destructed fit list");
}

}
