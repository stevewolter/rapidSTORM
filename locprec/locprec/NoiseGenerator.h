#ifndef LOCPREC_NOISEGENERATOR_H
#define LOCPREC_NOISEGENERATOR_H

#include <ConfigSet.hh>
#include <ConfigEntry.hh>
#include <memory>
#include <gsl/gsl_rng.h>

namespace locprec {
    class _NoiseGeneratorConfig : public simparm::ConfigSet {
      public:
        simparm::ConfigEntryDouble ups_G, ups_mu, ups_sigma, ups_x0, ups_theta;
        simparm::ConfigEntryFile noiseFile;
        simparm::ConfigEntryDouble varianceScale;
        simparm::ConfigEntryUnsignedLong width, height;
        simparm::ConfigEntryUnsignedLong random_seed;

        _NoiseGeneratorConfig() throw();
    };
    
    class NoiseGeneratorConfig : public _NoiseGeneratorConfig {
      private:
        void registerNamedEntries() throw();
      public:
        NoiseGeneratorConfig() throw();
        NoiseGeneratorConfig(const NoiseGeneratorConfig&) throw();

        NoiseGeneratorConfig* clone() const { return new NoiseGeneratorConfig(*this);}
    };

    template <typename T>
    class NoiseGenerator {
      public:
        virtual ~NoiseGenerator() throw() {}

        static std::auto_ptr<NoiseGenerator> factory
            (const NoiseGeneratorConfig &config, gsl_rng *rng) throw ();

        virtual void pixelNoise(T *vec, size_t size) throw() = 0;
    };
}
#endif
