#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dStorm/output/Output.h>
#include <simparm/Entry.h>
#include <simparm/FileEntry.h>
#include <simparm/Node.h>
#include <fstream>
#include <memory>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/units/frame_count.h>
#include <boost/foreach.hpp>

#if HAVE_EIGEN_SPARSECHOLESKY
#include <Eigen/Sparse>
#else
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <unsupported/Eigen/SparseExtra>
#endif
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/range/algorithm/binary_search.hpp>
#include <boost/units/io.hpp>

namespace br = boost::range;

namespace dStorm {
namespace kalman_filter {

struct BeadPosition {
    samplepos position, uncertainty;
    frame_index time;
    int bead_id;

    BeadPosition() {}
    BeadPosition( const Localization& l, const input::Traits<Localization>& t, int bead_id ) 
    : position( l.position() ), time( l.frame_number() ), bead_id( bead_id )
    {
        for (int i = 0; i < position.rows(); ++i) {
            if ( ! t.position().is_given[i] )
                position[i] = 0 * si::meter;
        }

        uncertainty = samplepos::Constant(1E-8f * si::meter);
        if (t.position_uncertainty_x().is_given) {
          uncertainty[0] = l.position_uncertainty_x();
        }
        if (t.position_uncertainty_y().is_given) {
          uncertainty[1] = l.position_uncertainty_y();
        }
        if (t.position_uncertainty_z().is_given) {
          uncertainty[2] = l.position_uncertainty_z();
        }
    }
};

class DriftSection {
    static const samplepos::Scalar position_unity, uncertainty_unity;

    static const int Dimensions = samplepos::RowsAtCompileTime;
    typedef Eigen::SparseMatrix<double> EquationSystem;
    typedef Eigen::VectorXd Vector;
    typedef std::vector<frame_index>::const_iterator TimeRef;

    TimeRef time_begin, time_end;
    EquationSystem equations_transposed;
    Vector measurements[Dimensions], weights[Dimensions], solutions[Dimensions], covariances[Dimensions];
    int measurement_index, time_offset;
    bool equation_systems_solved;
    const input::Traits<Localization>& traits;
    std::vector<int> bead_ids;

