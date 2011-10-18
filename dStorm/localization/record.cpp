#include "record.h"

namespace dStorm {
namespace localization {

std::ostream& operator<<(std::ostream& o, const localization::EmptyLine& e)
{
    return (o << e.number);
}


}
}
