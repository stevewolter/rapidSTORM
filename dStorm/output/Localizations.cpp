#include "debug.h"

#include "dStorm/output/Localizations.h"
#include <cassert>
#include <string.h>
#include <fstream>

#include <dStorm/output/Output.h>
#include <dStorm/engine/Image.h>
#include <vector>
#include <map>

using namespace std;

namespace dStorm {
namespace output {

Localizations::image_wise_iterator
Localizations::insert( const LocalizedImage& i )
{
    std::list<OneImage>::iterator insert = localizations.end(), first = localizations.begin(), test;
    while ( insert != first ) {
        --insert;
        if ( insert->forImage < i.forImage ) {
            ++insert;
            break;
        }
    }

    return localizations.insert( insert, i );
}

}
}
