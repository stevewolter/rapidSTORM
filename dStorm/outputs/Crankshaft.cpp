#define DSTORM_CRANKSHAFT_CPP
#include "Crankshaft.h"
#include <iostream> 
#include <sstream> 

#include <cassert>
#include <dStorm/data-c++/Vector.h>

#include <boost/shared_ptr.hpp>

#include "debug.h"
#include <boost/units/io.hpp>

using namespace std;
using namespace data_cpp;
using namespace ost;
using namespace dStorm::output;

namespace dStorm {
namespace outputs {

static std::string disambiguation(int id) {
    std::stringstream ss;
    ss << "Output" << id;
    return ss.str();
}

class Crankshaft::Clutch : public simparm::Object {
    Output& output;
    boost::shared_ptr<Output> content;
    bool important;
  public:
    Clutch(Output& content, bool important, int id, bool man)
        : simparm::Object(disambiguation(id), ""),
          output( content ),
          content( (man) ? &content : NULL), important(important) 
        { 
            assert( &output != NULL ); 
            this->simparm::Node::push_back( output.getNode() );
        }
    Clutch( const Clutch& o ) 
        : simparm::Object(o), output(o.output),
          content(o.content), important(o.important)
        {
            this->simparm::Node::push_back( output.getNode() );
        }
    ~Clutch() 
    {
        this->simparm::Node::erase( output.getNode() );
    }
    
    Output* operator->() { return &output; }
    Output& operator*() { return output; }
    bool isImportant() const { return important; }
};

Crankshaft::Crankshaft (const std::string& name) 
: OutputObject(name, "Crankshaft"),
  id(0)
{
    toDelete = clutches.end();
}

Crankshaft::~Crankshaft () {}

void Crankshaft::_add( Output *tm, bool imp, bool man, bool front ) 
{
    assert( tm != NULL );

    WriteLock changing_clutches(clutchesLock);
    DEBUG("Crankshaft accepted transmission " << tm->getNode().getName());
    clutches.push_back( Clutch( *tm, imp, id++, man ) );
    if ( front ) {
        this->Node::push_front( clutches.back() );
    }  else {
        this->Node::push_back( clutches.back() );
    }
    toDelete = clutches.end();
}

Output::AdditionalData
Crankshaft::announceStormSize(const Announcement &a) 
 
{
    ReadLock reader(clutchesLock);
    AdditionalData data;
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++){
        DEBUG("Announcing size to transmission " << (*i)->getNode().getName());
        data |= (*i)->announceStormSize(a);
        DEBUG("Announced size to transmission " << (*i)->getNode().getName());
    }
    return data;
}

void Crankshaft::propagate_signal(ProgressSignal s) {
    DEBUG("Announcing engine start");
    ReadLock reader(clutchesLock);
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++) {
        DEBUG("Announcing engine start to " << (*i)->getNode().getName());
        (*i)->propagate_signal(s);
        DEBUG("Announced engine start to " << (*i)->getNode().getName());
    }
    DEBUG("Announced engine start");
}

Output::Result Crankshaft::receiveLocalizations(const EngineResult& er) 
{
    DEBUG("Receiving " << er.number << " locs for " << er.forImage);
    bool haveImportantOutput = false;
    {
        ReadLock reader(clutchesLock);
        for (Clutches::iterator i = clutches.begin(); i != clutches.end();)
        {
            /* The result is fetched under the lock, but processed out
            * of it; this is done to avoid deadlocks on lock upgrade. */
            Output::Result r;
            DEBUG("Working on clutch " << (*i)->getNode().getName());
            r = (*i)->receiveLocalizations( er );
            DEBUG("Worked on clutch " << (*i)->getNode().getName() <<
                    " with result " << r);
            switch (r) {
                case Output::KeepRunning:
                    haveImportantOutput |= i->isImportant();
                    i++;
                    break;
                case Output::RemoveThisOutput:
                    {
                        DEBUG("Removing clutch " << (*i)->getNode().getName());
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
    DEBUG("Releasing readLock");

    if ( toDelete != clutches.end() ) {
        WriteLock changing_clutches(clutchesLock);
        MutexLock deletor(toDeleteLock);
        /* Double-check to avoid race conditions. */
        if ( toDelete != clutches.end() ) {
            clutches.erase( toDelete );
            toDelete = clutches.end();
        }
    }

    if ( ! haveImportantOutput)
        return Output::StopEngine;
    else
        return Output::KeepRunning;
}

void Crankshaft::check_for_duplicate_filenames
    (std::set<std::string>& present_filenames) 
{
    ReadLock reader(clutchesLock);
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++)
        (*i)->check_for_duplicate_filenames(present_filenames);
}

}
}
