#ifndef LOCPREC_SPOTMETER_H
#define LOCPREC_SPOTMETER_H

#include <dStorm/output/OutputSource.h>
#include <memory>

namespace dStorm { 
namespace outputs {

std::auto_ptr< dStorm::output::OutputSource > make_spot_meter_source();

}
}

#endif
