#include "TraceReducer.h"
#include <dStorm/unit_matrix_operators.h>
#include <dStorm/units/amplitude.h>

namespace dStorm {
namespace output {

class AveragingReducer : public TraceReducer {
  public:
    void reduce_trace_to_localization 
        (const Trace& from, Localization *to,
         const Localization::Position& shift) 
    {
        Localization::Position position = Localization::Position::Zero();

        int n = 0;
        frame_index last_image_number = 0;
        amplitude total_amplitude = 0;

        for ( data_cpp::Vector<Localization>::const_iterator
                i = from.begin(); i != from.end(); i++)
        {
            position += i->position();
            n += 1;
            last_image_number = 
                std::max( last_image_number, i->getImageNumber() );
            total_amplitude += i->strength();
        }

        if ( n == 0 ) n = 1;

        new(to) Localization( position / n - shift,
                              total_amplitude );
        to->setImageNumber( last_image_number );
        to->set_source_trace( from );
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