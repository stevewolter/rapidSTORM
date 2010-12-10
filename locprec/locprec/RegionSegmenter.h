#ifndef LOCPREC_REGIONSEGMENTER_H
#define LOCPREC_REGIONSEGMENTER_H

#include <dStorm/Localizations.h>
#include <dStorm/BinnedLocalizations.h>
#include <dStorm/LocalizationList.h>
#include <dStorm/TransmissionConfig.h>
#include <dStorm/Image.h>
#include <dStorm/Crankshaft.h>
#include <dStorm/TraceReducer.h>
#include <CImgBuffer/Image.h>

namespace locprec {
    class Segmenter : public dStorm::Crankshaft {
      public:
        enum SegmentationType { Maximum, Region };

      private:
        SegmentationType howToSegment;
        double res_enh;
        dStorm::Localizations points;

        dStorm::LocalizationList filler;
        dStorm::BinnedLocalizations bins;

        std::auto_ptr< dStorm::Transmission > output;
        std::auto_ptr< dStorm::TraceReducer > reducer;

        ost::Mutex mutex;

      protected:
        void segment() throw();
        void maximums() throw();

      public:
        Segmenter( SegmentationType howToSegment, double bin_size, 
                   std::auto_ptr<dStorm::Transmission> output,
                   std::auto_ptr<dStorm::TraceReducer> reducer) throw();
        Segmenter( const Segmenter & ) throw();
        virtual ~Segmenter() throw() {}
        Segmenter* clone() const { return new Segmenter(*this); }

        AdditionalData announceStormSize(int w, int h, int l) throw() 
        {
            ost::MutexLock lock( mutex );
            /* TODO: Evaluate return */
            output->announceStormSize(w,h,l);
            return dStorm::Crankshaft::announceStormSize(w,h,l);
        }
        void finish() throw() {
            dStorm::Crankshaft::finish();
            ost::MutexLock lock(mutex);
            if ( howToSegment == Maximum )
                maximums();
            else
                segment();
        }
    };

    class SegmenterConfig : public dStorm::TransmissionForwarderSource {
        simparm::ConfigEntryDataChoice<Segmenter::SegmentationType> method;
        simparm::ConfigEntryDouble bin_size;
        std::auto_ptr<dStorm::TraceReducerConfig> reducer;

      public:
        SegmenterConfig(dStorm::TransmissionSourceFactory& tc) throw()
        : dStorm::ConfigObject("Segmenter", "Segment target image"),
          dStorm::TransmissionForwarderSource(tc),
          method("SegmentationMethod", "Method for segmentation"),
          bin_size("SegmentationBinSize", "Bin size for segmentable image"),
          reducer( dStorm::TraceReducerConfig::factory() )
        {
            method.addChoice( Segmenter::Maximum,       
                "Maximum", "Local maximums");
            method.addChoice( Segmenter::Region, 
                "Regions", "Coherent regions");
            push_back( bin_size );
            push_back( method );
            push_back( *reducer );
        }
        SegmenterConfig(const SegmenterConfig& c) throw() 
        : dStorm::ConfigObject(c),
          dStorm::TransmissionForwarderSource(c),
          bin_size(c.bin_size),
          reducer( c.reducer->clone() )
        {
            push_back( bin_size );
            push_back( method );
            push_back( *reducer );
        }
        ~SegmenterConfig() throw() {}
        SegmenterConfig* clone() const 
            { return new SegmenterConfig(*this); }

        virtual std::auto_ptr<dStorm::Transmission> makeTransmission() throw() {
            assert( output.get() != NULL );
            return std::auto_ptr<dStorm::Transmission>( new Segmenter(
                method(), bin_size(), output->makeTransmission(), 
                reducer->make_trace_reducer() ) );
        }
    };
}

#endif
