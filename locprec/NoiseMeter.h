#ifndef LOCPREC_NOISEMETER_H
#define LOCPREC_NOISEMETER_H

#include <dStorm/Transmission.h>
#include <dStorm/Image.h>
#include <CImgBuffer/Image.h>
#include <map>
#include <iostream>
#include "Counter.h"

namespace locprec {
    class NoiseMeter : public dStorm::Transmission
    {
      public:
        typedef std::map<dStorm::Image::Pixel,Counter> CountMap;
      private:
        CountMap countMap;
        cimg_library::CImg<bool> noisePixels;
        int adNoise;
        std::ostream &output;
        ost::Mutex mutex;

        void fit() throw();

      public:
        NoiseMeter (int adNoise, std::ostream &output) throw () 
        : ConfigObject("NoiseMeter", "Noise meter"),
          adNoise(adNoise), output(output) {}
        ~NoiseMeter () throw () {}
        AdditionalData announceStormSize(int w, int h, int) throw() {
            ost::MutexLock lock(mutex);
            noisePixels.resize(w,h,1,1);
            return SourceImage;
        }
        Result receiveLocalizations(const dStorm::EngineResult &er) 
                throw() 
        {
            const cimg_library::CImg<dStorm::StormPixel>& image = *er.source;
            ost::MutexLock lock(mutex);
            noisePixels.fill(true);
            for (int f = 0; f < er.number; f++)
                for (int dx = -10; dx <= 10; dx++)
                  for (int dy = -10; dy <= 10; dy++) 
                {
                    int x = er.first[f].getXCenter()+dx;
                    int y = er.first[f].getYCenter()+dy;
                    if (x >= 0 && x < int(image.width) && 
                        y >= 0 && y < int(image.height))
                    noisePixels(x,y) = false;
                }
            for (unsigned int y = 0; y < image.height; y++) {
                for (unsigned int x = 0; x < image.width; x++) {
                 if ( x < 64 && y < 64 && noisePixels(x,y) == true )
                     countMap[ (image(x,y) / adNoise) ]++;
                }
            }

            return KeepRunning;
        }
        void deleteAllResults() throw() 
            { ost::MutexLock lock(mutex); countMap.clear(); }
        void finish() throw() {
            for (CountMap::iterator i = countMap.begin(); i != countMap.end(); i++)
                output << i->first * adNoise + adNoise / 2 << " "
                       << i->second << std::endl;
        }
    };

    class NoiseMeterConfig : public dStorm::TransmissionSource {
        simparm::ConfigEntryUnsignedLong adCorrection;
        simparm::ConfigEntryFile outputFile;
            
      public:
        NoiseMeterConfig() throw() :
            simparm::ConfigObject("NoiseMeterConfig", ""),
            adCorrection("ADCorrection", "A/D error correction factor", 8),
            outputFile("ToFile", "Output noise statistics to")
        { push_back( adCorrection ); push_back( outputFile ); }
        NoiseMeterConfig(const NoiseMeterConfig &c) throw() :
            simparm::ConfigObject(c),
            dStorm::TransmissionSource(c),
            adCorrection(c.adCorrection),
            outputFile(c.outputFile)
        { push_back( adCorrection ); push_back( outputFile ); }
        ~NoiseMeterConfig() throw() {}
        NoiseMeterConfig *clone() const { return new NoiseMeterConfig(*this); }

        virtual std::auto_ptr<dStorm::Transmission> makeTransmission() throw() {
            if ( outputFile )
                return std::auto_ptr<dStorm::Transmission>( 
                    new NoiseMeter( adCorrection(), outputFile.getOutputStream() ));
            else 
                return std::auto_ptr<dStorm::Transmission>(NULL);
        }
    };
}
#endif
