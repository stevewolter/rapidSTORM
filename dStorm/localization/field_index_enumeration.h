#ifndef DSTORM_LOCALIZATION_field_index_enumeration_H
#define DSTORM_LOCALIZATION_field_index_enumeration_H
#include "../Localization.h"
#include <boost/mpl/range_c.hpp>

namespace dStorm {
namespace localization {

typedef boost::mpl::range_c<int,0,dStorm::Localization::Fields::Count> FieldIndices;

}
}

#endif
