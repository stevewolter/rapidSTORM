#define VERBOSE
#include "debug.h"
#include "alignment_fitter.h"
#include <dStorm/Job.h>

#include <dStorm/localization_file/reader.h>
#include <boost/variant/get.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <gsl/gsl_multimin.h>
#include <boost/units/Eigen/Array>
#include <boost/lexical_cast.hpp>
#include <iomanip>
#include <simparm/text_stream/RootNode.h>
#include <simparm/Group.h>
#include <simparm/TriggerEntry.h>

#include <nonlinfit/Evaluation.h>
#include <nonlinfit/levmar/Fitter.hpp>
#include <nonlinfit/terminators/StepLimit.h>

typedef std::list<Eigen::Vector2f, Eigen::aligned_allocator<Eigen::Vector2f> > PositionList;
typedef std::map< int, boost::array< PositionList, 2 > > ImageMap;

struct parameters {
    double sigma;
};

class FitBadness 
{
    Eigen::Matrix2f rotation;
    Eigen::Vector2f translation;
    const ImageMap& images;
    double sigma;

public:
    FitBadness( const ImageMap& images, double sigma )
        : rotation( Eigen::Matrix2f::Identity() ), translation( Eigen::Vector2f::Zero() ), images(images), sigma(sigma) {}

    typedef nonlinfit::Evaluation< float, 6 > Result;
    typedef Result::Vector Position;
    typedef Result Derivatives;

    int variable_count() const { return 6; }

    bool evaluate( Result& r ) {
        r.set_zero();

        for ( ImageMap::const_iterator i = images.begin(); i != images.end(); ++i ) {
            for ( PositionList::const_iterator b = i->second[1].begin(); b != i->second[1].end(); ++b ) {
                Eigen::Vector2f t = rotation * *b + translation;
                for ( PositionList::const_iterator a = i->second[0].begin(); a != i->second[0].end(); ++a ) {
                    Eigen::Vector2f diff = *a-t;
                    Eigen::Matrix<float,1,6> jacobi_row;
                    jacobi_row.head<2>() = *b * diff.x(); 
                    jacobi_row.segment<2>(2) = *b * diff.y(); 
                    jacobi_row.tail<2>() = diff;
                    float v = - exp( - diff.squaredNorm() / sigma );
                    jacobi_row *= v / sigma * 2;
                    r.value += v;
                    r.gradient += jacobi_row;
                    r.hessian += jacobi_row.transpose() * jacobi_row;
                }
            }
        }
        
        return true;
    }

    void get_position( Result::Vector& v ) const {
        v.head<2>() = rotation.row(0);
        v.segment<2>(2) = rotation.row(1);
        v.tail<2>() = translation;
    }
    void set_position( const Result::Vector& v ) {
        rotation.row(0) = v.head<2>();
        rotation.row(1) = v.segment<2>(2);
        translation = v.tail<2>();
    }
};

class AlignmentFitter
{
protected:
    simparm::FileEntry file1, file2, output;
    simparm::Entry<double> sigma, shift_x, shift_y,
         scale_x, scale_y, shear_x, shear_y, target_volume;
    simparm::Entry<bool> pre_fit;
    simparm::Entry<long> image_count;

public:
    AlignmentFitter();
    AlignmentFitter* clone() const { return new AlignmentFitter(*this); }
    simparm::NodeHandle attach_ui( simparm::NodeHandle );
};

class AlignmentFitterConfig
: public dStorm::JobConfig, private AlignmentFitter
{
    simparm::Object name_object;
public:
    AlignmentFitterConfig();
    AlignmentFitterConfig* clone() const { return new AlignmentFitterConfig(*this); }
    simparm::NodeHandle attach_ui( simparm::NodeHandle );
    void attach_children_ui( simparm::NodeHandle );

    std::auto_ptr<dStorm::Job> make_job();
};

class AlignmentFitterJob 
: public dStorm::Job, private AlignmentFitter {
    static int ident;
    simparm::Object name_object;
    simparm::Entry<double> cur_step, cur_volume;
    simparm::TriggerEntry close;
    simparm::BaseAttribute::ConnectionStore listening;

    boost::mutex running_mutex;
    boost::condition can_stop_running;
    bool continue_running;

    void close_trigger() {
        if ( close.triggered() ) { close.untrigger(); stop(); }
    }

public:
    AlignmentFitterJob( const AlignmentFitter& );
    void run();
    void stop() { continue_running = false; can_stop_running.notify_all(); }
    simparm::NodeHandle attach_ui( simparm::NodeHandle );
};

int AlignmentFitterJob::ident = 1;

std::auto_ptr<dStorm::Job> AlignmentFitterConfig::make_job() { return std::auto_ptr<dStorm::Job>( new AlignmentFitterJob(*this) ); }

AlignmentFitter::AlignmentFitter()
: file1("File1", "File 1"), file2("File2", "File 2"), output("OutputFile", "Output file"),
  sigma("Sigma", "Sigma", 1), 
  shift_x("ShiftX", "Shift X in mum"),
  shift_y("ShiftY", "Shift Y in mum"),
  scale_x("ScaleX", "Scale factor X", 1),
  scale_y("ScaleY", "Scale factor Y", 1),
  shear_x("ShearX", "Shear factor in X", 0),
  shear_y("ShearY", "Shear factor Y", 0),
  target_volume("TargetVolume", "Target estimation volume", 1E-3),
  pre_fit("PreFit", "Pre-fit with simpler models", true),
  image_count("ImageCount", "Number of images to use", 100)
{}

