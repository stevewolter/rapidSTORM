#include "TraceReducer.h"
#include <dStorm/unit_matrix_operators.h>
#include <dStorm/units/amplitude.h>
#include <boost/units/systems/camera/intensity.hpp>

namespace dStorm {
namespace output {

class AveragingReducer : public TraceReducer {
  public:
    AveragingReducer* clone() const { return new AveragingReducer(); }
    void reduce_trace_to_localization 
        (const Input& start, const Input& end, Output to,
         const samplepos& shift) 
    {
        samplepos position = samplepos::Zero();

        int n = 0;
        frame_index last_image_number = 0;
        Localization::Amplitude::Type total_amplitude;
        total_amplitude = 0 * camera::ad_count;

        for ( Input i = start; i != end; ++i )
        {
            position += i->position();
            n += 1;
            last_image_number = 
                std::max( last_image_number, i->frame_number() );
            total_amplitude += i->amplitude.value();
        }

        if ( n == 0 ) n = 1;

        Localization r( *start );
        r.position = position / n - shift;
        r.amplitude = total_amplitude;
        r.frame_number = last_image_number;
        r.children = std::vector<Localization>();
        r.children->reserve( n );
        std::copy( start, end, back_inserter( *r.children ) );
        *to = r;
    }
};

class TraceReducer::Config::Implementation {
  public:
    Implementation() 
    {}
    Implementation* clone() const { return new Implementation(*this); }

    virtual std::auto_ptr<TraceReducer> make_trace_reducer()
        { return std::auto_ptr<TraceReducer>( new AveragingReducer() ); }
};

TraceReducer::Config::Config() 
: simparm::Object("TraceReducerMaker", ""),
  implementation( new Implementation() )
{}

TraceReducer::Config::Config(const Config& c) 
: simparm::Object(c),
  implementation( c.implementation->clone() )
{}

TraceReducer::Config::~Config()
{}

std::auto_ptr<TraceReducer> 
TraceReducer::Config::make_trace_reducer() const 
{
    return std::auto_ptr<TraceReducer>(new AveragingReducer());
}

}
}