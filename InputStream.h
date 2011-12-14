#ifndef DSTORM_INPUTSTREAM_H
#define DSTORM_INPUTSTREAM_H

#include <dStorm/helpers/thread.h>
#include <simparm/Node.hh>
#include <dStorm/Config_decl.h>
#include <dStorm/JobMaster.h>

namespace dStorm {

class GrandConfig;

class InputStream 
: public JobMaster
{
    class Pimpl;
    friend class Pimpl;
    std::auto_ptr<Pimpl> pimpl;

  public:
    InputStream(const GrandConfig&,
                std::istream&, std::ostream&);
    InputStream(std::istream*, std::ostream*);
    ~InputStream();
    
    void add_modules( Config& config );

    void start();

    std::auto_ptr<JobHandle> register_node( Job& );
};

}

#endif
