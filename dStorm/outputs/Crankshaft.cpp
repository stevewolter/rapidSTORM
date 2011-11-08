#define DSTORM_CRANKSHAFT_CPP
#include "Crankshaft.h"
#include <iostream> 
#include <sstream> 

#include <cassert>

#include <boost/shared_ptr.hpp>

#include "debug.h"
#include <boost/units/io.hpp>

using namespace std;
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
}

Crankshaft::~Crankshaft () {}

void Crankshaft::_add( Output *tm, bool imp, bool man, bool front ) 
{
    assert( tm != NULL );

    DEBUG("Crankshaft accepted transmission " << tm->getNode().getName());
    clutches.push_back( Clutch( *tm, imp, id++, man ) );
    if ( front ) {
        this->Node::push_front( clutches.back() );
    }  else {
        this->Node::push_back( clutches.back() );
    }
}

Output::AdditionalData
Crankshaft::announceStormSize(const Announcement &a) 
 
{
    AdditionalData data;
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++){
        DEBUG("Announcing size to transmission " << (*i)->getNode().getName());
        data |= (*i)->announceStormSize(a);
        DEBUG("Announced size to transmission " << (*i)->getNode().getName());
    }
    return data;
}

Output::RunRequirements 
Crankshaft::announce_run(const RunAnnouncement& a) 
{
    Output::RunRequirements requirements;
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++)
        requirements |= (*i)->announce_run(a);
    return requirements;
}

void Crankshaft::propagate_signal(ProgressSignal s) {
    DEBUG("Announcing engine start");
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
    for (Clutches::iterator i = clutches.begin(); i != clutches.end();)
    {
        Output::Result r = (*i)->receiveLocalizations( er );
        switch (r) {
            case Output::KeepRunning:
                haveImportantOutput |= i->isImportant();
                ++i;
                break;
            case Output::RemoveThisOutput:
                i = clutches.erase( i );
                i++;
                break;
        }
    }
    DEBUG("Releasing readLock");

    return Output::KeepRunning;
}

void Crankshaft::check_for_duplicate_filenames
    (std::set<std::string>& present_filenames) 
{
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++)
        (*i)->check_for_duplicate_filenames(present_filenames);
}

}
}
