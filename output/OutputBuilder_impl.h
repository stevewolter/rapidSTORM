#ifndef DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H
#define DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H

#include "output/OutputBuilder.h"

namespace dStorm {
namespace output {

template <typename Config, typename Output>
OutputBuilder<Config, Output>::OutputBuilder()
: name_object( Config::get_name(), Config::get_description() ),
  choice_object( Config::get_name(), Config::get_description() )
{ 
    choice_object.set_user_level( Config::get_user_level() );
}

}
}

#endif
