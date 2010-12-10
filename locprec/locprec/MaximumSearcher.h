#ifndef LOCPREC_MAXIMUMSEARCHER_H
#define LOCPREC_MAXIMUMSEARCHER_H

#include <locprec/MoleculeLocalization.h>
#include <dStorm/BinnedLocalizations.h>
#include <dStorm/LocalizationList.h>
#include <dStorm/Crankshaft.h>
#include <dStorm/TraceReducer.h>
#include <dStorm/TransmissionConfig.h>

namespace locprec {
    class MaximumSearcher : public dStorm::Crankshaft {
      private:
        dStorm::BinnedLocalizations binnedImage;
        dStorm::LocalizationList localizations;
        std::list<MoleculeLocalization> molLocs;

        std::auto_ptr< dStorm::Transmission > output;
        std::auto_ptr< dStorm::TraceReducer > reducer;

        ost::Mutex mutex;

      public:
        MaximumSearcher(double res_enh,
            std::auto_ptr<dStorm::Transmission> output,
            std::auto_ptr<dStorm::TraceReducer> reducer) throw()
          : ConfigObject("MaximumSearcher", ""),
            binnedImage(res_enh) ,
            output(output),
            reducer(reducer)
        {
            add( binnedImage );
            add( localizations );
        }
        virtual ~MaximumSearcher() throw() {}

        void finish() throw();
    };

    class MaximumSearcherConfig 
    : public dStorm::TransmissionForwarderSource 
    {
        simparm::ConfigEntryDouble bin_size;
        std::auto_ptr<dStorm::TraceReducerConfig> reducer;

      public:
        MaximumSearcherConfig(dStorm::TransmissionSourceFactory& tc) throw()
        : dStorm::ConfigObject("RegionSegmenter", "Segment into regions"),
          dStorm::TransmissionForwarderSource(tc),
          bin_size("SegmentationBinSize", "Bin size for segmentable image"),
          reducer( dStorm::TraceReducerConfig::factory() )
        {
            push_back( bin_size );
            push_back( *reducer );
        }
        MaximumSearcherConfig(const MaximumSearcherConfig& c) throw() 
        : dStorm::ConfigObject(c),
          dStorm::TransmissionForwarderSource(c),
          bin_size(c.bin_size),
          reducer( c.reducer->clone() )
        {
            push_back( bin_size );
            push_back( *reducer );
        }
        ~MaximumSearcherConfig() throw() {}
        MaximumSearcherConfig* clone() const 
            { return new MaximumSearcherConfig(*this); }

        virtual std::auto_ptr<dStorm::Transmission> makeTransmission() throw() {
            assert( output.get() != NULL );
            return std::auto_ptr<dStorm::Transmission>( new MaximumSearcher(
                bin_size(), output->makeTransmission(), 
                reducer->make_trace_reducer() ) );
        }
    };
}

#endif
