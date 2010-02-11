#ifndef DSTORM_CONFIG_H
#define DSTORM_CONFIG_H

#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <simparm/ChoiceEntry.hh>
#include <math.h>

namespace dStorm {
namespace engine {
   using namespace simparm;

   class SpotFinderFactory;

   /** Config entry collection class for the dSTORM engine. */
   class _Config : public Set
   {
      protected:
        void registerNamedEntries();
      public:
        _Config();
        ~_Config();

        /** The standard deviation for the exponential spot model. */
        DoubleEntry sigma_x, sigma_y, sigma_xy;
        /** The uncertainty allowed in sigma estimation. */
        DoubleEntry delta_sigma;
        /** Limit up to which X-Y correlation is considered negligible. */
        DoubleEntry sigma_xy_negligible_limit;
        /** Start value for Marquardt lambda. */
        DoubleEntry marquardtStartLambda;
        /** Maximum number of iteration steps in fitting process. */
        UnsignedLongEntry maximumIterationSteps;
        /** Maximum length of negligibly short iteration step. */
        DoubleEntry negligibleStepLength;
        /** Number of successive negligibly short steps indicating fit
        *  success. */
        UnsignedLongEntry successiveNegligibleSteps;
        /** The proportionality factor for the smoothing & NMS mask size */
        DoubleEntry maskSizeFactor;
        /** The proportionality factor for the fitting mask size (the fitting
        *  mask determines which pixel are fitted with the exponential PSF
        *  model). */
        DoubleEntry fitSizeFactor;
        /** The method to use for spot detection. */
        simparm::NodeChoiceEntry<SpotFinderFactory> spotFindingMethod;
        /** If this option is set, the sigma estimation code is disabled. */
        BoolEntry fixSigma;
        /** If this option is set, the sigma_x/y/xy parameters are fitted
         *  to the spots. */
        BoolEntry freeSigmaFitting;
        /** Threshold for the spot fitter for the degree of asymmetry
         *  in the residues from which on it will suscept multi-spots. */
        DoubleEntry asymmetry_threshold;

        /** Continue fitting until this number of bad fits occured. */
        UnsignedLongEntry motivation;
        /** Amplitude threshold to judge localizations by. */
        DoubleEntry amplitude_threshold;

        /** The smoothing/NMS mask size derived from the sigma value 
         *  in X direction */
        unsigned long x_maskSize() const 
        { return (unsigned long)round( maskSizeFactor() * sigma_x() );}
        /** The smoothing/NMS mask size derived from the sigma value
         *  in Y direction */
        unsigned long y_maskSize() const 
        { return (unsigned long)round( maskSizeFactor() * sigma_y() );}
        /** The fit window size derived from the sigma value in X direction */
        unsigned long fitWidth() const 
        { return (unsigned long)round( fitSizeFactor() * sigma_x() );}
        /** The fit window size derived from the sigma value in Y direction */
        unsigned long fitHeight() const 
        { return (unsigned long)round( fitSizeFactor() * sigma_y() );}

        /** Number of parallel computation threads to run. */
        UnsignedLongEntry pistonCount;
   };

   class Config : public _Config {
     private:
        /** Class changes the user level of sigma entries based on
         *  the setting of fixSigma. */
        class SigmaUserLevel : public Node::Callback {
            _Config& config;
            void adjust();
          public:
            SigmaUserLevel(_Config &config);
            void operator()(const Event&);
        };
        SigmaUserLevel user_level_watcher;
     public:
        Config();
        Config(const Config& c);
        virtual Config* clone() const { return new Config(*this); }
   };

}
}

#endif
