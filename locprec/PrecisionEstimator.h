#ifndef LOCPREC_PRECISIONESTIMATOR_H
#define LOCPREC_PRECISIONESTIMATOR_H

#include <memory>
#include <dStorm/Transmission.h>
#include <dStorm/TransmissionConfig.h>

namespace locprec {
    class PrecisionEstimator : public dStorm::Transmission {
      private:
        std::ostream *printSingle, *printMulti, *printSpots;
        double nm, res_enh;

        struct FitSigmas { double x, y, xy; int n; double a; };
        FitSigmas fitWithGauss(const dStorm::Localization* f, int n)
            throw();
        
      public:
        PrecisionEstimator (std::ostream *printSingle, std::ostream *printMulti,
                            std::ostream *printSpots, double pixelSize,
                            double fit_res_enh) throw () 
        : ConfigObject("PrecisionEstimator", ""),
          printSingle(printSingle), 
          printMulti(printMulti),
          printSpots(printSpots),
          nm(pixelSize),
          res_enh(fit_res_enh)
        {}
        ~PrecisionEstimator() throw() {}

        AdditionalData announceStormSize(int, int, int) throw(std::exception)
            { return LocalizationSources; }
        Result receiveLocalizations(const dStorm::EngineResult&) throw();
        void deleteAllResults() throw();
        void finish() throw();
    };

    class PrecisionEstimatorConfig : public dStorm::TransmissionSource {
      protected:
        simparm::ConfigEntryFile printSingle, printMulti, printSpots;
        simparm::ConfigEntryDouble pixelSizeInNm, resEnh;

        void registerNamedEntries() throw() {
            push_back( printSingle );
            push_back( printMulti );
            push_back( printSpots );
            push_back( pixelSizeInNm );
        }
    
      public:
        PrecisionEstimatorConfig() throw()
        : ConfigObject("PrecisionEstimatorConfig", "Output precision"
                       " estimates"),
          printSingle("SinglePrecisionsFile", "Save per-spot precision "
                      "estimates to:"),
          printMulti("CompletePrecisionsFile", "Save precision average "
                     "to"),
          printSpots("SpotFile", "Save localization coordinates "
                     "seperated by spot to"),
          pixelSizeInNm("PixelSize", "Estimated pixel size in nm for "
                        "FWHM size calculation", 81),
          resEnh("FitBinSize", "Bin size for precision Gauss fit", 0.1)
            { registerNamedEntries(); }
        PrecisionEstimatorConfig( const PrecisionEstimatorConfig& c ) throw()
        : ConfigObject(c), dStorm::TransmissionSource(c),
          printSingle(c.printSingle), printMulti(c.printMulti), 
          printSpots(c.printSpots), pixelSizeInNm(c.pixelSizeInNm)
            { registerNamedEntries(); }
        ~PrecisionEstimatorConfig() throw() {}
        PrecisionEstimatorConfig* clone() const 
            { return new PrecisionEstimatorConfig(); }

        std::auto_ptr<dStorm::Transmission> makeTransmission() throw() {
            if ( printSingle || printMulti || printSpots )
                return std::auto_ptr<dStorm::Transmission> ( 
                  new PrecisionEstimator(
                    (printSingle) ? &printSingle.getOutputStream() : NULL,
                    (printMulti) ? &printMulti.getOutputStream() : NULL,
                    (printSpots) ? &printSpots.getOutputStream() : NULL,
                    pixelSizeInNm() , (1/resEnh())
                  ));
            else
                return std::auto_ptr<dStorm::Transmission>( NULL );
        }
    };
}
#endif
