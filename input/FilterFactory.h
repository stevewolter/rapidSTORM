#ifndef DSTORM_INPUT_FILTERFACTORY_H
#define DSTORM_INPUT_FILTERFACTORY_H

#include <memory>
#include <string>

#include "input/Source.h"
#include "simparm/NodeHandle.h"

namespace dStorm {
namespace input {

template <typename InputType, typename OutputType>
class FilterFactory {
  public:
    virtual FilterFactory* clone() const = 0;
    virtual std::string getName() const = 0;
    virtual void attach_ui(simparm::NodeHandle at) = 0;
    virtual std::unique_ptr<input::Source<OutputType>> make_source(
        std::unique_ptr<input::Source<InputType>> input) = 0;
    virtual boost::shared_ptr<const Traits<OutputType>> make_meta_info(
        MetaInfo& meta_info,
        boost::shared_ptr<const Traits<InputType>> input_meta_info) = 0;
};

}
}

#endif
