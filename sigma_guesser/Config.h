#ifndef DSTORM_SIGMAGUESSER_CONFIG_H
#define DSTORM_SIGMAGUESSER_CONFIG_H

#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/UnitEntries/PixelEntry.h>

namespace dStorm {
namespace sigma_guesser {

class Config : public simparm::Object {
        /** If this option is set, the sigma estimation code is disabled. */
        simparm::BoolEntry fitSigma;

        /** The uncertainty allowed in sigma estimation. */
        simparm::DoubleEntry delta_sigma;
        /** Maximum number of pixels used in PSF estimation. */
        IntPixelAreaEntry maximum_estimation_size;

        Config();
        void registerNamedEntries();
};

}
}

#endif
