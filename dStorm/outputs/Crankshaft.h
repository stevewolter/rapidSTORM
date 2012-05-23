#ifndef DSTORM_CRANKSHAFT_H
#define DSTORM_CRANKSHAFT_H

#include "../output/Output.h"
#include <list>
#include <boost/noncopyable.hpp>

namespace dStorm {
namespace outputs {

class Crankshaft
: public output::Output,
  private boost::noncopyable 
{
    boost::optional< simparm::NodeHandle > current_ui;
    class Source;

    class Clutch;
    typedef std::list<Clutch> Clutches;
    Clutches clutches;
    int id;

    void _add( Output *tm, bool important, bool manage );
  protected:
    void prepare_destruction_();
    void store_results_( bool success );
    void run_finished_( const RunFinished& );
    void attach_ui_( simparm::NodeHandle at );

  public:
    enum Type { Yield, State };

    Crankshaft ();
    virtual ~Crankshaft ();

    void add( Output& transmission, Type type = Yield )
        { _add( &transmission, (type == Yield), false ); }
    void add( Output* tm, Type type = Yield )
        { _add( tm, (type == Yield), true ); }
    void add( std::auto_ptr<Output> tm, Type type = Yield )
        { if (tm.get() != NULL) _add( tm.release(), (type == Yield), true ); }

    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement&);
    void receiveLocalizations(const EngineResult&);

    bool empty() const { return clutches.empty(); }
    void check_for_duplicate_filenames
        (std::set<std::string>& present_filenames);
};
}
}

#endif
