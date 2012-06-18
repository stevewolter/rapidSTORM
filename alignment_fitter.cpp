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

typedef std::list<Eigen::Vector2f, Eigen::aligned_allocator<Eigen::Vector2f> > PositionList;
typedef std::map< int, boost::array< PositionList, 2 > > ImageMap;
ImageMap images;

struct parameters {
    double sigma;
};

double badness (const gsl_vector * x, void * params) {
    Eigen::Matrix2f rotation = Eigen::Matrix2f::Identity();
    Eigen::Vector2f translation = Eigen::Vector2f::Zero();
    if ( x->size >= 2 ) 
        for (int r = 0; r < 2; ++r)
            translation[r] = gsl_vector_get(x, r);
    if ( x->size >= 4 )
        for (int r = 0; r < 2; ++r)
            rotation(r,r) = gsl_vector_get(x, 2+r);
    if ( x->size >= 6 )
        for (int r = 0; r < 2; ++r)
            rotation(r,1-r) = gsl_vector_get(x, 4+r);

    double score = 0;
    for ( ImageMap::const_iterator i = images.begin(); i != images.end(); ++i ) {
        for ( PositionList::const_iterator b = i->second[1].begin(); b != i->second[1].end(); ++b ) {
            Eigen::Vector2f t = rotation * *b + translation;
            for ( PositionList::const_iterator a = i->second[0].begin(); a != i->second[0].end(); ++a ) {
                score += - exp( - (*a-t).squaredNorm() / static_cast<parameters*>(params)->sigma );
            }
        }
    }
    return score;
}

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
    simparm::Entry<double>* entries[6] = { &shift_x, &shift_y, &scale_x, &scale_y, &shear_x, &shear_y };

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

        parameters params;
        params.sigma = sigma();

        cur_step = 0;
        cur_volume = 1;
        cur_step.show();
        cur_volume.show();
        for (int pc = (pre_fit()) ? 2 : 6; pc <= 6; pc += 2) {
            cur_step = pc/2;
            gsl_vector* x = gsl_vector_alloc(pc), *first_step = gsl_vector_alloc(pc);
            gsl_multimin_fminimizer* m = gsl_multimin_fminimizer_alloc(gsl_multimin_fminimizer_nmsimplex2, pc);
            gsl_multimin_function function;
            function.f = &badness;
            function.n = pc;
            function.params = &params;

            for (int i = 0; i < pc; ++i) {
                gsl_vector_set(x, i, (*entries[i])());
                gsl_vector_set(first_step, i, (i < 2) ? 1 : 1E-3 );
            }
            gsl_multimin_fminimizer_set(m, &function, x, first_step);

            while ( gsl_multimin_fminimizer_iterate(m) == GSL_SUCCESS )  {
                if ( gsl_multimin_fminimizer_size(m) < target_volume() ) break;
                if ( ! continue_running ) return;
                cur_volume = gsl_multimin_fminimizer_size(m);
            }
            for (int i = 0; i < pc; ++i) *entries[i] = gsl_vector_get(gsl_multimin_fminimizer_x(m), i);
        }
        cur_step.hide();
        cur_volume.hide();

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

std::auto_ptr< dStorm::JobConfig > make_alignment_fitter_config() {
    return std::auto_ptr< dStorm::JobConfig >( new AlignmentFitterConfig() );
}
