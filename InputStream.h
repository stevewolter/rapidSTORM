#ifndef DSTORM_INPUTSTREAM_H
#define DSTORM_INPUTSTREAM_H

#include <memory>
#include <simparm/text_stream/RootNode.h>
#include <dStorm/Config_decl.h>
#include "JobStarter.h"

namespace dStorm {

class MainThread;

class InputStream 
: public simparm::text_stream::RootNode
{
    std::auto_ptr< job::Config > orig_config, current_config;
    std::auto_ptr< JobStarter > starter;
    MainThread& main_thread;

    void reset_config();
    void processCommand( const std::string& cmd, std::istream& rest);
    bool print(const std::string& what);
  public:
    InputStream( MainThread& master );
    ~InputStream();
    void set_config( const job::Config& );
    using simparm::text_stream::RootNode::processCommand;
};

}

#endif
