#ifndef LOCPREC_REGIONSEGMENTER_H
#define LOCPREC_REGIONSEGMENTER_H

#include <dStorm/outputs/BinnedLocalizations_strategies_config.h>
#include <boost/ptr_container/ptr_array.hpp>
#include <dStorm/output/binning/config.h>
#include <dStorm/output/Localizations.h>
#include <dStorm/outputs/BinnedLocalizations.h>
#include <dStorm/outputs/LocalizationList.h>
#include <dStorm/output/FilterBuilder.h>
#include <dStorm/engine/Image.h>
#include <dStorm/outputs/Crankshaft.h>
#include <dStorm/outputs/TraceFilter.h>
#include <dStorm/output/TraceReducer.h>
#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/helpers/thread.h>
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/FileEntry.hh>
#include <cassert>

namespace locprec {
    class Segmenter : public dStorm::outputs::Crankshaft,
        public simparm::Node::Callback,
        private dStorm::Display::DataSource
    {
        class _Config;
      public:
        enum SegmentationType { Maximum, Region };

        typedef simparm::Structure<_Config> Config;
        typedef dStorm::output::FilterBuilder<Segmenter> Source;

      private:
        typedef dStorm::Image<dStorm::Pixel,2> ColorImage;
        typedef dStorm::Image<int,2> RegionImage;

        ost::Mutex mutex;

        std::auto_ptr<Announcement> announcement;
        SegmentationType howToSegment;
        boost::ptr_array< dStorm::output::binning::Unscaled, 2 > binners;
        simparm::Entry<double> threshold;
        simparm::Entry<unsigned long> dilation;
        dStorm::output::Localizations points;

        dStorm::outputs::BinnedLocalizations
            <dStorm::outputs::DummyBinningListener>* bins;

        std::auto_ptr< dStorm::Display::Change > next_change;
        std::auto_ptr< dStorm::Display::Manager::WindowHandle > display;

        std::auto_ptr< dStorm::output::Output > output;
        std::auto_ptr< dStorm::output::TraceReducer > reducer;

        std::string load_segmentation, save_segmentation;

        static ColorImage color_regions( const RegionImage& );
        void display_image( const ColorImage& );
        std::auto_ptr<dStorm::Display::Change> get_changes();

      protected:
        RegionImage segment_image();
        void segment();
        void maximums();

      public:
        Segmenter( const Config& config,
                   std::auto_ptr<dStorm::output::Output> output);
        Segmenter( const Segmenter & );
        ~Segmenter();
        Segmenter* clone() const 
            { throw std::runtime_error("Object unclonable."); }

        AdditionalData announceStormSize(const Announcement &a) ;
        void propagate_signal(ProgressSignal s) {
            dStorm::outputs::Crankshaft::propagate_signal(s);
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

        void operator()(const simparm::Event&);
    };

    struct Segmenter::_Config : public simparm::Object {
        simparm::DataChoiceEntry<Segmenter::SegmentationType> method;
        dStorm::outputs::DimensionSelector selector;
        simparm::Entry<double> threshold;
        simparm::Entry<unsigned long> dilation;
        simparm::FileEntry save_segmentation, load_segmentation;
        dStorm::output::TraceReducer::Config reducer;
        dStorm::outputs::TraceCountFilter::Source trace_filter;

        _Config();
        void registerNamedEntries();
        bool determine_output_capabilities
            ( dStorm::output::Capabilities& cap ) 
        { 
            cap.set_intransparency_for_source_data();
            cap.set_cluster_sources( true );
            return trace_filter.determine_output_capabilities( cap ); 
        }
    };
}

#endif
