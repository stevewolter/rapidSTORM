#define DSTORM_CRANKSHAFT_CPP
#include "Crankshaft.h"
#include <foreach.h>
#include <iostream>

#include <cassert>
#include <data-c++/Vector.h>

using namespace std;
using namespace data_cpp;
using namespace ost;

namespace dStorm {

class Crankshaft::Clutch {
    Transmission *content;
    bool important;
  public:
    Clutch(Transmission *content, bool important) throw()
        : content(content), important(important) { assert( content != NULL ); }
    
    Transmission* operator->() throw() { return content; }
    bool isImportant() const throw() { return important; }
};

Crankshaft::Crankshaft ()throw () 
: ConfigObject("Crankshaft", "Crankshaft")
{
    toDelete = clutches.end();
}

Crankshaft::~Crankshaft () throw () {}

void Crankshaft::_add( Transmission *tm, bool imp, bool man ) throw() 
{
    assert( tm != NULL );

    WriteLock changing_clutches(clutchesLock);
    PROGRESS("Crankshaft accepted transmission " << tm->getName());
    clutches.push_back( Clutch( tm, imp ) );
    toDelete = clutches.end();
    this->ConfigNode::push_back( *tm );
    if (man) this->ConfigNode::manage( std::auto_ptr<ConfigNode>(tm) );
}

Transmission::AdditionalData
Crankshaft::announceStormSize(int w, int h, int l) throw() {
    PROGRESS("Announcing size " << w << ", " << h << ", " << l);
    ReadLock reader(clutchesLock);
    AdditionalData data = NoData;
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++){
        PROGRESS("Announcing size to transmission " << (*i)->getName());
        data = AdditionalData( data | (*i)->announceStormSize(w,h,l) );
        PROGRESS("Announced size to transmission " << (*i)->getName());
    }
    return data;
}

Transmission::Result Crankshaft::receiveLocalizations(const EngineResult& er) throw() 
{
    bool haveImportantTransmissions = false;
    {
        ReadLock reader(clutchesLock);
        for (Clutches::iterator i = clutches.begin(); i != clutches.end(); )
        {
            /* The result is fetched under the lock, but processed out
            * of it; this is done to avoid deadlocks on lock upgrade. */
            Transmission::Result r;
            LOCKING("Working on clutch " << (*i)->getName());
            r = (*i)->receiveLocalizations( er );
            LOCKING("Worked on clutch " << (*i)->getName() <<
                    " with result " << r);
            switch (r) {
                case Transmission::KeepRunning:
                    haveImportantTransmissions |= i->isImportant();
                    i++;
                    break;
                case Transmission::RemoveTransmission:
                    {
                        PROGRESS("Removing clutch " << (*i)->getName());
                        MutexLock deletor(toDeleteLock);
                        if ( toDelete == clutches.end() )
                            toDelete = i;
                    }
                    break;
                case Transmission::StopEngine:
                case Transmission::RestartEngine:
                    return r;
            }
        }
    }
    LOCKING("Releasing readLock");

    if ( toDelete != clutches.end() ) {
        WriteLock changing_clutches(clutchesLock);
        MutexLock deletor(toDeleteLock);
        /* Double-check to avoid race conditions. */
        if ( toDelete != clutches.end() ) {
            clutches.erase( toDelete );
            toDelete = clutches.end();
        }
    }

    if ( ! haveImportantTransmissions)
        return Transmission::StopEngine;
    else
        return Transmission::KeepRunning;
}

void Crankshaft::deleteAllResults() throw() {
    ReadLock reader(clutchesLock);
    for (Clutches::iterator i = clutches.begin(); i != clutches.end();i++){
        LOCKING("Working on clutch " << (*i)->getName());
        (*i)->deleteAllResults();
        LOCKING("Worked on clutch " << (*i)->getName());
    }
}

void Crankshaft::finish() throw() {
    ReadLock reader(clutchesLock);
    for (Clutches::iterator i = clutches.begin(); i != clutches.end();i++){
        (*i)->finish();
    }
}

}
