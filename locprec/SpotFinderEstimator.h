#ifndef LOCPREC_SPOTFINDERESTIMATOR_H
#define LOCPREC_SPOTFINDERESTIMATOR_H

#include <dStorm/EngineDebug.h>
#include <dStorm/Transmission.h>
#include <dStorm/TransmissionConfig.h>
#include <cc++/thread.h>
#include "NoiseSource.h"
#include <list>
#include <data-c++/Vector.h>

namespace locprec {
    class SpotFinderEstimator : public dStorm::Transmission
    {
      private:
        const std::list<Fluorophore*>* fluorophores;
        int msx, msy;

        ost::Mutex mutex;
        int numberOfSamples;
        int noCol;
        data_cpp::Vector<double[8]> data;
        std::ostream &out;

      public:
        SpotFinderEstimator (std::ostream &) throw ();
        ~SpotFinderEstimator() throw();

        AdditionalData announceStormSize(int, int, int totalImageCount) throw()
        { 
            ost::MutexLock lock(mutex);
            numberOfSamples = totalImageCount;
            data.resize(totalImageCount);
            return AdditionalData(CandidateTree | InputBuffer);
        }
        Result receiveLocalizations(const dStorm::EngineResult&) throw();
        void deleteAllResults() throw() {}
        void finish() throw();

    };

    class SpotFinderEstimatorConfig : public dStorm::TransmissionSource {
        simparm::ConfigEntryFile outputFile;

      public:
        SpotFinderEstimatorConfig() throw()
        : ConfigObject("SpotFinderEstimator", 
                       "Estimate spot finder performance"),
          outputFile("ToFile", "Write statistics to")
        { push_back( outputFile ); }
        SpotFinderEstimatorConfig( const SpotFinderEstimatorConfig& c )
            throw()
        : ConfigObject(c), dStorm::TransmissionSource(c),
          outputFile(c.outputFile) {}
        ~SpotFinderEstimatorConfig() throw() {}
        SpotFinderEstimatorConfig* clone() const 
            { return new SpotFinderEstimatorConfig(*this); }

        virtual std::auto_ptr<dStorm::Transmission> makeTransmission() throw() {
            if ( outputFile )
                return std::auto_ptr<dStorm::Transmission>( 
                  new SpotFinderEstimator( 
                    outputFile.getOutputStream() 
                  ) 
                );
            else
                return std::auto_ptr<dStorm::Transmission>( NULL );
        }
    };

}
#endif
