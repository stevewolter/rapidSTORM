#include "engine/TraceReducer.h"
#include "statistics/Variance.h"

namespace dStorm {

class AveragingReducer : public TraceReducer {
  public:
    void reduce_trace_to_localization 
        (const Trace& from, Localization *to,
         const Eigen::Vector2d& shift) 
    {
        Variance x_position, y_position;
        int last_image_number = 0;
        double total_amplitude = 0;
        double max_parab = 0;

        for ( data_cpp::Vector<Localization>::const_iterator
                i = from.begin(); i != from.end(); i++)
        {
            x_position.addValue( i->x() );
            y_position.addValue( i->y() );
            last_image_number = 
                std::max( last_image_number, i->getImageNumber() );
            total_amplitude += i->getStrength();
            max_parab = std::max(i->parabolicity(), max_parab);
        }

        new(to) Localization( 
            x_position.mean() - shift.x(),
            y_position.mean() - shift.y(),
            last_image_number, total_amplitude, &from, max_parab );
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
: simparm::Node(c), simparm::Object(c),
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