    int time_index( frame_index f ) {
        TimeRef i = std::lower_bound( time_begin, time_end, f );
        assert( i != time_end && *i == f );
        return i - time_begin;
    }
    int bead_index( int bead_id ) {
        std::vector<int>::iterator i = br::find( bead_ids, bead_id );
        if ( i == bead_ids.end() )
            i = bead_ids.insert( i, bead_id );
        return i - bead_ids.begin();
    }
    void solve_equation_systems();
    
public:
    DriftSection( int number_of_beads, int number_of_measurements,
                    TimeRef begin_time, TimeRef end_time,
                    const input::Traits<Localization>& traits );
    void add_measurement( BeadPosition position );
    std::vector<BeadPosition> bead_positions();
    std::vector<BeadPosition> get_drift();
};

DriftSection::DriftSection( 
    int number_of_beads, int number_of_measurements,
    TimeRef begin_time, TimeRef end_time,
    const input::Traits<Localization>& traits )
: time_begin( begin_time ), time_end( end_time ),
  equations_transposed( (end_time-begin_time) - 1 + number_of_beads, number_of_measurements ),
  measurement_index(0),
  time_offset(number_of_beads),
  equation_systems_solved(false),
  traits(traits)
{
    bead_ids.reserve( number_of_beads );
    for (int i = 0; i < Dimensions; ++i) {
        measurements[i].resize( number_of_measurements );
        weights[i].resize( number_of_measurements );
    }
}

void DriftSection::add_measurement( BeadPosition measurement ) {
    int bead_column = bead_index( measurement.bead_id );
    equations_transposed.startVec( measurement_index );
    equations_transposed.insertBack( bead_column, measurement_index ) = 1;
    if ( measurement.time != *time_begin )
        equations_transposed.insertBack( time_offset + time_index(measurement.time) - 1, measurement_index ) = 1;
    for (int i = 0; i < Dimensions; ++i) 
        measurements[i][ measurement_index ] = measurement.position[i] / position_unity;
    for (int i = 0; i < Dimensions; ++i) 
        weights[i][ measurement_index ] = 1.0 / pow<2>( measurement.uncertainty[i] / uncertainty_unity );
    ++measurement_index;
}


#if HAVE_EIGEN_SPARSECHOLESKY
typedef Eigen::SimplicialLDLT< Eigen::SparseMatrix<double> > SparseSolver;
static bool decomposition_failed( const SparseSolver& s )
    { return s.info() != Eigen::Success; }
#else
typedef Eigen::SparseLDLT< Eigen::SparseMatrix<double> > SparseSolver;
static bool decomposition_failed( const SparseSolver& s )
    { return ! s.succeeded(); }
#endif

void DriftSection::solve_equation_systems() {
    assert( measurement_index == equations_transposed.cols() );
    assert( int(bead_ids.size()) == time_offset );
    equations_transposed.finalize();

    for (int i = 0; i < Dimensions; ++i) {
        EquationSystem squaring_matrix;
        squaring_matrix = equations_transposed * weights[i].asDiagonal();
        EquationSystem square = squaring_matrix * equations_transposed.transpose();

        SparseSolver decomposed;
        decomposed.compute( square );
        if ( decomposition_failed( decomposed ) )
            throw std::runtime_error("Unable to invert least squares variable matrix");

        Eigen::VectorXd t = squaring_matrix * measurements[i];
        solutions[i] = decomposed.solve( t );
        if ( decomposition_failed( decomposed ) )
            throw std::runtime_error("Unable to solve equation system for positions");

        covariances[i] = Eigen::VectorXd( time_offset );
        for (int bead = 0; bead < time_offset; ++bead) {
            Eigen::VectorXd identity_column = Eigen::VectorXd::Unit( t.rows(), bead );
            Eigen::VectorXd inverse_column;
            inverse_column = decomposed.solve( identity_column );
            if ( decomposition_failed( decomposed ) )
                throw std::runtime_error("Unable to solve equation system for covariances");
            covariances[i][bead] = inverse_column[bead];
        }
    }

    equation_systems_solved = true;
}

std::vector<BeadPosition> DriftSection::bead_positions() {
    if ( ! equation_systems_solved ) solve_equation_systems();

    std::vector<BeadPosition> rv( time_offset, BeadPosition() );
    for (int i = 0; i < time_offset; ++i) {
        for (int d = 0; d < Dimensions; ++d) {
            rv[i].position[d] = float(solutions[d][i]) * position_unity;
            rv[i].uncertainty[d] = float(sqrt(covariances[d][i])) * uncertainty_unity;
        }
        rv[i].bead_id = bead_ids[i];
        rv[i].time = *time_begin;
    }
    return rv;
}

std::vector<BeadPosition> DriftSection::get_drift() {
    if ( ! equation_systems_solved ) solve_equation_systems();

    std::vector<BeadPosition> rv( (time_end - time_begin), BeadPosition() );
    for ( TimeRef t = time_begin; t != time_end; ++t )
        rv[t - time_begin].time = *t;
    rv[0].position.fill( 0 * si::meter );
    for (int i = 0; i < int(rv.size()-1); ++i) {
        for (int d = 0; d < Dimensions; ++d) {
            rv[i+1].position[d] = float(solutions[d][i + time_offset]) * position_unity;
        }
    }
    return rv;
}

class NonlinearDriftEstimator : public output::Output {
private:
    static const int Dimensions = samplepos::RowsAtCompileTime;
    std::vector< frame_index > times;
    std::vector< BeadPosition > measurements;
    std::string output_file;
    frame_count slice_size;

    boost::optional< input::Traits<Localization> > traits;

    int time_index( frame_index t ) {
        assert( t != times.front() );
        return boost::range::lower_bound( times, t ) - times.begin();
    }

