#include "debug.h"
#include "input/Link.h"

#ifdef VERBOSE
#include "engine/Image.h"
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"
#include "localization/Traits.h"
#include "Localization.h"
#include "input/MetaInfo.h"
#endif

#include "engine/InputTraits.h"
#include "input/MetaInfo.h"
#include "input/Source.h"

namespace dStorm {
namespace input {

template <typename Type>
void Link<Type>::update_current_meta_info( TraitsRef new_traits ) {
    assert( new_traits.get() );
    if ( new_traits->provides< dStorm::engine::ImageStack >() )
        assert( new_traits->traits< dStorm::engine::ImageStack >()->plane_count() > 0 );
    meta_info = new_traits;
    DEBUG("Publishing traits for " << this);
    meta_info_signal( meta_info );
}

template <typename Type>
Link<Type>::Connection
Link<Type>::notify( const TraitsSignal::slot_type& whom ) { 
    DEBUG(this << " adding notification");
    return Connection( new boost::signals2::scoped_connection(
            meta_info_signal.connect( whom ) ) ); 
}

}
}

