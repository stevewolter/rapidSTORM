#ifndef LOCPREC_PRECISIONESTIMATOR_H
#define LOCPREC_PRECISIONESTIMATOR_H

#include <memory>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/outputs/LocalizationList.h>
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>
#include <Eigen/Core>

namespace dStorm {
namespace output {
    class SinglePrecisionEstimator 
      : public OutputObject 
    {
      private:
        simparm::FileEntry printTo;
        double res_enh;

        ost::Mutex mutex;

        class _Config;

      public:
        typedef simparm::Structure<_Config> Config;
        typedef output::FileOutputBuilder<SinglePrecisionEstimator> Source;

        SinglePrecisionEstimator ( const Config& config );
        SinglePrecisionEstimator *clone() const;

        AdditionalData announceStormSize(const Announcement&); 
        Result receiveLocalizations(const EngineResult&);
        void propagate_signal(ProgressSignal);

        void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
            { insert_filename_with_check( printTo(), present_filenames ); }

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    class MultiPrecisionEstimator 
    : public OutputObject 
    {
        simparm::UnsignedLongEntry usedSpots;
        simparm::DoubleEntry x_sd, y_sd, corr;
        double res_enh;

        outputs::LocalizationList localizations;

        ost::Mutex mutex;

        void registerNamedEntries();
        void estimatePrecision();

        class _Config;
        
      public:
        typedef simparm::VirtualStructure<_Config> Config;
        typedef output::OutputBuilder<MultiPrecisionEstimator> Source;

        MultiPrecisionEstimator( const Config& config );
        MultiPrecisionEstimator(const MultiPrecisionEstimator& c);

        MultiPrecisionEstimator *clone() const;

        AdditionalData announceStormSize(const Announcement& a);
        Result receiveLocalizations(const EngineResult& e)
            { return localizations.receiveLocalizations(e); }
        void propagate_signal(ProgressSignal s) 
        {
            localizations.propagate_signal(s);
            if ( s == Engine_run_succeeded )
                estimatePrecision();
        }

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    class PrecisionEstimatorConfig 
    {
      protected:

        void registerNamedEntries(simparm::Node& n) {
            n.push_back(resEnh);
        }

      public:
        simparm::DoubleEntry resEnh;

        PrecisionEstimatorConfig()
            : resEnh("FitBinSize", "Bin size for precision Gauss fit", 0.1)
            {}

        bool can_work_with(Capabilities cap) 
            { return cap.test( Capabilities::ClustersWithSources ); }
    };

    struct MultiPrecisionEstimator::_Config 
        : public PrecisionEstimatorConfig, public simparm::Object
    { 
        _Config(); 
        _Config* clone() const { return new _Config(*this); }
        void registerNamedEntries() {
            PrecisionEstimatorConfig::registerNamedEntries(*this);
        }
    };

    class SinglePrecisionEstimator::_Config :
        public PrecisionEstimatorConfig, public simparm::Object
    {
      protected:
        void registerNamedEntries() {
            PrecisionEstimatorConfig::registerNamedEntries(*this);
            push_back( outputFile );
        }
      public:
        BasenameAdjustedFileEntry outputFile;

        _Config();
        _Config* clone() const { return new _Config(*this); }
    };
}
}
#endif
