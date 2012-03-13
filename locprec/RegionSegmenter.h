#ifndef LOCPREC_REGIONSEGMENTER_H
#define LOCPREC_REGIONSEGMENTER_H

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
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
#include <dStorm/display/Manager.h>
#include <boost/thread/mutex.hpp>
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/FileEntry.hh>
#include <cassert>

namespace locprec {
    class Segmenter : public dStorm::outputs::Crankshaft,
        public simparm::Node::Callback,
        private dStorm::display::DataSource
    {
        class _Config;
      public:
        enum SegmentationType { Maximum, Region };

        typedef simparm::Structure<_Config> Config;
        typedef dStorm::output::FilterBuilder<Segmenter> Source;

      private:
        typedef dStorm::Image<dStorm::Pixel,2> ColorImage;
        typedef dStorm::Image<int,2> RegionImage;

        boost::mutex mutex;

        std::auto_ptr<Announcement> announcement;
        SegmentationType howToSegment;
        boost::ptr_array< dStorm::output::binning::Unscaled, 2 > binners;
        simparm::Entry<double> threshold;
        simparm::Entry<unsigned long> dilation;
        dStorm::output::Localizations points;

        dStorm::outputs::BinnedLocalizations<>* bins;

        std::auto_ptr< dStorm::display::Change > next_change;
        std::auto_ptr< dStorm::display::Manager::WindowHandle > display;

        std::auto_ptr< dStorm::output::Output > output;
        std::auto_ptr< dStorm::output::TraceReducer > reducer;

        std::string load_segmentation, save_segmentation;

        static ColorImage color_regions( const RegionImage& );
        void display_image( const ColorImage& );
        std::auto_ptr<dStorm::display::Change> get_changes();

        void store_results_( bool success ) {
            dStorm::outputs::Crankshaft::store_results_( success );
            boost::lock_guard<boost::mutex> lock(mutex);
            if ( howToSegment == Maximum )
                maximums();
            else
                segment();
        }

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

        void operator()(const simparm::Event&);
    };

    struct Segmenter::_Config : public simparm::Object {
        simparm::DataChoiceEntry<Segmenter::SegmentationType> method;
        dStorm::outputs::DimensionSelector<2> selector;
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
