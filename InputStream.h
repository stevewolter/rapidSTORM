#ifndef DSTORM_INPUTSTREAM_H
#define DSTORM_INPUTSTREAM_H

#include <dStorm/helpers/thread.h>
#include <simparm/Node.hh>
#include <dStorm/Config_decl.h>
#include <dStorm/JobMaster.h>

namespace dStorm {

class InputStream 
: public JobMaster
{
    class Pimpl;
    friend class Pimpl;
    std::auto_ptr<Pimpl> pimpl;

  public:
    InputStream(const Config&,
                std::istream&, std::ostream&);
    InputStream(std::istream*, std::ostream*);
    ~InputStream();
    
    void add_modules( Config& config );

    void start();

    void register_node( Job& );
    void erase_node( Job& );
};

}

#endif
