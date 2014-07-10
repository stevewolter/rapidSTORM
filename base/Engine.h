#ifndef DSTORM_ENGINE_H
#define DSTORM_ENGINE_H

#include "input/Traits.h"
#include <memory>

namespace dStorm {

class EngineBlock {
  public:
    virtual ~EngineBlock() {}
};

class Engine {
  public:
    virtual ~Engine() {}
    virtual void restart() = 0;
    virtual void stop() = 0;
    virtual void repeat_results() = 0;
    virtual bool can_repeat_results() = 0;
    virtual void change_input_traits( std::auto_ptr< input::BaseTraits > ) = 0;
    virtual std::auto_ptr<EngineBlock> block() = 0;
    virtual std::auto_ptr<EngineBlock> block_termination() = 0;
};

}

#endif
