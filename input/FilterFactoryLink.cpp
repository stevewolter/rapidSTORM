#include "input/FilterFactoryLink.h"

#include "engine/InputTraits.h"
#include "input/Forwarder.h"
#include "input/MetaInfo.h"
#include "input/Source.h"
#include "input/FilterFactory.h"
#include "input/InputMutex.h"
#include "output/LocalizedImage_traits.h"

namespace dStorm {
namespace input {

template <typename InputType, typename OutputType>
class FilterFactoryLink
: public Forwarder
{
    typedef FilterFactory<InputType, OutputType> MyFilterFactory;

  public:
    FilterFactoryLink(std::unique_ptr<MyFilterFactory> filter)
        : filter_(std::move(filter)) {}

    FilterFactoryLink* clone() const OVERRIDE;
    std::string name() const OVERRIDE { return filter_->getName(); }
    void registerNamedEntries( simparm::NodeHandle node ) OVERRIDE;
    void traits_changed( TraitsRef, Link* ) OVERRIDE;

  private:
    BaseSource* makeSource() OVERRIDE;

    std::unique_ptr<MyFilterFactory> filter_;
};

template <typename InputType, typename OutputType>
FilterFactoryLink<InputType, OutputType>*
FilterFactoryLink<InputType, OutputType>::clone() const {
    std::unique_ptr<FilterFactory<InputType, OutputType>> factory(
        filter_->clone());
    return new FilterFactoryLink<InputType, OutputType>(std::move(factory));
}

template <typename InputType, typename OutputType>
void FilterFactoryLink<InputType, OutputType>::registerNamedEntries( simparm::NodeHandle node ) {
    Forwarder::registerNamedEntries( node );
    filter_->attach_ui( node );
}

template <typename InputType, typename OutputType>
void FilterFactoryLink<InputType, OutputType>::traits_changed( TraitsRef upstream, Link* ) {
    if (!upstream || !upstream->provides<InputType>()) {
        this->update_current_meta_info(TraitsRef());
        return;
    }

    boost::shared_ptr< MetaInfo > my_info( new MetaInfo(*upstream) );
    boost::shared_ptr<const Traits<InputType>> input_traits =
        upstream->traits<InputType>();
    boost::shared_ptr<Traits<OutputType>> output_traits =
        filter_->make_meta_info(*my_info, input_traits);
    my_info->set_traits(output_traits);
    this->update_current_meta_info(my_info);
}

template <typename InputType, typename OutputType>
BaseSource* FilterFactoryLink<InputType, OutputType>::makeSource() {
    std::auto_ptr<BaseSource> upstream = Forwarder::upstream_source();
    std::unique_ptr<Source<InputType>> typed_upstream(
        dynamic_cast<Source<InputType>*>(upstream.get()));
    if (!typed_upstream) {
        throw std::runtime_error(name() + " cannot process input "
            "of the current type");
    } else {
        upstream.release();
    }

    return filter_->make_source(std::move(typed_upstream)).release();
}

std::auto_ptr<Link> CreateLink(
    std::unique_ptr<FilterFactory<engine::ImageStack, engine::ImageStack>> filter) {
    return std::auto_ptr<Link>(
        new FilterFactoryLink<engine::ImageStack, engine::ImageStack>(std::move(filter)));
}

std::auto_ptr<Link> CreateLink(
    std::unique_ptr<FilterFactory<engine::ImageStack, output::LocalizedImage>> filter) {
    return std::auto_ptr<Link>(
        new FilterFactoryLink<engine::ImageStack, output::LocalizedImage>(std::move(filter)));
}

std::auto_ptr<Link> CreateLink(
    std::unique_ptr<FilterFactory<output::LocalizedImage, output::LocalizedImage>> filter) {
    return std::auto_ptr<Link>(
        new FilterFactoryLink<output::LocalizedImage, output::LocalizedImage>(std::move(filter)));
}

}
}
