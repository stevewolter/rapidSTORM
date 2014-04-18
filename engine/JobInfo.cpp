#include "debug.h"
#include "engine/JobInfo.h"
#include "engine/InputTraits.h"
#include "engine/FitJudgerFactory.h"

namespace dStorm {
namespace engine {

JobInfo::JobInfo( boost::shared_ptr<const InputTraits> t, int fluorophore, const FitJudgerFactory& f )
: traits_store(t),
  traits(*traits_store),
  fluorophore(fluorophore)
{
    for (int i = 0; i < traits.plane_count(); ++i)
        judgers.push_back( f.make_fit_judger( traits.plane(i) ) );
}

JobInfo::JobInfo( const JobInfo& o ) 
: judgers(o.judgers),
  traits_store(o.traits_store),
  traits(*traits_store),
  fluorophore(o.fluorophore)
{
}

JobInfo::~JobInfo() {}

const FitJudger& JobInfo::get_judger( int plane ) const {
    return judgers[plane];
}

}
}
