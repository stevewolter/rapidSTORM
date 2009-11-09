#include "Localizations.h"
#include <cassert>
#include <string.h>
#include <fstream>
#include <foreach.h>

#include <dStorm/output/Output.h>
#include <dStorm/engine/Image.h>
#include <dStorm/data-c++/Vector.h>
#include <dStorm/data-c++/VectorList.h>
#include <dStorm/helpers/thread.h>
#include <vector>
#include <map>

using namespace std;
using namespace data_cpp;

namespace dStorm {
namespace output {

Localizations::~Localizations() {
    STATUS("Destructed fit list");
}

}
}
