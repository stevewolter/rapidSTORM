#ifndef LOCPREC_NOISEGENERATOR_H
#define LOCPREC_NOISEGENERATOR_H

#include <simparm/Set.h>
#include <simparm/Entry.h>
#include <simparm/Entry.h>
#include <simparm/FileEntry.h>
#include <memory>
#include <gsl/gsl_rng.h>

namespace input_simulation {
    class NoiseGeneratorConfig {
        simparm::Set name_object;
      public:
        simparm::Entry<double> ups_G, ups_mu, ups_sigma, ups_x0, ups_theta;
        simparm::FileEntry noiseFile;
        simparm::Entry<double> varianceScale;
        simparm::Entry<unsigned long> width, height;
        simparm::Entry<unsigned long> random_seed;

        NoiseGeneratorConfig();
        void attach_ui( simparm::NodeHandle );
    };
    
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
