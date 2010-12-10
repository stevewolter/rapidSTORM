#ifndef DSTORM_CONFIG_H
#define DSTORM_CONFIG_H

#include <ConfigSet.hh>
#include <ConfigEntry.hh>
#include <math.h>

namespace dStorm {
   using namespace simparm;

   class _Config : public ConfigSet
   {
      protected:
        void registerEntries() throw();
      public:
        _Config() throw();

        /** The standard deviation for the exponential spot model. */
        ConfigEntryDouble sigma_x, sigma_y, sigma_xy;
        /** The uncertainty allowed in sigma estimation. */
        ConfigEntryDouble delta_sigma;
        /** Limit up to which X-Y correlation is considered negligible. */
        ConfigEntryDouble sigma_xy_negligible_limit;
        /** Start value for Marquardt lambda. */
        ConfigEntryDouble marquardtStartLambda;
        /** Maximum number of iteration steps in fitting process. */
        ConfigEntryUnsignedLong maximumIterationSteps;
        /** Maximum length of negligibly short iteration step. */
        ConfigEntryDouble negligibleStepLength;
        /** Number of successive negligibly short steps indicating fit
        *  success. */
        ConfigEntryUnsignedLong successiveNegligibleSteps;
        /** The proportionality factor for the smoothing & NMS mask size */
        ConfigEntryDouble maskSizeFactor;
        /** The proportionality factor for the fitting mask size (the fitting
        *  mask determines which pixel are fitted with the exponential PSF
        *  model). */
        ConfigEntryDouble fitSizeFactor;
        /** The method to use for spot detection. */
        ConfigEntryChoice spotFindingMethod;
        /** If this option is set, the sigma estimation code is disabled. */
        ConfigEntryBool fixSigma;

        ConfigEntryUnsignedLong motivation;
        ConfigEntryDouble amplitude_threshold, 
                        amplitude_limit;

        /* The mask size derived from the sigma value in X direction */
        unsigned long x_maskSize() const 
        { return (unsigned long)round( maskSizeFactor() * sigma_x() );}
        unsigned long y_maskSize() const 
        { return (unsigned long)round( maskSizeFactor() * sigma_y() );}
        unsigned long fitWidth() const 
        { return (unsigned long)round( fitSizeFactor() * sigma_x() );}
        unsigned long fitHeight() const 
        { return (unsigned long)round( fitSizeFactor() * sigma_y() );}

        ConfigEntryUnsignedLong reconstruction_mask_1, 
                                reconstruction_mask_2;

        /** Number of parallel computation threads to run. */
        ConfigEntryUnsignedLong pistonCount;

   };

   class Config : public _Config {
     private:
        class SigmaUserLevel : public ConfigNode::Callback {
            _Config& config;
          public:
            SigmaUserLevel(_Config &config) : config(config) { 
                receive_changes_from( config.fixSigma );
                (*this)( config.fixSigma, ValueChanged, NULL ); 
            }
            void operator()(ConfigNode& source, Cause cause, ConfigNode *arg) throw();
        };
        SigmaUserLevel user_level_watcher;
     public:
        Config() throw();
        Config(const Config& c) throw();
        virtual Config* clone() { return new Config(*this); }
   };

}

#endif
