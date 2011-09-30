#ifndef LOCPREC_NOISEMETER_H
#define LOCPREC_NOISEMETER_H

#include <dStorm/output/Output.h>
#include <dStorm/image/constructors.h>
#include <dStorm/engine/Image.h>
#include <dStorm/output/OutputBuilder.h>
#include <map>
#include <iostream>
#include "Counter.h"
#include <dStorm/Image.h>
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>

namespace locprec {
    class NoiseMeter : public dStorm::output::OutputObject
    {
      public:
        typedef std::map<dStorm::engine::StormPixel,Counter> CountMap;
      private:
        CountMap countMap;
        typedef dStorm::Image<bool,2> Image;
        Image noisePixels;
        int adNoise;
        std::ostream &output;
        dStorm::input::Traits<dStorm::Localization> traits;

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
            bool can_work_with( dStorm::output::Capabilities )
                {return true;}
        };

      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::output::OutputBuilder<NoiseMeter> Source;

        NoiseMeter (Config& config) 
        : OutputObject("NoiseMeter", "Noise meter"),
          adNoise(config.adCorrection()), 
          output(config.outputFile.get_output_stream()) {}
        NoiseMeter *clone() const { return new NoiseMeter(*this); }

        AdditionalData announceStormSize(const Announcement &a)
 
        {
            Image::Size sz;
            for (int i = 0; i < 2; ++i) {
                sz[i] = *a.position().range()[i].second * *a.position().resolution()[i];
            }
            noisePixels = Image(sz);
            traits = a;
            return AdditionalData().set_source_image();
        }
        Result receiveLocalizations(const EngineResult &er) 
 
        {
            const dStorm::engine::Image& image = er.source;
            noisePixels.fill(true);
            Eigen::Matrix2i centers;
            for (EngineResult::const_iterator l = er.begin(); l != er.end(); ++l) {
                for (int i = 0; i < 2; ++i) {
                    boost::units::quantity<boost::units::camera::length> 
                        q = l->position()[i] * *traits.position().resolution()[i];
                    centers[i] = round(q / boost::units::camera::pixel);
                }

                for (int dx = -10; dx <= 10; dx++)
                  for (int dy = -10; dy <= 10; dy++) 
                {
                    int x = centers[0]+dx;
                    int y = centers[1]+dy;
                    if (x >= 0 && x < int(image.width_in_pixels()) && 
                        y >= 0 && y < int(image.height_in_pixels()))
                    noisePixels(x,y) = false;
                }
            }
            for (int z = 0; z < image.depth_in_pixels(); z++)
              for (int y = 0; y < image.height_in_pixels(); y++)
                for (int x = 0; x < image.width_in_pixels(); x++)
                    if ( noisePixels(x,y) == true )
                        countMap[ (image(x,y,z) / adNoise) ]++;

            return KeepRunning;
        }
        void propagate_signal(ProgressSignal s) {
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
