#ifndef DSTORM_LOCALIZATIONFILE_RECORD_DECL_H
#define DSTORM_LOCALIZATIONFILE_RECORD_DECL_H

#include <boost/variant/variant.hpp>
#include "Localization_decl.h"
#include "input/Traits.h"

namespace dStorm {
namespace localization {

struct EmptyLine;
typedef 
boost::variant< dStorm::Localization, EmptyLine > Record;

}

namespace input {
template <>
struct Traits<localization::Record> ;
}

}

#endif
