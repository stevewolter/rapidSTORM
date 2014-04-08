#include "shell/ReplayJob.h"
#include "job/Config.h"
#include "ModuleLoader.h"

namespace dStorm {
namespace shell {

std::auto_ptr< JobConfig > make_replay_job() {
    return std::auto_ptr< JobConfig >( new job::Config( true ) );
}

}
}
