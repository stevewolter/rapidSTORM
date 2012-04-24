#include "ScheduleIndexFinder.hpp"
#include "EvaluationTags.h"

namespace dStorm {
namespace guf {

ScheduleIndexFinder::ScheduleIndexFinder( bool disjoint, bool use_floats, const Optics& optics )
: do_disjoint( disjoint && 
    optics.supports_guaranteed_row_width() ),
  use_floats( use_floats ),
  optics(optics)
{
}

int ScheduleIndexFinder::get_evaluation_tag_index( const guf::Spot& position ) const
{
    return get_evaluation_tag_index( evaluation_tags(), position);
}

}
}