    void store_results_( bool success ) {
        if ( ! success || times.empty() ) return;

        std::vector< BeadPosition > meta_positions;
        std::vector< frame_index > meta_times;

        int slice_count = times.size() / slice_size.value() + 1;

        std::vector< std::vector<BeadPosition> > intra_slice_positions;
        intra_slice_positions.reserve( slice_count );

        std::vector< frame_index >::const_iterator last_end = times.begin();
        for (int slice = 0; slice < slice_count; ++slice) {
            std::vector< frame_index >::const_iterator slice_start = last_end, 
                slice_end = (slice+1 == slice_count) ? times.end() : (slice_start + times.size() / slice_count);

            meta_times.push_back( *slice_start );

            int number_of_measurements = 0; std::set<int> active_beads;
            BOOST_FOREACH( const BeadPosition& position, measurements )
                if ( position.time >= *slice_start && (slice_end == times.end() || position.time < *slice_end ) ) {
                    ++number_of_measurements;
                    active_beads.insert( position.bead_id );
                }

            DriftSection section( active_beads.size(), number_of_measurements, slice_start, slice_end, *traits );
            BOOST_FOREACH( const BeadPosition& position, measurements )
                if ( position.time >= *slice_start && (slice_end == times.end() || position.time < *slice_end ) )
                    section.add_measurement( position );
                    
            intra_slice_positions.push_back( section.get_drift() );
            std::vector< BeadPosition > origins = section.bead_positions();
            std::copy( origins.begin(), origins.end(), std::back_inserter( meta_positions ) );

            last_end = slice_end;
        }

        std::set<int> active_beads;
        BOOST_FOREACH( const BeadPosition& position, meta_positions )
            active_beads.insert( position.bead_id );

        DriftSection meta_section( active_beads.size(), meta_positions.size(), meta_times.begin(), meta_times.end(), *traits );
        BOOST_FOREACH( const BeadPosition& position, meta_positions )
            meta_section.add_measurement( position );

        std::vector< BeadPosition > meta_trace = meta_section.get_drift();

        std::ofstream output( output_file.c_str() );
        for (int slice = 0; slice < slice_count; ++slice) {
            BOOST_FOREACH( const BeadPosition& p, intra_slice_positions[slice] ) {
                output << p.time.value();
                for (int dim = 0; dim < Dimensions; ++dim)
                    output << " " << quantity<si::nanolength>( meta_trace[slice].position[dim] + p.position[dim] ).value() ;
                output << "\n";
            }
        }
    }
    void attach_ui_( simparm::NodeHandle at ) {}

public:
    struct Config { 
        simparm::FileEntry output_file;
        simparm::Entry< frame_index > slice_size;

        Config() : output_file("ToFile", "Write localization count to file", "-drift.txt"),
                   slice_size("SectionSize", "Section size", 200 * camera::frame) {}
        bool can_work_with(output::Capabilities) { return true; }
        void attach_ui( simparm::NodeHandle at ) { output_file.attach_ui( at ); slice_size.attach_ui( at ); }
        static std::string get_name() { return "NonlinearDrift"; }
        static std::string get_description() { return "Nonlinear drift estimator"; }
        static simparm::UserLevel get_user_level() { return simparm::Expert; }
    };

    NonlinearDriftEstimator(const Config & c) : output_file( c.output_file() ), slice_size( c.slice_size() ) {}

    RunRequirements announce_run(const RunAnnouncement&) {
        return RunRequirements();
    }
    AdditionalData announceStormSize(const Announcement &a) {
        if ( a.source_traits.size() >= 1 && a.source_traits[0].get() )
            traits = *a.source_traits[0];
        else
            throw std::runtime_error("The current input for the nonlinear drift estimator consists only "
                "of flat localizations without trace information. "
                "Time traces such as those produced by the emission tracker are needed.");
        return AdditionalData();
    }
    void receiveLocalizations(const EngineResult& er) {
        for ( EngineResult::const_iterator bead = er.begin(); bead != er.end(); ++bead ) {
            int bead_id = bead - er.begin();
            BOOST_FOREACH( const Localization& l, *bead->children ) {
                measurements.push_back( BeadPosition( l, *traits, bead_id ) );
                std::vector< frame_index >::iterator insert_place = boost::range::lower_bound( times, l.frame_number() );
                if ( insert_place == times.end() || *insert_place != l.frame_number() )
                    times.insert( insert_place, l.frame_number() );
            }
        }
    }

};

const samplepos::Scalar DriftSection::position_unity = 1E-6f * si::meter;
const samplepos::Scalar DriftSection::uncertainty_unity = 1E-8f * si::meter;

std::auto_ptr< output::OutputSource > create_drift_correction() {
    return std::auto_ptr< output::OutputSource >( new output::OutputBuilder< NonlinearDriftEstimator::Config, NonlinearDriftEstimator >() );
}

}
}
