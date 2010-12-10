#ifndef DSTORM_CRANKSHAFT_H
#define DSTORM_CRANKSHAFT_H

#include <dStorm/Transmission.h>
#include <list>
#include <cc++/thread.h>

namespace dStorm {
    class Crankshaft : public Transmission {
        ost::ThreadLock clutchesLock;
        class Clutch;
        typedef std::list<Clutch> Clutches;
        Clutches clutches;
        ost::Mutex toDeleteLock;
        /** This iterator saves any Transmission that should be deleted from
         *  the crankshaft. By default, it is set to clutches.end() to indicate
         *  no deletion; if set differently, the element referenced by this iterator
         *  will be deleted once all readers cleared the critical sections. */
        std::list<Clutch>::iterator toDelete;

        void _add( Transmission *tm, bool important, bool manage ) throw();

      public:
        enum Type { Yield, State };

        Crankshaft () throw ();
        virtual ~Crankshaft () throw ();

        void add( Transmission& transmission, Type type = Yield ) throw()
            { _add( &transmission, (type == Yield), false ); }
        void add( Transmission* tm, Type type = Yield ) throw()
            { _add( tm, (type == Yield), true ); }
        void add( std::auto_ptr<Transmission> tm, Type type = Yield ) throw()
            { if (tm.get() != NULL) _add( tm.release(), (type == Yield), true ); }

        AdditionalData announceStormSize(int w, int h, int l) throw();
        Result receiveLocalizations(const EngineResult&) throw();
        void deleteAllResults() throw();
        void finish() throw();

        bool empty() const throw() { return clutches.empty(); }
    };
}

#endif
