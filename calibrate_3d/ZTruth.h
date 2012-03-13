#ifndef DSTORM_CALIBRATE3D_ZTRUTH_H
#define DSTORM_CALIBRATE3D_ZTRUTH_H

#include "expression/Parser.h"
#include <dStorm/localization/Traits.h>
#include <dStorm/output/LocalizedImage.h>

namespace dStorm {
namespace expression { namespace source { class LValue; } }
namespace calibrate_3d {

class TrueZ;

class ZTruth {
    expression::Parser parser;
    TrueZ *new_z_variable;
    std::auto_ptr< expression::source::LValue > filter, new_z_expression;
    input::Traits<Localization> localization_traits;
public:
    ZTruth( const std::string& filter_expression, const std::string& true_z_expression );
    void set_meta_info( const input::Traits<Localization>& );

    output::LocalizedImage::iterator calibrate( output::LocalizedImage& );
    quantity<si::length> true_z( const Localization& ) const;
};

}
}

#endif
