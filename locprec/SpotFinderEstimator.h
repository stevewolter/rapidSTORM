#ifndef LOCPREC_SPOTFINDERESTIMATOR_H
#define LOCPREC_SPOTFINDERESTIMATOR_H

#include <dStorm/Output.h>
#include <dStorm/OutputBuilder.h>
#include <cc++/thread.h>
#include "NoiseSource.h"
#include <list>
#include <data-c++/Vector.h>
#include <simparm/FileEntry.hh>

namespace locprec {
    class SpotFinderEstimator 
    : public simparm::Object, public dStorm::Output
    {
      private:
        const std::list<Fluorophore*>* fluorophores;
        dStorm::Input *carburettor;
        int msx, msy;

        ost::Mutex mutex;
        int numberOfSamples;
        int noCol;
        data_cpp::Vector<double[8]> data;
        std::ostream &out;

        void finish();

        class _Config : public simparm::Object {
          protected:
            void registerNamedEntries() {push_back( outputFile );}
          public:
            simparm::FileEntry outputFile;

            _Config();
        };

      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::OutputBuilder<SpotFinderEstimator> Source;

        SpotFinderEstimator (Config&);
        ~SpotFinderEstimator();
        SpotFinderEstimator* clone() const { 
            throw std::runtime_error("No SpotFinderEstimator::clone"); }

        AdditionalData announceStormSize(const Announcement &a)

        { 
            ost::MutexLock lock(mutex);
            numberOfSamples = a.length;
            data.resize(a.length);
            carburettor = a.carburettor;
            return AdditionalData(CandidateTree | InputBuffer);
        }
        Result receiveLocalizations(const EngineResult&);
        
        void propagate_signal(ProgressSignal s)
            { if ( s == Engine_run_succeeded ) finish(); }
    };

}
#endif
