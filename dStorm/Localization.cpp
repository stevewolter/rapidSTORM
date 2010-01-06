#include "Localization.h"
#include <boost/units/io.hpp>

namespace dStorm {

std::ostream&
operator<<(std::ostream &o, const Localization& loc)
{
    return
        o << loc.position().transpose() << " " << loc.getImageNumber() 
            << " " << loc.strength() 
            << " " << loc.two_kernel_improvement()
            << "\n";
}

}
