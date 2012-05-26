#ifndef DSTORM_INPUTSTREAM_H
#define DSTORM_INPUTSTREAM_H

#include <memory>
#include <dStorm/Config_decl.h>
#include "JobStarter.h"
#include <simparm/text_stream/Node.h>
#include <dStorm/stack_realign.h>

namespace dStorm {

class MainThread;

class InputStream 
: public simparm::text_stream::Node
{
    class Backend;

    std::auto_ptr< job::Config > orig_config, current_config;
    std::auto_ptr< JobStarter > starter;
    MainThread& main_thread;
    Backend* const root_backend;

    void reset_config();
    void processCommand( const std::string& cmd, std::istream& rest);
    InputStream( MainThread& master, const job::Config& );
    bool received_quit_command();

  public:
    ~InputStream();
    DSTORM_REALIGN_STACK void processCommands( );
    static boost::shared_ptr<InputStream> create( MainThread&, const job::Config& );
};

}

#endif
