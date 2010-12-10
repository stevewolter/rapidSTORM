#ifndef LOCPREC_SPOTMETER_H
#define LOCPREC_SPOTMETER_H
#include <map>
#include <iostream>
#include "Counter.h"
#include <dStorm/TransmissionConfig.h>

namespace locprec {
    class SpotMeter : public dStorm::Transmission
    {
      private:
        typedef std::map<dStorm::Image::Pixel,Counter> CountMap;
        CountMap countMap;
        int binSize;
        ost::Mutex mutex;
        std::ostream &to;

      public:
        SpotMeter (std::ostream &to, int binSize) throw () 
            : ConfigObject("SpotMeter", ""), binSize(binSize), to(to) {}
        ~SpotMeter () throw () {}

        AdditionalData announceStormSize(int , int , int) throw() { return NoData;}
        Result receiveLocalizations(const dStorm::EngineResult &er) 
                throw() 
        {
            ost::MutexLock lock(mutex);
            for (int f = 0; f < er.number; f++) {
                double realAmp = er.first[f].getStrength();
                countMap[ int(realAmp / binSize) * binSize ]++;
            }
            return KeepRunning;
        }
        void deleteAllResults() throw() 
            { ost::MutexLock lock(mutex); countMap.clear(); }
        void finish() throw() {
            ost::MutexLock lock(mutex);
            for (CountMap::iterator i = countMap.begin(); 
                                    i != countMap.end(); i++)
                to << i->first << " " << i->second << "\n";
        }
    };

    class SpotMeterConfig : public dStorm::TransmissionSource {
      private:
        simparm::ConfigEntryFile targetFile;
        simparm::ConfigEntryUnsignedLong modulus;

      public:
        SpotMeterConfig() throw()
        : ConfigObject("SpotMeter", "Histogram localization amplitudes"),
          targetFile("OutputFile", "Histogram target file"),
          modulus("BinSize", "Size for histogram bins") 
        {
            modulus.setMin(1);
            push_back(targetFile);
            push_back(modulus);
        }
        SpotMeterConfig(const SpotMeterConfig& o) throw() 
        : ConfigObject(o), TransmissionSource(o),
          targetFile(o.targetFile), modulus(o.modulus) 
        {
            push_back(targetFile);
            push_back(modulus);
        }
        ~SpotMeterConfig() throw() {}

        SpotMeterConfig* clone() const throw() { return new SpotMeterConfig(*this); }
        std::auto_ptr<dStorm::Transmission> makeTransmission() throw() {
            return std::auto_ptr<dStorm::Transmission> ( 
                new SpotMeter(targetFile.getOutputStream(), modulus()) );
        }
    };

}

#endif
