#ifndef DSTORM_INPUT_FILEMETHOD_H
#define DSTORM_INPUT_FILEMETHOD_H

#include "Forwarder.h"
#include <simparm/FileEntry.hh>
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/Set.hh>

class TestState;

namespace dStorm {
namespace input {

class FileMethod
: public simparm::Set,
  public Forwarder,
  protected simparm::Listener
{
    simparm::FileEntry input_file;

  protected:
    void operator()( const simparm::Event& );

  public:
    FileMethod();
    FileMethod(const FileMethod&);
    ~FileMethod();

    virtual void traits_changed( TraitsRef, Link* );

    FileMethod* clone() const { return new FileMethod(*this); }
    void registerNamedEntries( simparm::Node& node ) { 
        this->push_back( input_file );
        Forwarder::registerNamedEntries(*this);
        node.push_back( *this );
    }
    std::string name() const { return getName(); }
    std::string description() const { return getDesc(); }

    BaseSource* makeSource() { return Forwarder::makeSource(); }

    static void unit_test( TestState& ); 
};

}
}

#endif
