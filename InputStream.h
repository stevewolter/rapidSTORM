#ifndef DSTORM_INPUTSTREAM_H
#define DSTORM_INPUTSTREAM_H

#include <memory>
#include <simparm/Node.hh>
#include <simparm/IO.hh>
#include <dStorm/Config_decl.h>
#include "JobStarter.h"

namespace dStorm {

class MainThread;

class InputStream 
: public simparm::IO
{
    std::auto_ptr< job::Config > orig_config, current_config;
    JobStarter starter;
    MainThread& main_thread;

    void reset_config();
    void processCommand( const std::string& cmd, std::istream& rest);
    void print(const std::string& what);
  public:
    InputStream( MainThread& master );
    ~InputStream();
    void set_config( const job::Config& );
    void processCommand( std::istream& stream ) { simparm::IO::processCommand(stream); }
};

}

#endif
