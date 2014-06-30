#include "localization/Traits.h"

#include "unit_interval.h"
#include "engine/InputTraits.h"

namespace dStorm {
namespace input {

Traits<dStorm::Localization>::Traits() 
    : in_sequence(false) {}

Traits<dStorm::Localization>*
Traits<dStorm::Localization>::clone() const
    { return new Traits(*this); }

std::string Traits<dStorm::Localization>::desc() const
 { return "localization"; }

}
}
