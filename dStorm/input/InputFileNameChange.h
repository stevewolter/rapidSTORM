#ifndef DSTORM_INPUT_INPUTFILENAME_CHANGED_H

#include <string>
#include <boost/signals2/slot.hpp>
#include <boost/signals2/signal.hpp>

namespace dStorm {
namespace input {

typedef typename boost::signals2::signal< void (const std::string&) > 
    InputFileNameChange;

}
}

#endif
