#ifndef LOCPREC_REGIONSEGMENTER_H
#define LOCPREC_REGIONSEGMENTER_H

#include <dStorm/engine/Localizations.h>
#include <dStorm/transmissions/BinnedLocalizations.h>
#include <dStorm/transmissions/LocalizationList.h>
#include <dStorm/FilterBuilder.h>
#include <dStorm/engine/Image.h>
#include <dStorm/transmissions/Crankshaft.h>
#include <dStorm/transmissions/TraceFilter.h>
#include <dStorm/engine/TraceReducer.h>
#include <simparm/NumericEntry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/FileEntry.hh>
#include <cassert>

namespace locprec {
    class Segmenter : public dStorm::Crankshaft,
        public simparm::Node::Callback
    {
        class _Config;
      public:
        enum SegmentationType { Maximum, Region };

        typedef simparm::Structure<_Config> Config;
        typedef dStorm::FilterBuilder<Segmenter> Source;

      private:
        SegmentationType howToSegment;
        double res_enh;
        simparm::DoubleEntry threshold;
        simparm::UnsignedLongEntry dilation;
        dStorm::Localizations points;

        dStorm::LocalizationList filler;
        dStorm::BinnedLocalizations<dStorm::DummyBinningListener> bins;

        std::auto_ptr<cimg_library::CImgDisplay> display;

        std::auto_ptr< dStorm::Output > output;
        std::auto_ptr< dStorm::TraceReducer > reducer;

        ost::Mutex mutex;

        std::string load_segmentation, save_segmentation;

      protected:
        cimg_library::CImg<int> segment_image();
        void segment();
        void maximums();

      public:
        Segmenter( const Config& config,
                   std::auto_ptr<dStorm::Output> output);
        Segmenter( const Segmenter & );
        virtual ~Segmenter() {}
        Segmenter* clone() const 
            { throw std::runtime_error("Object unclonable."); }

        AdditionalData announceStormSize(const Announcement &a) ;
        void propagate_signal(ProgressSignal s) {
            dStorm::Crankshaft::propagate_signal(s);
            if ( s == Engine_run_succeeded ) {
                ost::MutexLock lock(mutex);
                if ( howToSegment == Maximum )
                    maximums();
                else
                    segment();
            }
            if ( s == Job_finished_successfully ||
                 s == Prepare_destruction )
                output->propagate_signal(s);
        }

        void operator()(simparm::Node&, Cause, simparm::Node*);
    };

    struct Segmenter::_Config : public simparm::Object {
        simparm::DataChoiceEntry<Segmenter::SegmentationType> method;
        simparm::DoubleEntry bin_size;
        simparm::DoubleEntry threshold;
        simparm::UnsignedLongEntry dilation;
        simparm::FileEntry save_segmentation, load_segmentation;
        dStorm::TraceReducer::Config reducer;
        dStorm::TraceCountFilter::Source trace_filter;

        _Config();
        void registerNamedEntries();
    };
}

#endif
