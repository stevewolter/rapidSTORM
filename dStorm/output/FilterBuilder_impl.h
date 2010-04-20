#ifndef DSTORM_ENGINE_CONFIGURED_SOURCE_IMPL_H
#define DSTORM_ENGINE_CONFIGURED_SOURCE_IMPL_H

#include "FilterBuilder.h"

namespace dStorm {
namespace output {

template <class Type>
FilterBuilder<Type>::FilterBuilder(bool failSilently)
: FilterSource( 
        static_cast<typename Type::Config&>(*this) ),
  failSilently(failSilently) 
{ 
    this->push_back( this->help_file ); 
}

template <class Type>
FilterBuilder<Type>::
FilterBuilder(const FilterBuilder<Type>& o)
: Type::Config(o),
  FilterSource(
        static_cast<typename Type::Config&>(*this), o ),
  failSilently(failSilently) 
{
    if ( o.getFactory() != NULL )
        this->set_output_factory( *o.getFactory() );
    this->push_back( this->help_file ); 
}

template <class Type>
FilterBuilder<Type>* FilterBuilder<Type>::clone() const
{ return new FilterBuilder(*this); }

template <class Type>
void 
FilterBuilder<Type>::set_source_capabilities
    ( Capabilities cap ) 
{
    this->viewable = 
        this->Type::Config::determine_output_capabilities( cap );
    this->FilterSource::set_source_capabilities( cap );
}

template <class Type>
std::auto_ptr<Output> FilterBuilder<Type>::make_output() 
{
    typename Type::Config& config =
        static_cast<typename Type::Config&>(*this);
    try {
        return std::auto_ptr<Output>( new Type( 
            config, FilterSource::make_output() ) );
    } catch ( Source_Is_Transparent& transparent ) {
        return transparent.output;
    } catch (...) {
        if ( !failSilently ) 
            throw;
        else
            return std::auto_ptr<Output>( NULL );
    }
}

}
}

#endif
