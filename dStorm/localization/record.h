#ifndef DSTORM_LOCALIZATION_FILE_RECORD_H
#define DSTORM_LOCALIZATION_FILE_RECORD_H

#include "dStorm/localization/record_decl.h"
#include "dStorm/Localization.h"
#include "dStorm/localization/Traits.h"
#include "dStorm/units/frame_count.h"
#include <iostream>

namespace dStorm {
namespace localization {

struct EmptyLine {
    frame_index number;
    EmptyLine(frame_index n) : number(n) {}
};

std::ostream& operator<<(std::ostream&, const EmptyLine&);

typedef 
boost::variant< dStorm::Localization, EmptyLine > Record;

}

namespace input {
template <>
struct Traits<localization::Record> 
: public Traits<Localization> {
    Traits* clone() const { return new Traits(*this); }
};
}

}

#endif
