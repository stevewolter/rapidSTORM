#ifndef DSTORM_INPUT_BASENAME_CHANGED_H
#define DSTORM_INPUT_BASENAME_CHANGED_H

#include <string>
#include <boost/signals2/slot.hpp>
#include <boost/signals2/signal.hpp>
#include "output/Basename.h"

namespace dStorm {
namespace signals {

struct BasenameChange 
: public boost::signals2::signal< void (const output::Basename&) > 
{
};

}
}

#endif
