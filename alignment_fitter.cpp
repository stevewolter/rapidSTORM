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
#include <simparm/ProgressEntry.h>
#include <fstream>

typedef std::list<Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d> > PositionList;
typedef std::map< int, boost::array< PositionList, 2 > > ImageMap;

class ProgressStepLimit {
    int steps, total_steps;
    boost::shared_ptr<simparm::ProgressEntry> progress;
public:
    ProgressStepLimit( int step_count, std::string stage ) 
        : steps(0), total_steps( step_count ), progress( new simparm::ProgressEntry("FitProgress" + stage, "Fit progress") ) {}
    void matrix_is_unsolvable() {}
    template <typename Position>
    void improved( const Position&, const Position& ) { ++steps; progress->value = steps * 1.0 / total_steps; }
    void failed_to_improve( bool ) { ++steps; progress->value = steps * 1.0 / total_steps; }
    bool should_continue_fitting() const { return steps < total_steps; }
    void attach_ui( simparm::NodeHandle h ) { progress->attach_ui(h); }
};

struct BadnessFunction 
{
    Eigen::Matrix2d rotation;
    Eigen::Vector2d translation;
    const ImageMap& images;
    double sigma;

    BadnessFunction( const ImageMap& images, double sigma )
        : rotation( Eigen::Matrix2d::Identity() ), translation( Eigen::Vector2d::Zero() ), images(images), sigma(sigma) {}

    double evaluate( gsl_vector * nabla_f ) const {
        int count = 0;
        double value = 0;
        if ( nabla_f ) gsl_vector_set_zero( nabla_f );
        for ( ImageMap::const_iterator i = images.begin(); i != images.end(); ++i ) {
            for ( PositionList::const_iterator b = i->second[1].begin(); b != i->second[1].end(); ++b ) {
                Eigen::Vector2d t = rotation * *b + translation;
                for ( PositionList::const_iterator a = i->second[0].begin(); a != i->second[0].end(); ++a ) {
                    Eigen::Vector2d diff = *a-t;

                    double v = - exp( - diff.squaredNorm() / sigma ), pf = v / sigma * 2;

                    if ( nabla_f ) {
                        *gsl_vector_ptr( nabla_f, 0 ) += diff.x() * pf;
                        *gsl_vector_ptr( nabla_f, 1 ) += diff.y() * pf;
                        if ( nabla_f->size > 2 ) {
                            *gsl_vector_ptr( nabla_f, 2 ) += b->x() * diff.x() * pf;
                            *gsl_vector_ptr( nabla_f, 3 ) += b->y() * diff.y() * pf;
                        }
                        if ( nabla_f->size > 4 ) {
                            *gsl_vector_ptr( nabla_f, 4 ) += b->y() * diff.x() * pf;
                            *gsl_vector_ptr( nabla_f, 5 ) += b->x() * diff.y() * pf;
                        }
                    }

                    ++count;
                    value += v;
                }
            }
        }
        return value;
    }

    void set_parameters( const gsl_vector *x ) {
        translation.x() = gsl_vector_get(x, 0);
        translation.y() = gsl_vector_get(x, 1);
        if ( x->size > 2 ) {
            rotation(0,0) = gsl_vector_get(x, 2);
            rotation(1,1) = gsl_vector_get(x, 3);
        }
        if ( x->size > 4 ) {
            rotation(0,1) = gsl_vector_get(x, 4);
            rotation(1,0) = gsl_vector_get(x, 5);
        }
    }
};

double badness_f( const gsl_vector * x, void * params) {
    BadnessFunction& b = *(BadnessFunction*)params;
    b.set_parameters( x );
    return b.evaluate(NULL);
}

void badness_df( const gsl_vector * x, void * params, gsl_vector*g ) {
    BadnessFunction& b = *(BadnessFunction*)params;
    b.set_parameters( x );
    b.evaluate(g);
}

void badness_fdf( const gsl_vector * x, void * params, double* v, gsl_vector *g ) {
    BadnessFunction& b = *(BadnessFunction*)params;
    b.set_parameters( x );
    *v = b.evaluate(g);
}

class AlignmentFitter
{
protected:
    simparm::FileEntry file1, file2, output;
    simparm::Entry<double> sigma, shift_x, shift_y,
         scale_x, scale_y, shear_x, shear_y;
    simparm::Entry<long> image_count;
    simparm::Entry<int> fit_steps;
    simparm::Entry<double> initial_length;

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
    void close_when_finished() override {}

    std::auto_ptr<dStorm::Job> make_job();
};

class AlignmentFitterJob 
: public dStorm::Job, private AlignmentFitter {
    static int ident;
    simparm::Object name_object;
    simparm::ProgressEntry progress;
    simparm::TriggerEntry close;
    simparm::BaseAttribute::ConnectionStore listening;

    boost::mutex running_mutex;
    boost::condition can_stop_running;
    bool continue_running;

    ImageMap images;

    void close_trigger() {
        if ( close.triggered() ) { close.untrigger(); stop(); }
    }
    void improve_position( Eigen::Matrix<double,6,1>&, int variables );

public:
    AlignmentFitterJob( const AlignmentFitter& );
    void run();
    void stop() { continue_running = false; can_stop_running.notify_all(); }
    simparm::NodeHandle attach_ui( simparm::NodeHandle );
    void close_when_finished() override {}
};

int AlignmentFitterJob::ident = 1;

std::auto_ptr<dStorm::Job> AlignmentFitterConfig::make_job() { return std::auto_ptr<dStorm::Job>( new AlignmentFitterJob(*this) ); }

