#include "EmissionTracker.h"
#include <algorithm>

namespace dStorm {

template <typename T>
T sq(const T& a) { return a*a; }

class EmissionTracker::LocalizationCollater {
  private:
    data_cpp::Vector<Localization> results;
    TraceReducer& reducer;

  public:
    LocalizationCollater(TraceReducer &reducer) throw() : reducer(reducer) {}

    void clear() throw() { results.clear(); }

    void operator()( const Trace& e ) throw() {
        reducer.reduce_trace_to_localization( e, results.allocate() );
        results.commit();
    }

    /** Return a pointer to the Localizations stored in this collater
     *  since the last clear(). */
    const Localization* ptr() throw() { return results.ptr(); }
    /** Return the number of Localizations stored in this collater
     *  since the last clear(). */
    int size() throw() { return results.size(); }
};

double distSq(const Localization& e, const Localization &loc) throw() {
    return ( sq(e.x() - loc.x()) + sq(e.y() - loc.y()) );
}

EmissionTracker::_Config::_Config(TransmissionSourceFactory& tc) throw() 
: ConfigObject("", ""),
  TransmissionForwarderSource(tc),
  allowBlinking("AllowBlinking", "Allow fluorophores to skip n frames")
{
}

EmissionTracker::EmissionTracker( std::auto_ptr<Transmission> output,
                                  std::auto_ptr<TraceReducer> reducer )
: ConfigObject("EmissionTracker", "Emission tracking status"),
  track_modulo(2), next_to_track_moved(mutex), 
    output(allocator), garbage_bin(allocator),
    reducer(reducer), collater(new LocalizationCollater(*reducer)), target(output)
{}

EmissionTracker::~EmissionTracker() throw() {}

Transmission::Result
EmissionTracker::receiveLocalizations(const EngineResult &er) throw()
{
    ost::MutexLock lock(mutex);
    const double maxDist = 1.0;

    int imNum = er.forImage;
    int currentCell = imNum, lastCell = imNum-1;
    tracking[currentCell].prepare( imNum );

    /* Wait for the tracking to arrive at the current position. */
    while (next_to_track != imNum) {
        next_to_track_moved.wait();
    }

    //cerr << "BEGIN " << currentCell << " " << lastCell << endl;
    for (int loc = 0; loc < er.number; loc++) 
    {
        const Localization *locs = er.first + loc;
        int cell_x = int(round(locs->x())), cell_y = int(round(locs->y()));

        //cerr << "Searching for emission for " << cell_x << " " << cell_y << endl;
        List::iterator e;
        if (lastCell >= 0)
            e = tracking[lastCell].positional(cell_x, cell_y);

        if ( !e || distSq(e->back(), *locs) > maxDist ) {
            //cerr << "Im " << imNum << " has new emission for "
                 //<< cell_x << " " << cell_y << endl;
            /* Initialize new Emission. */
            e = getNewEmissionNode();
            e->clear();
            //cerr << "Got new emission " << endl;
        }

        //cerr << "Updating emission position." << endl;
        e->push_back( *locs );
        //cerr << "Position is " << e->x() << "," << e->y() << endl;
        List::splice(tracking[currentCell].emissions.end(), e);
        //cerr << "Moved into place." << endl;

        int nplx = int(floor(e->back().x()-maxDist)), 
            nphx = int(ceil(e->back().x()+maxDist/2)), 
            nply = int(floor(e->back().y()-maxDist)),
            nphy = int(ceil(e->back().y()+maxDist/2));
        
        //cerr << "Updating [" << nplx << "," << nply << "] to ["
             //<< nphx << "," << nphy << "]" << endl;
        for (int x = nplx; x <= nphx; x++)
            for (int y = nply; y <= nphy; y++)
                tracking[currentCell].positional(x,y) = e;
        //cerr << "Updated" << endl;
    }

    if (lastCell >= 0)
        finalizeImage(lastCell);

    next_to_track = imNum+1;
    next_to_track_moved.signal();
    //cerr << "END " << imNum << endl;
    return KeepRunning;
}

void EmissionTracker::finalizeImage(int i) throw() {
    List& ended = tracking[i].emissions;

    std::for_each( ended.begin(), ended.end(), *collater );

    EngineResult er;
    er.forImage = i;
    er.first = collater->ptr();
    er.number = collater->size();

    target->receiveLocalizations(er);
}

}
