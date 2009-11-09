#include "Localization.h"

namespace dStorm {

std::ostream&
operator<<(std::ostream &o, const Localization& loc)
{
    return
        o << loc.x() << " " << loc.y() << " " << loc.getImageNumber() 
            << " " << loc.getStrength() << " " << loc.parabolicity() 
            << "\n";
}

}
