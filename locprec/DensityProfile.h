#ifndef LOCPREC_DENSITYPROFILE_H
#define LOCPREC_DENSITYPROFILE_H

#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>

namespace locprec {

class DensityProfile : public dStorm::output::OutputObject {
    Eigen::Rotation2D<double> rotation;
    double binSize, expectedPeriod;
    std::vector<int> pos_counts, neg_counts;

    class _Config : public simparm::Object {
      protected:
        void registerNamedEntries() 
            { push_back( angle ); push_back( binSize );
              push_back( expectedPeriod ); }
      public:
        simparm::Entry<double> angle, binSize, expectedPeriod;

        _Config()
            : Object("DensityProfile", ""),
            angle("RotationAngle", "Rotation angle in degrees") ,
            binSize("BinSize", "Bin size in pixels", 0.1),
            expectedPeriod("ExpectedPeriod", "Expected period", -1)
            { binSize.min = (1E-5); userLevel = Expert; }
        bool can_work_with( dStorm::output::Capabilities ) {return true;}
    };

    void store_results_( bool success ) {
        for ( int i = neg_counts.size()-1; i >= 0; i--)
            std::cout << (-i-1)*binSize << " " 
                        << neg_counts[i] << "\n";
        for ( unsigned int i = 0; i < pos_counts.size(); i++)
            std::cout << i * binSize << " " << pos_counts[i] << "\n";
    }

  public:
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<DensityProfile> Source;
    
    DensityProfile(const Config& config)
    : OutputObject("DensityProfile", ""),
      rotation(2 * M_PI * config.angle() / 360),
      binSize( config.binSize() ),
      expectedPeriod( config.expectedPeriod() )
    {}
    DensityProfile* clone() const { return new DensityProfile(*this); }

    AdditionalData announceStormSize(const Announcement &) 
        { return AdditionalData(); }
    RunRequirements announce_run(const RunAnnouncement&) {
        pos_counts.clear();
        neg_counts.clear();
        return RunRequirements(); 
    }
    void receiveLocalizations(const EngineResult &er) 
    {
        for (EngineResult::const_iterator i = er.begin(); i != er.end(); ++i)
        {
            Eigen::Vector2d orig( 
                i->position().x().value(),
                i->position().y().value() );
            double x = (rotation * orig).x();

            if ( expectedPeriod > 0 ) {
                while ( x >= expectedPeriod ) x -= expectedPeriod;
                while ( x < 0 ) x += expectedPeriod;
            }
            int bin = round( x / binSize );
            unsigned int vecbin = ( bin >= 0 ) ? bin : -(bin+1);
            std::vector<int>& v = ( bin >= 0 ) ? pos_counts : neg_counts;
            if ( v.size() <= vecbin ) v.resize( vecbin + 10, 0 );
            v[vecbin]++;
        }
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}

#endif
