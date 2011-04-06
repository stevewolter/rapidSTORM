#define DSTORM_SIGMAGUESSER_CPP
#include "debug.h"
#include <fit++/Exponential2D.hh>
#include "Output.h"
#include <dStorm/engine/Input.h>
#include <dStorm/engine/Image.h>
#include <dStorm/output/OutputSource.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/image/slice.h>
#include <limits>
#include <boost/units/io.hpp>

#include <cassert>
#include <math.h>

#include "studentPinv.h"
#include "Fitter.h"

#include <sstream>
#include <simparm/Message.hh>

using namespace std;
using namespace fitpp;

namespace dStorm {
namespace sigma_guesser {

Output::Output(const Config& c)
: OutputObject("SigmaGuesser", "Standard deviation estimator"),
  engine(NULL),
  delta( c.delta_sigma() ),
  status("Status", "PSF size estimation")
{
    nextCheck = 23;
    maximum_area = c.maximum_estimation_size();
    fitter.reset( new Fitter(c) );

    status = "Waiting for initialization";
    status.editable = false;

    push_back( status );
}
Output::~Output() {}

output::Output::AdditionalData
Output::announceStormSize(const Announcement& a) 
{
    engine = a.engine;
    traits = a.input_image_traits;
    if ( traits.get() ) {
        use_traits( *traits );
        deleteAllResults();
    }
    return AdditionalData();
}

void Output::use_traits( const input::Traits<engine::Image>& traits )
{
    fitter->useConfig( traits );
    stringstream ss;
    ss << "Trying (" << traits.psf_size().x() * 2.35f << ", " << traits.psf_size().y() * 2.35f << ")";
    status = ss.str();
}

output::Output::Result
Output::receiveLocalizations(const EngineResult& er)

{
    ost::MutexLock lock(mutex);
    DEBUG("Adding fits");

    if ( ! traits.get() ) return RemoveThisOutput;
    if ( ! er.source.is_valid() ) return KeepRunning;
    used_area += er.source.size() / camera::pixel;
    if ( used_area > maximum_area ) {
        DEBUG("Reached size limit");
        simparm::Message m( "Maximum PSF size estimation area reached",
            "The number of pixels used in PSF size estimation has reached its limit before "
            "it converged. "
            "PSF size estimation is aborted and the current estimation will be used to compute the rest of this job. "
            "The PSF currently used in computation is probably wrong and it should be considered "
            "to give it explicitely.",
            simparm::Message::Warning );
        send(m);
        nextCheck = numeric_limits<int>::max();
        return RemoveThisOutput;
    }

    for (int i = 0; i < er.number; i++)
        meanAmplitude.addValue(er.first[i].amplitude());

    DEBUG("Updated mean amplitude");
    int old_n = n;
    double sigmas[4];
    for (int i = 0; i < er.number; i++) {
        if (meanAmplitude.mean() < er.first[i].amplitude()) {
            DEBUG("Fitting " << i);
            assert( er.source.depth_in_pixels() == 1 );
            bool good = fitter->fit(er.source.slice(2, 0 * camera::pixel), er.first[i], sigmas);
            DEBUG("Fitted " << i);
            if (good) {
                for (int i = 0; i < 3; i++) {
                    DEBUG("Guess for " << i << " is " << sigmas[i+((i==2)?1:0)]);
                    data[i].addValue(sigmas[i+((i==2)?1:0)]);
                }
                n++;
            } 
        }
    }
    discarded += n - old_n;
    DEBUG("Fitted data");

    DEBUG("Have " << n << " of " << nextCheck);
    if (n >= nextCheck) {
        Output::Result r = check();
        nextCheck = 3*nextCheck/2;
        return r;
    } else
        return KeepRunning;
}

output::Output::Result
Output::check() {
    DEBUG("Checking result");
    std::auto_ptr< input::Traits< engine::Image > > new_traits;
    int converged = 0, failed = 0;
    double t_term = studentPinv95(n-1);
    for (int i = 0; i < 3; i++) {
        double confidence = t_term * sqrt( data[i].variance() / n );
        double confInterval[2];
        double curVal = data[i].mean();
        confInterval[0] = curVal - confidence, confInterval[1] = curVal + confidence;

        if ( (confInterval[0] >= accept[i][0] &&
              confInterval[1] <= accept[i][1]) ) 
        {
            DEBUG("Sigma " << i << " converged at "
                     << (accept[i][0] + accept[i][1])/2);
            converged++;
        } else if ( 
             confInterval[0] > decline[i][1] ||
             confInterval[1] < decline[i][0] )
        {
            double newValue = curVal;
            if ( new_traits.get() == NULL )
                new_traits.reset( traits->clone() );

            DEBUG("Sigma " << i << " changed to " << newValue);
            if ( i == 0 )
                new_traits->psf_size().x() = float(newValue) * camera::pixel / traits->plane(0).resolution[0]->in_dpm();
            else if ( i == 1 )
                new_traits->psf_size().y() = float(newValue) * camera::pixel / traits->plane(0).resolution[1]->in_dpm();
            else
                throw std::logic_error("Correlation changed even though it was fixed to zero. Confused.");
                    
            failed++;
        } else {
            DEBUG("Sigma " << i << " undecided at " << curVal);
        }
    }
    DEBUG("Checked result");
    if (failed && engine) {
        stringstream ss;
        ss << "Trying (" << traits->psf_size().x() * 2.35f << ", " << traits->psf_size().y() * 2.35f << ")";
        status = ss.str();

        use_traits( *new_traits );
        engine->change_input_traits( std::auto_ptr< input::BaseTraits >(new_traits.release()) );
        engine->restart();
        engine = NULL;
        nextCheck = n+1;
        DEBUG("Significant difference");
        return KeepRunning;
    } else if (converged == 3) {
        nextCheck = numeric_limits<int>::max();
        DEBUG("Insignificant difference");
        return RemoveThisOutput;
    } else {
        nextCheck = (3*n)/2;
        DEBUG("No significance. Next check at " << nextCheck);
        return KeepRunning;
    }
}

void Output::deleteAllResults() {
    ost::MutexLock lock(mutex);
    DEBUG("Resetting entries");
    double sigmas[3] = {
        traits->psf_size().x() * traits->plane(0).resolution[0]->in_dpm() / camera::pixel,
        traits->psf_size().y() * traits->plane(0).resolution[1]->in_dpm() / camera::pixel,
        0 };
    for (int i = 0; i < 3; i++) {
        data[i].reset();
        accept[i][0] = sigmas[i] - delta;
        accept[i][1] = sigmas[i] + delta;
        decline[i][0] = sigmas[i] - 0.5 * delta;
        decline[i][1] = sigmas[i] + 0.5 * delta;
    }
    n = discarded = 0;
    meanAmplitude.reset();
    used_area = 0 * camera::pixel * camera::pixel;

    DEBUG("Resetting fitter");
    fitter->useConfig(*traits);
    DEBUG("Deleted all results");
}

}

namespace output {

template <>
std::auto_ptr<OutputSource> make_output_source<sigma_guesser::Output>()
{
    return std::auto_ptr<OutputSource>( new OutputBuilder<sigma_guesser::Output>() );
}


}
}
