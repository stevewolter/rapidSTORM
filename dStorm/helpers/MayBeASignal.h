#ifndef DSTORM_MayBeASignal_H
#define DSTORM_MayBeASignal_H

namespace dStorm {

class MayBeASignal 
{
    bool terminate, got_signal;
    int signum;
  public:
    MayBeASignal(bool terminate) 
        : terminate(terminate), got_signal(false) {}
    MayBeASignal(int signum) 
        : terminate(true), got_signal(true), signum(signum) {}
    
    bool should_terminate() { return terminate; }
    bool did_receive_signal() { return got_signal; }
    int signal_number() { return signum; }
};

}

#endif
