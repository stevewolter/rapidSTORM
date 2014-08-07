#ifndef DSTORM_INPUT_FILTERFACTORYLINK_H
#define DSTORM_INPUT_FILTERFACTORYLINK_H

#include "helpers/make_unique.hpp"
#include "input/FilterFactory.h"
#include "input/Forwarder.hpp"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "input/Source.h"
#include "input/Link.h"

namespace dStorm {
namespace input {

template <typename InputType, typename OutputType = InputType>
class FilterFactoryLink
: public Forwarder
{
    typedef FilterFactory<InputType, OutputType> MyFilterFactory;

  public:
    FilterFactoryLink(std::unique_ptr<MyFilterFactory> filter)
        : filter_(std::move(filter)) {}
    FilterFactoryLink(const FilterFactoryLink& o)
        : Forwarder(o), filter_(o.filter_->clone()) {}

    FilterFactoryLink* clone() const OVERRIDE;
    void registerNamedEntries( simparm::NodeHandle node ) OVERRIDE;
    void traits_changed( TraitsRef, Link* ) OVERRIDE;

  private:
    BaseSource* makeSource() OVERRIDE;
    void republish_traits_locked() { 
        input::InputMutexGuard lock( input::global_mutex() );
        republish_traits();
    }
    void republish_traits() { 
        if ( Forwarder::upstream_traits().get() )
            traits_changed( Forwarder::upstream_traits(), NULL ); 
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
    Forwarder::registerNamedEntries( node );
    filter_->attach_ui( node, boost::bind(&FilterFactoryLink::republish_traits_locked, this) );
}

template <typename InputType, typename OutputType>
void FilterFactoryLink<InputType, OutputType>::traits_changed( TraitsRef upstream, Link* ) {
    if (!upstream || !upstream->provides<InputType>()) {
        this->update_current_meta_info(upstream);
        return;
    }

    boost::shared_ptr< MetaInfo > my_info( new MetaInfo(*upstream) );
    boost::shared_ptr<const Traits<InputType>> input_traits =
        upstream->traits<InputType>();
    if (input_traits) {
        my_info->set_traits(filter_->make_meta_info(input_traits));
    }
    this->update_current_meta_info(my_info);
}

template <typename InputType, typename OutputType>
BaseSource* FilterFactoryLink<InputType, OutputType>::makeSource() {
    std::unique_ptr<BaseSource> upstream = Forwarder::upstream_source();
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
std::unique_ptr<Link> CreateLink(
    std::unique_ptr<FilterFactory<InputType, OutputType>> filter) {
    return make_unique<FilterFactoryLink<InputType, OutputType>>(std::move(filter));
}

}
}

#endif
