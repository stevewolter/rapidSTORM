#ifndef DSTORM_COMMANDLINE_H
#define DSTORM_COMMANDLINE_H

#include <dStorm/helpers/thread.h>

namespace dStorm {

class CommandLine
: public dStorm::Thread
{
    class Pimpl;
    std::auto_ptr<Pimpl> pimpl;

  public:
    CommandLine(int argc, char *argv[]);
    ~CommandLine();

    void run();
    void abnormal_termination(std::string reason);
};

}

#endif
