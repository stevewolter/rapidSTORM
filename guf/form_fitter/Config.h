#ifndef DSTORM_FORMFITTER_CONFIG_H
#define DSTORM_FORMFITTER_CONFIG_H

#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/output/Capabilities.h>
#include <boost/array.hpp>
#include <boost/optional/optional.hpp>
#include <dStorm/polynomial_3d.h>
#include <dStorm/Direction.h>

namespace dStorm {
namespace form_fitter {

class FormCalibratorConfig {
    boost::array< boost::optional< simparm::BoolEntry >, polynomial_3d::Order > z_terms;
    simparm::Entry< bool > circular_psf_, astigmatism_, universal_best_sigma_;
public:
    FormCalibratorConfig();
    void registerNamedEntries( simparm::Node& at );

    bool fit_z_term( Direction, polynomial_3d::Term term ) const
        { return (*z_terms[ polynomial_3d::offset(term) ])(); }
    bool symmetric() const { return circular_psf_(); }
    bool astigmatism() const { return astigmatism_(); }
    bool universal_best_sigma() const { return universal_best_sigma_(); }
};

struct Config : public simparm::Object, public FormCalibratorConfig {
        simparm::BoolEntry auto_disable, mle;
        simparm::Entry<unsigned long> number_of_spots, max_per_image;
        simparm::BoolEntry visual_selection, 
                           laempi_fit, disjoint_amplitudes;
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
