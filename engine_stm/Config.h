#ifndef DSTORM_STM_ENGINE_CONFIG_H
#define DSTORM_STM_ENGINE_CONFIG_H

#include <simparm/Object.hh>

namespace dStorm {
namespace engine_stm {

struct Config : public simparm::Object
{
    bool throw_errors;
    Config();
};

}
}

#endif
