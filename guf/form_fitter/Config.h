#ifndef DSTORM_FORMFITTER_CONFIG_H
#define DSTORM_FORMFITTER_CONFIG_H

#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <simparm/Entry.hh>
#include <dStorm/output/Capabilities.h>

namespace dStorm {
namespace form_fitter {

struct Config : public simparm::Object {
        simparm::BoolEntry auto_disable, mle;
        simparm::Entry<unsigned long> number_of_spots;
        simparm::BoolEntry visual_selection, circular_psf, 
                           laempi_fit, disjoint_amplitudes;

        Config();
        void registerNamedEntries();

        bool can_work_with(output::Capabilities cap)  {
        	return cap.test( output::Capabilities::SourceImage );
        }
};

}
}

#endif
