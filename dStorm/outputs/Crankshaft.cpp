#define DSTORM_CRANKSHAFT_CPP
#include "Crankshaft.h"
#include <iostream> 
#include <sstream> 

#include <cassert>
#include <dStorm/data-c++/Vector.h>

using namespace std;
using namespace data_cpp;
using namespace ost;

namespace dStorm {

static std::string disambiguation(int id) {
    std::stringstream ss;
    ss << "Output" << id;
    return ss.str();
}

class Crankshaft::Clutch : public simparm::Object {
    Output *content;
    bool important;
  public:
    Clutch(Output *content, bool important, int id)
        : simparm::Object(disambiguation(id), ""),
          content(content), important(important) 
        { 
            assert( content != NULL ); 
            this->simparm::Node::push_back( *content );
        }
    Clutch( const Clutch& o ) 
        : simparm::Node(o), simparm::Object(o),
          content(o.content), important(o.important)
        {
            this->simparm::Node::push_back( *content );
        }
    
    Output* operator->() { return content; }
    Output& operator*() { return *content; }
    bool isImportant() const { return important; }
};

Crankshaft::Crankshaft () 
: Object("Crankshaft", "Crankshaft"),
  id(0)
{
    toDelete = clutches.end();
}

Crankshaft::~Crankshaft () {}

void Crankshaft::_add( Output *tm, bool imp, bool man, bool front ) 
{
    assert( tm != NULL );

    WriteLock changing_clutches(clutchesLock);
    PROGRESS("Crankshaft accepted transmission " << tm->getName());
    clutches.push_back( Clutch( tm, imp, id++ ) );
    if ( front ) {
        this->Node::push_front( clutches.back() );
    }  else {
        this->Node::push_back( clutches.back() );
    }
    toDelete = clutches.end();
    if (man) this->Node::manage( std::auto_ptr<Node>(tm) );
}

Output::AdditionalData
Crankshaft::announceStormSize(const Announcement &a) 
 
{
    ReadLock reader(clutchesLock);
    AdditionalData data = NoData;
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++){
        PROGRESS("Announcing size to transmission " << (*i)->getName());
        data = AdditionalData( data | (*i)->announceStormSize(a) );
        PROGRESS("Announced size to transmission " << (*i)->getName());
    }
    return data;
}

void Crankshaft::propagate_signal(ProgressSignal s) {
    PROGRESS("Announcing engine start");
    ReadLock reader(clutchesLock);
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++)
        (*i)->propagate_signal(s);
}

Output::Result Crankshaft::receiveLocalizations(const EngineResult& er) 
{
    bool haveImportantOutput = false;
    {
        ReadLock reader(clutchesLock);
        for (Clutches::iterator i = clutches.begin(); i != clutches.end();)
        {
            /* The result is fetched under the lock, but processed out
            * of it; this is done to avoid deadlocks on lock upgrade. */
            Output::Result r;
            LOCKING("Working on clutch " << (*i)->getName());
            r = (*i)->receiveLocalizations( er );
            LOCKING("Worked on clutch " << (*i)->getName() <<
                    " with result " << r);
            switch (r) {
                case Output::KeepRunning:
                    haveImportantOutput |= i->isImportant();
                    i++;
                    break;
                case Output::RemoveThisOutput:
                    {
                        PROGRESS("Removing clutch " << (*i)->getName());
                        MutexLock deletor(toDeleteLock);
                        if ( toDelete == clutches.end() )
                            toDelete = i;
                    }
                    i++;
                    break;
                case Output::StopEngine:
                case Output::RestartEngine:
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
            this->Node::erase( **toDelete );
            clutches.erase( toDelete );
            toDelete = clutches.end();
        }
    }

    if ( ! haveImportantOutput)
        return Output::StopEngine;
    else
        return Output::KeepRunning;
}

}
