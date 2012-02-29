#ifndef LOCPREC_NOISEGENERATOR_H
#define LOCPREC_NOISEGENERATOR_H

#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <simparm/Structure.hh>
#include <memory>
#include <gsl/gsl_rng.h>

namespace input_simulation {
    class _NoiseGeneratorConfig : public simparm::Set {
      protected:
        void registerNamedEntries();
      public:
        simparm::Entry<double> ups_G, ups_mu, ups_sigma, ups_x0, ups_theta;
        simparm::FileEntry noiseFile;
        simparm::Entry<double> varianceScale;
        simparm::Entry<unsigned long> width, height;
        simparm::Entry<unsigned long> random_seed;

        _NoiseGeneratorConfig();
    };
    
    typedef simparm::Structure<_NoiseGeneratorConfig>
        NoiseGeneratorConfig;

    template <typename T>
    class NoiseGenerator {
      private:
        class _Config;

      public:
        virtual ~NoiseGenerator() {}

        static std::auto_ptr<NoiseGenerator> factory
            (const NoiseGeneratorConfig &config, gsl_rng *rng);

        virtual void pixelNoise(T *vec, size_t size) = 0;
    };
}
#endif
