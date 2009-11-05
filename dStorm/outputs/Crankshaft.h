#ifndef DSTORM_CRANKSHAFT_H
#define DSTORM_CRANKSHAFT_H

#include <dStorm/output/Output.h>
#include <list>
#include <dStorm/helpers/thread.h>

namespace dStorm {
    class Crankshaft : public Output, public simparm::Object {
        class Source;

        ost::ThreadLock clutchesLock;
        class Clutch;
        typedef std::list<Clutch> Clutches;
        Clutches clutches;
        ost::Mutex toDeleteLock;
        /** This iterator saves any Output that should be deleted
         *  from the crankshaft. By default, it is set to clutches.end()
         *  to indicate no deletion; if set differently, the element
         *  referenced by this iterator will be deleted once all readers
         *  cleared the critical sections. */
        std::list<Clutch>::iterator toDelete;
        int id;

        void _add( Output *tm, bool important, bool manage, bool front = false );

        /** No copy constructor defined. */
        Crankshaft( const Crankshaft& );

      public:
        enum Type { Yield, State };

        Crankshaft ();
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

        AdditionalData announceStormSize(const Announcement&) 
;
        void propagate_signal(ProgressSignal);
        Result receiveLocalizations(const EngineResult&);

        bool empty() const { return clutches.empty(); }
    };
}

#endif
