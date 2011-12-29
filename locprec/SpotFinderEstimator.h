#ifndef LOCPREC_SPOTFINDERESTIMATOR_H
#define LOCPREC_SPOTFINDERESTIMATOR_H

#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/helpers/thread.h>
#include "NoiseSource.h"
#include <list>
#include <simparm/FileEntry.hh>

namespace locprec {
    class SpotFinderEstimator 
    : public dStorm::output::OutputObject
    {
      private:
        const boost::ptr_list<Fluorophore>* fluorophores;
        dStorm::engine::Input *carburettor;
        boost::units::quantity< camera::length, int > msx, msy;

        ost::Mutex mutex;
        int numberOfSamples;
        int noCol;
        std::vector<double[8]> data;
        std::ostream &out;

        dStorm::input::Traits<dStorm::Localization> traits;

        void finish();

        class _Config : public simparm::Object {
          protected:
            void registerNamedEntries() {push_back( outputFile );}
          public:
            simparm::FileEntry outputFile;

            _Config();
            bool can_work_with( dStorm::output::Capabilities )
                {return true;}
        };

      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::output::OutputBuilder<SpotFinderEstimator> Source;

        SpotFinderEstimator (Config&);
        ~SpotFinderEstimator();
        SpotFinderEstimator* clone() const { 
            throw std::runtime_error("No SpotFinderEstimator::clone"); }

        AdditionalData announceStormSize(const Announcement &a)
        { 
            ost::MutexLock lock(mutex);
            traits = a;
            if ( ! a.image_number().range().second.is_set() )
                throw std::runtime_error("Unknown total frame count not "
                                         "allowed in SpotFinderEstimator");
            numberOfSamples = *a.image_number().range().second / camera::frame;
            data.resize(numberOfSamples);
            carburettor = a.carburettor;
            return AdditionalData().set_candidate_tree()
                                   .set_input_buffer();
        }
        Result receiveLocalizations(const EngineResult&);
        
        void propagate_signal(ProgressSignal s)
            { if ( s == Engine_run_succeeded ) finish(); }
    };

}
#endif
