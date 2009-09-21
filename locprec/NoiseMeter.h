#ifndef LOCPREC_NOISEMETER_H
#define LOCPREC_NOISEMETER_H

#include <dStorm/Output.h>
#include <dStorm/engine/Image.h>
#include <dStorm/OutputBuilder.h>
#include <map>
#include <iostream>
#include "Counter.h"
#include <CImg.h>
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>

namespace locprec {
    class NoiseMeter : public simparm::Object, public dStorm::Output
    {
      public:
        typedef std::map<dStorm::StormPixel,Counter> CountMap;
      private:
        CountMap countMap;
        cimg_library::CImg<bool> noisePixels;
        int adNoise;
        std::ostream &output;
        ost::Mutex mutex;

        void fit();

        class _Config : public simparm::Object {
          protected:
            void registerNamedEntries() {
                push_back( adCorrection ); 
                push_back( outputFile ); 
            }
                
          public:
            simparm::UnsignedLongEntry adCorrection;
            simparm::FileEntry outputFile;

            _Config();
        };

      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::OutputBuilder<NoiseMeter> Source;

        NoiseMeter (Config& config) 
        : Object("NoiseMeter", "Noise meter"),
          adNoise(config.adCorrection()), 
          output(config.outputFile.get_output_stream()) {}
        NoiseMeter *clone() const { return new NoiseMeter(*this); }

        AdditionalData announceStormSize(const Announcement &a)
 
        {
            ost::MutexLock lock(mutex);
            noisePixels.resize(a.width,a.height,a.depth,a.colors);
            return SourceImage;
        }
        Result receiveLocalizations(const EngineResult &er) 
 
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
        void propagate_signal(ProgressSignal s) {
            ost::MutexLock lock(mutex);
            if ( s == Engine_is_restarted )
                countMap.clear();
            else if ( s == Engine_run_succeeded )
                for (CountMap::iterator i = countMap.begin(); i!=countMap.end(); i++)
                    output << i->first * adNoise + adNoise / 2 << " "
                        << i->second << "\n";
        }
    };
}
#endif
