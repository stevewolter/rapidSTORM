#ifndef DSTORM_CALIBRATE_3D_PARAMETER_LINEARIZER_H
#define DSTORM_CALIBRATE_3D_PARAMETER_LINEARIZER_H

#include <dStorm/engine/InputTraits.h>
#include <gsl/gsl_vector.h>
#include "Config.h"

namespace dStorm {
namespace calibrate_3d {

class ParameterLinearizer {
    class Pimpl;
    std::auto_ptr<Pimpl> pimpl;
public:
    ParameterLinearizer( const Config_& );
    ~ParameterLinearizer();
    void set_traits( const engine::InputTraits& );

    int parameter_count() const;
    void linearize( const engine::InputTraits&, gsl_vector* );
    void delinearize( const gsl_vector*, engine::InputTraits& );
};

}
}

#endif
