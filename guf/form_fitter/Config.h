#ifndef DSTORM_FORMFITTER_CONFIG_H
#define DSTORM_FORMFITTER_CONFIG_H

#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/output/Capabilities.h>
#include <boost/array.hpp>
#include <boost/optional/optional.hpp>
#include <dStorm/polynomial_3d.h>

namespace dStorm {
namespace form_fitter {

struct Config : public simparm::Object {
        simparm::BoolEntry auto_disable, mle;
        simparm::Entry<unsigned long> number_of_spots, max_per_image;
        simparm::BoolEntry visual_selection, circular_psf, 
                           laempi_fit, disjoint_amplitudes;
        boost::array< boost::optional< simparm::BoolEntry >, polynomial_3d::Order > z_terms;
        simparm::FileEntry z_calibration;

        Config();
        void registerNamedEntries();

        bool can_work_with(output::Capabilities cap)  {
        	return cap.test( output::Capabilities::SourceImage );
        }
};

}
}

#endif
