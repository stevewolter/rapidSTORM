#ifndef DSTORM_INPUT_FILTERFACTORYLINK_H
#define DSTORM_INPUT_FILTERFACTORYLINK_H

#include "helpers/make_unique.hpp"
#include "input/FilterFactory.h"
#include "input/Forwarder.h"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "input/Source.h"
#include "input/Link.h"

namespace dStorm {
namespace input {

template <typename InputType, typename OutputType = InputType>
class FilterFactoryLink
: public Forwarder<InputType, OutputType>
{
    typedef FilterFactory<InputType, OutputType> MyFilterFactory;
    typedef Forwarder<InputType, OutputType> MyForwarder;

  public:
    FilterFactoryLink(std::unique_ptr<MyFilterFactory> filter,
                      std::unique_ptr<Link<InputType>> upstream)
        : MyForwarder(std::move(upstream)), filter_(std::move(filter)) {}
    FilterFactoryLink(const FilterFactoryLink& o)
        : MyForwarder(o), filter_(o.filter_->clone()) {}

    FilterFactoryLink* clone() const OVERRIDE;
    void registerNamedEntries( simparm::NodeHandle node ) OVERRIDE;
    void traits_changed( boost::shared_ptr<const MetaInfo>, Link<InputType>* ) OVERRIDE;

  private:
    Source<OutputType>* makeSource() OVERRIDE;
    void republish_traits_locked() { 
        input::InputMutexGuard lock( input::global_mutex() );
        republish_traits();
    }
    void republish_traits() { 
        if ( MyForwarder::upstream_traits().get() )
            traits_changed( MyForwarder::upstream_traits(), NULL ); 
    }

    std::unique_ptr<MyFilterFactory> filter_;
};

template <typename InputType, typename OutputType>
FilterFactoryLink<InputType, OutputType>*
FilterFactoryLink<InputType, OutputType>::clone() const {
    return new FilterFactoryLink(*this);
}

template <typename InputType, typename OutputType>
void FilterFactoryLink<InputType, OutputType>::registerNamedEntries( simparm::NodeHandle node ) {
    MyForwarder::registerNamedEntries( node );
    filter_->attach_ui( node, boost::bind(&FilterFactoryLink::republish_traits_locked, this) );
}

template <typename InputType, typename OutputType>
void FilterFactoryLink<InputType, OutputType>::traits_changed(
    boost::shared_ptr<const MetaInfo> upstream, Link<InputType>* ) {
    if (!upstream || !upstream->template provides<InputType>()) {
        this->update_current_meta_info(upstream);
        return;
    }

    boost::shared_ptr< MetaInfo > my_info( new MetaInfo(*upstream) );
    boost::shared_ptr<const Traits<InputType>> input_traits =
        upstream->template traits<InputType>();
    if (input_traits) {
        my_info->set_traits(filter_->make_meta_info(input_traits));
    }
    this->update_current_meta_info(my_info);
}

template <typename InputType, typename OutputType>
Source<OutputType>* FilterFactoryLink<InputType, OutputType>::makeSource() {
    std::unique_ptr<BaseSource> upstream = MyForwarder::upstream_source();
    std::unique_ptr<Source<InputType>> typed_upstream(
        dynamic_cast<Source<InputType>*>(upstream.get()));
    if (!typed_upstream) {
        throw std::logic_error("Cannot process input "
            "of the current type");
    } else {
        upstream.release();
    }

    return filter_->make_source(std::move(typed_upstream)).release();
}

template <typename InputType, typename OutputType>
std::unique_ptr<Link<OutputType>> CreateLink(
    std::unique_ptr<FilterFactory<InputType, OutputType>> filter,
    std::unique_ptr<Link<InputType>> upstream) {
    return make_unique<FilterFactoryLink<InputType, OutputType>>(
        std::move(filter), std::move(upstream));
}

}
}

#endif
