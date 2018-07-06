#ifndef DSTORM_INPUTS_FILEMETHOD_H
#define DSTORM_INPUTS_FILEMETHOD_H

#include <memory>

#include "input/Forwarder.h"
#include "simparm/FileEntry.h"
#include "simparm/Group.h"

namespace dStorm {
namespace inputs {

class FileMethod
: public input::Forwarder {
  public:
    FileMethod();
    void add_choice(std::unique_ptr<input::Link> link);
    void traits_changed( TraitsRef, input::Link* ) OVERRIDE;

    FileMethod* clone() const OVERRIDE { return new FileMethod(*this); }
    void registerNamedEntries( simparm::NodeHandle node ) OVERRIDE { 
        simparm::NodeHandle r = name_object.attach_ui( node );
        input_file.attach_ui(r);
        Forwarder::registerNamedEntries(r);

        listening = input_file.value.notify_on_value_change( 
            boost::bind( &FileMethod::republish_traits, this ) );
    }
    std::string name() const OVERRIDE { return name_object.getName(); }

    input::BaseSource* makeSource() OVERRIDE { return Forwarder::makeSource(); }

  private:
    void republish_traits();

    simparm::Group name_object;
    simparm::FileEntry input_file;
    simparm::BaseAttribute::ConnectionStore listening;
};

}
}

#endif
