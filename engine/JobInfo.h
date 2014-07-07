#ifndef DSTORM_ENGINE_JOBINFO_H
#define DSTORM_ENGINE_JOBINFO_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#include "engine/Image_decl.h"
#include "helpers/virtual_clone_allocator.hpp"

namespace dStorm {
namespace engine {

class FitJudgerFactory;
class FitJudger;

struct JobInfo {
private:
    boost::ptr_vector< FitJudger, VirtualCloneAllocator<FitJudger> > judgers;
    boost::shared_ptr<const InputTraits> traits_store;

public:
    const InputTraits& traits;
    int fluorophore;
public:
    JobInfo( boost::shared_ptr<const InputTraits>, int fluorophore, const FitJudgerFactory& );
    JobInfo( const JobInfo& );
    ~JobInfo();

    const FitJudger& get_judger( int plane ) const;
};


}
}

#endif
