#ifndef DSTORM_ENGINE_CONFIGURED_SOURCE_IMPL_H
#define DSTORM_ENGINE_CONFIGURED_SOURCE_IMPL_H

#include "FilterBuilder.h"

namespace dStorm {
namespace output {

template <class Type, class OutputType>
FilterBuilder<Type,OutputType>::FilterBuilder()
: name_object( Type::get_name(), Type::get_description() )
{ 
    name_object.userLevel = Type::get_user_level();
}

template <class Type, class OutputType>
FilterBuilder<Type,OutputType>::
FilterBuilder(const FilterBuilder<Type,OutputType>& o)
: FilterSource(o),
  config(o.config),
  name_object(o.name_object)
{
    if ( o.getFactory() != NULL )
        this->set_output_factory( *o.getFactory() );
}

template <class Type, class OutputType>
FilterBuilder<Type,OutputType>* FilterBuilder<Type,OutputType>::clone() const
{ return new FilterBuilder(*this); }

template <class Type, class OutputType>
void 
FilterBuilder<Type,OutputType>::set_source_capabilities
    ( Capabilities cap ) 
{
    name_object.viewable = 
        config.determine_output_capabilities( cap );
    FilterSource::set_source_capabilities( cap );
}

template <class Type, class OutputType>
std::auto_ptr<Output> FilterBuilder<Type,OutputType>::make_output() 
{
    return std::auto_ptr<Output>( new OutputType( 
        config, FilterSource::make_output() ) );
}

}
}

#endif
