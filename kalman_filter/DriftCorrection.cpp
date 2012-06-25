#include <dStorm/output/Output.h>
#include <simparm/Entry.h>
#include <simparm/FileEntry.h>
#include <simparm/Node.h>
#include <fstream>
#include <memory>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/units/frame_count.h>
#include <boost/foreach.hpp>

#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>
#include <unsupported/Eigen/SparseExtra>
#include <unsupported/Eigen/UmfPackSupport>
#include <boost/lexical_cast.hpp>

namespace dStorm {
namespace kalman_filter {

class NonlinearDriftEstimator : public output::Output {
private:
    struct BeadPosition {
        samplepos position, uncertainty;
        frame_index time;

        BeadPosition( const Localization& l ) 
        : position( l.position() ), uncertainty( l.position.uncertainty() ), time( l.frame_number() ) {}
    };
    struct Bead {
        std::vector< BeadPosition > positions;
    };
    std::vector< Bead > beads;
    std::map< frame_index, int > time_identifiers;
    std::string output_file;

    void store_results_( bool success ) {
        if ( ! success ) return;
        int bead_count = beads.size();
        int distinct_times = time_identifiers.size() - 1;
        int variable_count = bead_count + distinct_times;
        int time_offset = bead_count;

        int measurement_count = 0;
        BOOST_FOREACH( const Bead& b, beads ) {
            measurement_count += b.positions.size();
        }

        typedef Eigen::SparseMatrix<double, Eigen::RowMajor> VariableMatrix;
        VariableMatrix variables( measurement_count, variable_count );
        Eigen::VectorXd measurements[2];
        for (int i = 0; i < 2; ++i) 
            measurements[i].resize( measurement_count );
        variables.reserve( measurement_count * 2 );
        int measurement_index = 0;
        for ( int bead_index = 0; bead_index < bead_count; ++bead_index ) {
            BOOST_FOREACH( const BeadPosition& measurement, beads[bead_index].positions ) {
                variables.startVec( measurement_index );
                variables.insertBack( measurement_index, bead_index ) = 1;
                if ( time_identifiers[measurement.time] != 0 )
                    variables.insertBack( measurement_index, time_offset + time_identifiers[measurement.time] - 1 ) = 1;
                for (int i = 0; i < 2; ++i) 
                    measurements[i][ measurement_index ] = measurement.position[i].value() * 1E9;
                ++measurement_index;
            }
        }
        variables.finalize();

        Eigen::SparseLDLT< VariableMatrix > decomposed( variables.transpose() * variables );
        if ( ! decomposed.succeeded() )
            throw std::runtime_error("Unable to invert least squares variable matrix");
        Eigen::VectorXd result[2];
        for (int i = 0; i < 2; ++i) {
            Eigen::VectorXd t = variables.transpose() * measurements[i];
            result[i] = decomposed.solve( t );
        }

        std::ofstream output( output_file.c_str() );
        for (int bead = 0; bead < bead_count; ++bead)
            output << result[0][bead] / 1E9 << " " << result[1][bead] / 1E9 << "\n";
        output << "\n\n";
        typedef std::pair< frame_index, int > TimePair;

        BOOST_FOREACH( TimePair time, time_identifiers ) {
            if ( time.second == 0 )
                output << time.first << " 0 0\n";
            else
                output << time.first << " " << result[0][ time.second + time_offset - 1 ] 
                                     << " " << result[1][ time.second + time_offset - 1 ] << "\n";
        }
    }
    void attach_ui_( simparm::NodeHandle at ) {}

public:
    struct Config { 
        simparm::FileEntry output_file;

        Config() : output_file("ToFile", "Write localization count to file", "-drift.txt") {}
        bool can_work_with(output::Capabilities) { return true; }
        void attach_ui( simparm::NodeHandle at ) { output_file.attach_ui( at ); }
        static std::string get_name() { return "NonlinearDrift"; }
        static std::string get_description() { return "Nonlinear drift estimator"; }
        static simparm::UserLevel get_user_level() { return simparm::Expert; }
    };

    NonlinearDriftEstimator(const Config & c) : output_file( c.output_file() ) {}

    RunRequirements announce_run(const RunAnnouncement&) {
        return RunRequirements();
    }
    AdditionalData announceStormSize(const Announcement &a) {
        return AdditionalData();
    }
    void receiveLocalizations(const EngineResult& er) {
        for ( EngineResult::const_iterator bead = er.begin(); bead != er.end(); ++bead ) {
            beads.push_back( Bead() );
            Bead& b = beads.back();
            for ( std::vector<Localization>::const_iterator l = bead->children->begin(); l != bead->children->end(); ++l ) {
                b.positions.push_back( BeadPosition( *l ) );
                time_identifiers.insert( std::make_pair( l->frame_number(), time_identifiers.size() ) );
            }
        }
    }

};

std::auto_ptr< output::OutputSource > create_drift_correction() {
    return std::auto_ptr< output::OutputSource >( new output::OutputBuilder< NonlinearDriftEstimator::Config, NonlinearDriftEstimator >() );
}

}
}
