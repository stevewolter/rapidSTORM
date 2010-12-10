#include "TraceReducer.h"
#include <Variance.h>

namespace dStorm {

class AveragingReducer : public TraceReducer {
  public:
    void reduce_trace_to_localization 
        (const Trace& from, Localization *to) throw() 
    {
        Variance x_position, y_position;
        int last_image_number = 0;
        double total_amplitude = 0;

        for ( data_cpp::Vector<Localization>::const_iterator i = from.begin();
            i != from.end(); i++)
        {
            x_position.addValue( i->x() );
            y_position.addValue( i->y() );
            last_image_number = 
                std::max( last_image_number, i->getImageNumber() );
            total_amplitude += i->getStrength();
        }

        new(to) Localization( x_position.mean(), y_position.mean(),
                                last_image_number, total_amplitude, &from );
    }
};

class BasicTraceReducerConfig : public TraceReducerConfig {
  public:
    BasicTraceReducerConfig() throw() 
        : simparm::ConfigObject("TraceReducerMaker", "") ,
          TraceReducerConfig()
    {}
    BasicTraceReducerConfig* clone() const
        { return new BasicTraceReducerConfig(); }

    virtual std::auto_ptr<TraceReducer> make_trace_reducer() throw()
        { return std::auto_ptr<TraceReducer>( new AveragingReducer() ); }
};

static std::auto_ptr<TraceReducerConfig> mkBasicTraceReducerConfig() throw() {
    return std::auto_ptr<TraceReducerConfig>( new BasicTraceReducerConfig() );
}

std::auto_ptr<TraceReducerConfig> (*TraceReducerConfig::factory)() throw()
    = mkBasicTraceReducerConfig;

}
