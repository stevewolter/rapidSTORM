#ifndef DSTORM_INPUT_FILEMETHOD_H
#define DSTORM_INPUT_FILEMETHOD_H

#include "chain/Choice.h"
#include "chain/Forwarder.h"
#include "chain/FileContext_decl.h"
#include <simparm/FileEntry.hh>
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/Set.hh>

namespace dStorm {
namespace input {

class FileMethod
: public simparm::Set,
  public chain::Forwarder,
  protected simparm::Listener
{
  public:
    boost::shared_ptr<const chain::FileContext> context;
    simparm::FileEntry input_file;
    chain::Choice children;

  protected:
    void operator()( const simparm::Event& );

  public:
    FileMethod();
    FileMethod(const FileMethod&);
    ~FileMethod();

    virtual AtEnd traits_changed( TraitsRef, Link* );
    virtual AtEnd context_changed( ContextRef, Link* );

    FileMethod* clone() const { return new FileMethod(*this); }
    simparm::Node& getNode() { return *this; }

    BaseSource* makeSource() { return Forwarder::makeSource(); }

};

}
}

#endif
