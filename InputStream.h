#ifndef DSTORM_INPUTSTREAM_H
#define DSTORM_INPUTSTREAM_H

#include <memory>
#include <dStorm/Config_decl.h>
#include "shell/JobFactory.h"
#include "shell/JobMetaFactory.h"
#include <simparm/text_stream/Node.h>
#include <dStorm/stack_realign.h>

namespace dStorm {

class InputStream 
: public simparm::text_stream::Node
{
    class Backend;

    shell::JobMetaFactory rapidstorm, alignment_fitter, replay_job;
    boost::ptr_vector< shell::JobFactory > configs;
    Backend* const root_backend;

    void reset_config();
    void create_alignment_fitter();
    void create_replay_job();
    void processCommand( const std::string& cmd, std::istream& rest);
    InputStream( const JobConfig&, bool wxWidgets );
    bool received_quit_command();

  public:
    ~InputStream();
    DSTORM_REALIGN_STACK void processCommands( );
    static boost::shared_ptr<InputStream> create( const JobConfig&, bool wxWidgets );
};

}

#endif
