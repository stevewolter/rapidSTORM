#ifndef DSTORM_INPUT_FILTERFACTORY_H
#define DSTORM_INPUT_FILTERFACTORY_H

#include <memory>
#include <string>

#include "input/Source.h"
#include "simparm/NodeHandle.h"

namespace dStorm {
namespace input {

template <typename InputType, typename OutputType = InputType>
class FilterFactory {
  public:
    virtual ~FilterFactory() {}
    virtual FilterFactory* clone() const = 0;
    virtual void attach_ui(simparm::NodeHandle at,
                           std::function<void()> traits_change_callback) = 0;
    virtual std::unique_ptr<input::Source<OutputType>> make_source(
        std::unique_ptr<input::Source<InputType>> input) = 0;
    virtual boost::shared_ptr<const Traits<OutputType>> make_meta_info(
        MetaInfo& meta_info,
        boost::shared_ptr<const Traits<InputType>> input_meta_info) = 0;

    // This method needs to be overridden if the FilterFactory is used as an
    // option in a dStorm::input::Choice.
    virtual std::string getName() const {
        throw std::logic_error("Not implemented"); }
};

}
}

#endif