AlignmentFitter::AlignmentFitter()
: file1("File1", "File 1", ""), file2("File2", "File 2", ""), output("OutputFile", "Output file", ""),
  sigma("Sigma", "Sigma", 1), 
  shift_x("ShiftX", "Shift X in mum", 0),
  shift_y("ShiftY", "Shift Y in mum", 0),
  scale_x("ScaleX", "Scale factor X", 1),
  scale_y("ScaleY", "Scale factor Y", 1),
  shear_x("ShearX", "Shear factor in X", 0),
  shear_y("ShearY", "Shear factor Y", 0),
  image_count("ImageCount", "Number of images to use", 10000),
  fit_steps("FittingSteps", "Fit iterations", 100),
  initial_length("InitialStepLength", "Initial step length", 1)
{}

AlignmentFitterConfig::AlignmentFitterConfig()
: name_object("AlignmentFitter", "Fit alignment") {}

AlignmentFitterJob::AlignmentFitterJob( const AlignmentFitter& a )
: AlignmentFitter(a),
  name_object("AlignmentFitter" + boost::lexical_cast<std::string>(ident),
              "Alignment fitting " + boost::lexical_cast<std::string>(ident)),
  progress("FitProgress", "Fit progress"),
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
    image_count.attach_ui( r );
    fit_steps.attach_ui( r );
    initial_length.attach_ui( r );
    return r;
}

simparm::NodeHandle AlignmentFitterJob::attach_ui( simparm::NodeHandle at ) {
    simparm::NodeHandle r = name_object.attach_ui(at);
    AlignmentFitter::attach_ui(r);
    progress.attach_ui( r );
    close.attach_ui( r );

    listening = close.value.notify_on_value_change(
        boost::bind( &AlignmentFitterJob::close_trigger, this ) );

    return r;
}

void AlignmentFitterJob::improve_position( Eigen::Matrix<double,6,1>& p, int VariableCount ) {
    BadnessFunction badness( images, sigma() );

    gsl_multimin_fdfminimizer * minimizer = 
        gsl_multimin_fdfminimizer_alloc( gsl_multimin_fdfminimizer_conjugate_fr, VariableCount );

    gsl_vector *start_position = gsl_vector_alloc( VariableCount );
    for (int i = 0; i < VariableCount; ++i) gsl_vector_set( start_position, i, p[i] );

    gsl_multimin_function_fdf function;
    function.f = &badness_f;
    function.df = &badness_df;
    function.fdf = &badness_fdf;
    function.n = VariableCount;
    function.params = &badness;

    int success = gsl_multimin_fdfminimizer_set( minimizer, &function, start_position, initial_length(), 0.1 );
    if ( success != GSL_SUCCESS )
        throw std::runtime_error("Minimizer could not be set");

    for (int i = 0; i < fit_steps(); ++i) {
        progress.value = i * 1.0 / fit_steps();
        success = gsl_multimin_fdfminimizer_iterate( minimizer );
        if ( success == GSL_SUCCESS )
            continue;
        else if ( success == GSL_ENOPROG )
            break;
        else
            throw std::runtime_error("Minimizer could not be iterated");
    }
    progress.value = 1.0;

    gsl_vector* end_position = gsl_multimin_fdfminimizer_x( minimizer );
    for (int i = 0; i < VariableCount; ++i) p[i] = gsl_vector_get( end_position, i );

    shift_x = p[0];
    shift_y = p[1];
    scale_x = p[2];
    scale_y = p[3];
    shear_x = p[4];
    shear_y = p[5];

    gsl_vector_free( start_position );
    gsl_multimin_fdfminimizer_free( minimizer );
}

void AlignmentFitterJob::run() {
    Eigen::Matrix<double,6,1> position;
    simparm::Entry<double>* entries[6] = { &shift_x, &shift_y, &scale_x, &scale_y, &shear_x, &shear_y };
    for (int i = 0; i < 6; ++i)
        position[i] = entries[i]->value();

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
                            boost::units::value( l->position() ).head<2>().cast<double>() * 1E6 ); }
            }

        improve_position( position, 6 );

        if ( output ) {
            std::ofstream output_stream( output().c_str() );
            output_stream << scale_x() << " " << shear_x() << " " << shift_x() * 1E-6 << "\n" 
                                            << shear_y() << " " << scale_y() << " " << shift_y() * 1E-6 << "\n" 
                                            << "0 0 1\n";
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
    images[ 0 ][ 0 ].push_back( Eigen::Vector2d::Constant( 2 ) + Eigen::Vector2d::UnitX() );
    images[ 0 ][ 1 ].push_back( Eigen::Vector2d::Constant( 4 ) + Eigen::Vector2d::UnitY() );

    gsl_vector* orig = gsl_vector_alloc( 6 ), *shifted = gsl_vector_alloc(6), *gradient = gsl_vector_alloc(6);
    BadnessFunction badness( images, 25 );
    const double delta = 1E-4;

    for (int i = 0; i < 6; ++i)
        gsl_vector_set( orig, i, 1.0 + (i-3) * 0.05 );

    badness.set_parameters( orig );
    double exact = badness.evaluate( gradient );

    for (int i = 0; i < 6; ++i) {
        gsl_vector_memcpy( shifted, orig );
        *gsl_vector_ptr( shifted, i ) += delta;
        badness.set_parameters( shifted );
        double shifted_result = badness.evaluate( NULL );

        double epsilon = shifted_result - exact;
        BOOST_CHECK_CLOSE( epsilon / delta, gsl_vector_get( gradient, i ), 1 );
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
