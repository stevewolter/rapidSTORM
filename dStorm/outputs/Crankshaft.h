#ifndef DSTORM_CRANKSHAFT_H
#define DSTORM_CRANKSHAFT_H

#include "../output/Output.h"
#include <list>

namespace dStorm {
namespace outputs {

class Crankshaft : public output::OutputObject {
    class Source;

    class Clutch;
    typedef std::list<Clutch> Clutches;
    Clutches clutches;
    int id;

    void _add( Output *tm, bool important, bool manage, bool front = false );
    void prepare_destruction_();
    void run_finished_( const RunFinished& );

    /** No copy constructor defined. */
    Crankshaft( const Crankshaft& );

  public:
    enum Type { Yield, State };

    Crankshaft (const std::string& name = "Crankshaft");
    virtual ~Crankshaft ();
    Crankshaft *clone() const 
        { throw std::runtime_error(
                    "Crankshaft::clone not implemented."); }

    void add( Output& transmission, Type type = Yield )
        { _add( &transmission, (type == Yield), false ); }
    void add( Output* tm, Type type = Yield )
        { _add( tm, (type == Yield), true ); }
    void add( std::auto_ptr<Output> tm, Type type = Yield )
        { if (tm.get() != NULL) _add( tm.release(), (type == Yield), true ); }

    void push_front( Output& transmission, Type type = Yield )
        { _add( &transmission, (type == Yield), false, true ); }
    void push_front( Output* tm, Type type = Yield )
        { _add( tm, (type == Yield), true, true ); }
    void push_front( std::auto_ptr<Output> tm, Type type = Yield )
        { if (tm.get() != NULL) _add( tm.release(), (type == Yield),
            true, true ); }

    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement&);
    void receiveLocalizations(const EngineResult&);
    void store_results();

    bool empty() const { return clutches.empty(); }
    void check_for_duplicate_filenames
        (std::set<std::string>& present_filenames);

};
}
}

#endif