AlignmentFitterConfig::AlignmentFitterConfig()
: name_object("AlignmentFitter", "Fit alignment") {}

AlignmentFitterJob::AlignmentFitterJob( const AlignmentFitter& a )
: AlignmentFitter(a),
  name_object("AlignmentFitter" + boost::lexical_cast<std::string>(ident),
              "Alignment fitting " + boost::lexical_cast<std::string>(ident)),
  cur_step("CurrentStep", "Estimation step"),
  cur_volume("CurrentVolume", "Current estimation volume"),
  close("CloseJob", "Close job"),
  continue_running(true)
{
    ++ident;
}

simparm::NodeHandle AlignmentFitterConfig::attach_ui( simparm::NodeHandle at ) {
    simparm::NodeHandle r = name_object.attach_ui(at);
    attach_children_ui( r );
    return r;
}

void AlignmentFitterConfig::attach_children_ui( simparm::NodeHandle at ) {
    AlignmentFitter::attach_ui(at);
}

simparm::NodeHandle AlignmentFitter::attach_ui( simparm::NodeHandle r ) {
    file1.attach_ui( r );
    file2.attach_ui( r );
    output.attach_ui( r );
    sigma.attach_ui( r );
    shift_x.attach_ui( r );
    shift_y.attach_ui( r );
    scale_x.attach_ui( r );
    scale_y.attach_ui( r );
    shear_x.attach_ui( r );
    shear_y.attach_ui( r );
    pre_fit.attach_ui( r );
    target_volume.attach_ui( r );
    image_count.attach_ui( r );
    return r;
}

simparm::NodeHandle AlignmentFitterJob::attach_ui( simparm::NodeHandle at ) {
    simparm::NodeHandle r = name_object.attach_ui(at);
    AlignmentFitter::attach_ui(r);
    cur_step.attach_ui( r );
    cur_volume.attach_ui( r );
    close.attach_ui( r );

    listening = close.value.notify_on_value_change(
        boost::bind( &AlignmentFitterJob::close_trigger, this ) );

    return r;
}

void AlignmentFitterJob::run() {
    FitBadness::Result::Vector start_position;
    simparm::Entry<double>* entries[6] = { &shift_x, &shift_y, &scale_x, &scale_y, &shear_x, &shear_y };
    for (int i = 0; i < 6; ++i)
        start_position[i] = entries[i]->value();
    ImageMap images;

    try {
        dStorm::input::Traits<dStorm::localization::Record> context;
        typedef dStorm::localization_file::Reader::Source Source;
        std::auto_ptr<Source> file_source[2];
        file_source[0] = dStorm::localization_file::Reader::ChainLink::read_file( file1, context );
        file_source[1] = dStorm::localization_file::Reader::ChainLink::read_file( file2, context );

        for (int m = 0; m < 2; ++m)
            for ( dStorm::input::Source<dStorm::localization::Record>::iterator
                    i = file_source[m]->begin(); i != file_source[m]->end(); ++i ) {
                const dStorm::Localization* l = boost::get<dStorm::Localization>(&*i);
                if ( l && l->frame_number().value() >= image_count.value() ) continue;
                if ( l ) { images[ l->frame_number().value() ][m].push_back( 
                            boost::units::value( l->position() ).head<2>() * 1E6 ); }
            }

        FitBadness badness( images, sigma() );
        badness.set_position( start_position );
        nonlinfit::levmar::Config config;
        nonlinfit::levmar::Fitter fitter(config);
        fitter.fit( badness, badness, nonlinfit::terminators::StepLimit(100) );

        FitBadness::Result::Vector final_position;
        badness.get_position( final_position );
        for (int i = 0; i < 6; ++i)
            entries[i]->value = final_position[i];

        if ( output ) {
            output.get_output_stream() << scale_x() << " " << shear_x() << " " << shift_x() * 1E-6 << "\n" 
                                            << shear_y() << " " << scale_y() << " " << shift_y() * 1E-6 << "\n" 
                                            << "0 0 1\n";
            output.close_output_stream();
        }

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }

    boost::mutex::scoped_lock l( running_mutex );
    while ( continue_running )
        can_stop_running.wait(l);
}

void check_fit_badness_derivatives() {
    ImageMap images;
    images[ 0 ][ 0 ].push_back( Eigen::Vector2f::Constant( 2 ) + Eigen::Vector2f::UnitX() );
    images[ 0 ][ 1 ].push_back( Eigen::Vector2f::Constant( 4 ) + Eigen::Vector2f::UnitY() );
    FitBadness badness( images, 25 );
    FitBadness::Result::Vector orig;
    FitBadness::Result exact;
    const double delta = 1E-4;

    for (int i = 0; i < 6; ++i)
        orig[i] = 1.0 + (i-3) * 0.05;

    badness.set_position( orig );
    badness.evaluate( exact );

    for (int i = 0; i < 6; ++i) {
        FitBadness::Result::Vector shifted = orig;
        FitBadness::Result shifted_result;
        shifted[i] += delta;
        badness.set_position( shifted );
        badness.evaluate( shifted_result );

        double epsilon = shifted_result.value - exact.value;
        BOOST_CHECK_CLOSE( epsilon / delta, exact.gradient[i], 1 );
    }
}

boost::unit_test::test_suite* register_alignment_fitter_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "alignment_fitter" );
    rv->add( BOOST_TEST_CASE( &check_fit_badness_derivatives ) );
    return rv;
}


std::auto_ptr< dStorm::JobConfig > make_alignment_fitter_config() {
    return std::auto_ptr< dStorm::JobConfig >( new AlignmentFitterConfig() );
}
