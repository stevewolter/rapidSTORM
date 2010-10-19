#include "VerboseInputFilter.h"
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <iostream>

void VerboseInputFilter::modify_traits( TraitsRef ref )
{
    std::cerr << "Traits are passing" << std::endl;
}

void VerboseInputFilter::modify_context( ContextRef ref )
{
    std::cerr << "Context is passing" << std::endl;
}

dStorm::input::Source<dStorm::engine::Image>*
VerboseInputFilter::makeSource( std::auto_ptr< dStorm::input::Source<dStorm::engine::Image> > ptr )
{
    std::cerr << "Image source is passing" << std::endl;
    return ptr.release();
}

dStorm::input::Source<dStorm::Localization>*
VerboseInputFilter::makeSource( std::auto_ptr< dStorm::input::Source<dStorm::Localization> > ptr )
{
    std::cerr << "Localization source is passing" << std::endl;
    return ptr.release();
}
