#ifndef DSTORM_INPUTS_FILEMETHOD_H
#define DSTORM_INPUTS_FILEMETHOD_H

#include <memory>

#include "input/Choice.h"
#include "input/Forwarder.h"
#include "input/fwd.h"
#include "simparm/FileEntry.h"
#include "simparm/Group.h"

class TestState;

namespace dStorm {
namespace inputs {

class FileMethod
: public input::Choice
{
    simparm::Group name_object;
    simparm::FileEntry input_file;
    simparm::BaseAttribute::ConnectionStore listening;
    bool replacing_file_name;
    TraitsRef traits_after_file_name_replacement;

    FileMethod* clone() const OVERRIDE { return new FileMethod(*this); }
    void registerNamedEntries( simparm::NodeHandle node ) { 
        simparm::NodeHandle r = name_object.attach_ui( node );
        input_file.attach_ui(r);
        input::Choice::registerNamedEntries(r);

        listening = input_file.value.notify_on_value_change( 
            boost::bind( &FileMethod::republish_traits, this ) );
    }

    void republish_traits();
    void update_current_meta_info( TraitsRef new_traits ) OVERRIDE;

  public:
    FileMethod();
    static void unit_test(TestState&);
};

}
}

#endif
