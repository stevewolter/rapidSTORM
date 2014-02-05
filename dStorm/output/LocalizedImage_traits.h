#ifndef DSTORM_OUTPUT_LOCALIZED_IMAGE_TRAITS_H
#define DSTORM_OUTPUT_LOCALIZED_IMAGE_TRAITS_H

#include "dStorm/engine/InputTraits.h"
#include "dStorm/input/Traits.h"
#include "dStorm/output/LocalizedImage_decl.h"
#include "dStorm/localization/Traits.h"
#include "dStorm/engine/Input_decl.h"
#include <boost/smart_ptr/shared_ptr.hpp>
#include <vector>
#include "core/Engine.h"

namespace boost { struct recursive_mutex; }

namespace dStorm {
class Engine;

namespace input {

template <>
struct Traits<output::LocalizedImage>
: public Traits<Localization>
{
    /** If the data source knows which carburettor supplies the
        * images, this pointer is set to it, and NULL otherwise. */
    engine::Input* carburettor;
    Engine *engine;

    Traits( const std::string& name, const std::string& description );
    Traits( 
        const Traits<Localization>& traits,
        const std::string& name = "", const std::string& description = "",
        dStorm::engine::Input* carburettor = NULL,
        Engine *engine = NULL);
    ~Traits();
    Traits<output::LocalizedImage>* clone() const { return new Traits(*this); }

    typedef boost::shared_ptr<Traits> Ptr;
    typedef boost::shared_ptr<const Traits> ConstPtr;

    bool source_image_is_set, smoothed_image_is_set, candidate_tree_is_set;
    boost::shared_ptr< const engine::InputTraits > input_image_traits;
    std::string name, description;
};

}
}

#endif
