#ifndef DSTORM_INPUTSTREAM_H
#define DSTORM_INPUTSTREAM_H

#include <dStorm/helpers/thread.h>
#include <simparm/Node.hh>
#include "engine/CarConfig_decl.h"
#include <dStorm/JobMaster.h>
#include "engine/CarConfig.h"

namespace dStorm {

class InputStream 
: public Thread, public JobMaster
{
    class Pimpl;
    friend class Pimpl;
    std::auto_ptr<Pimpl> pimpl;

  public:
    InputStream(const engine::CarConfig&,
                std::istream&, std::ostream&);
    InputStream(std::istream*, std::ostream*);
    ~InputStream();
    
    void add_modules( engine::CarConfig& config );

    void run();
    void abnormal_termination(std::string reason);

    void register_node( Job& );
    void erase_node( Job& );
};

}

#endif
