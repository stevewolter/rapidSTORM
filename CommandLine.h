#ifndef DSTORM_COMMANDLINE_H
#define DSTORM_COMMANDLINE_H

#include <dStorm/JobMaster.h>

namespace dStorm {

class CommandLine
: public JobMaster
{
    class Pimpl;
    std::auto_ptr<Pimpl> pimpl;

  public:
    CommandLine(int argc, char *argv[]);
    ~CommandLine();

    void run();
    std::auto_ptr<JobHandle> register_node( Job& );
};

}

#endif
