#ifndef DSTORM_INPUT_BASENAMEWATCHER_H
#define DSTORM_INPUT_BASENAMEWATCHER_H

#include <simparm/FileEntry.hh>
#include <simparm/Attribute.hh>
#include <cstring>
#include "Config.h"

namespace dStorm {
namespace input {

/** The BasenameWatcher is a property change callback for a filename
 *  that keeps a basename attribute current. Whenever the watched
 *  file name entry changes, its list of extensions is scanned and
 *  the first extension that matches the end of the filename is
 *  selected. The selected extension is removed from the filename,
 *  and the result is assigned to the basename attribute. */
class BasenameWatcher 
: public simparm::Node::Callback
{
    MethodChoice& choice;
    simparm::Attribute<std::string> &output;
    simparm::Attribute<std::string> *current;

    void attach();

 public:
    BasenameWatcher(
        MethodChoice& choice,
        simparm::Attribute<std::string>& output
    );

    void operator()( const simparm::Event& );
};

}
}

#endif
