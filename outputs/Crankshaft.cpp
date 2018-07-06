#define DSTORM_CRANKSHAFT_CPP
#include "outputs/Crankshaft.h"

#include <iostream> 
#include <sstream> 
#include <cassert>

#include <boost/shared_ptr.hpp>
#include <boost/units/io.hpp>

#include "input/Source.h"
#include "simparm/Object.h"

#include "debug.h"

using namespace std;
using namespace dStorm::output;

namespace dStorm {
namespace outputs {

static std::string disambiguation(int id) {
    std::stringstream ss;
    ss << "Output" << id;
    return ss.str();
}

class Crankshaft::Clutch {
    simparm::Object name_object;
    Output& output;
    boost::shared_ptr<Output> content;
    bool important;
  public:
    Clutch(Output& content, bool important, int id, bool man)
        : name_object(disambiguation(id), ""),
          output( content ),
          content( (man) ? &content : NULL), important(important) 
        { 
            assert( &output != NULL ); 
        }
    ~Clutch() {}
    
    Output* operator->() { return &output; }
    Output& operator*() { return output; }
    bool isImportant() const { return important; }
    void attach_ui( simparm::NodeHandle at ) {
        output.attach_ui( name_object.attach_ui( at ) );
    }
};

Crankshaft::Crankshaft () 
: id(0)
{
}

Crankshaft::~Crankshaft () {}

void Crankshaft::_add( Output *tm, bool imp, bool man ) 
{
    assert( tm != NULL );

    clutches.push_back( Clutch( *tm, imp, id++, man ) );
    if ( current_ui )
        clutches.back().attach_ui( *current_ui );
}

void Crankshaft::announceStormSize(const Announcement &a) 
{
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++){
        (*i)->announceStormSize(a);
    }
}

Output::RunRequirements 
Crankshaft::announce_run(const RunAnnouncement& a) 
{
    Output::RunRequirements requirements;
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++)
        requirements |= (*i)->announce_run(a);
    return requirements;
}

void Crankshaft::store_results_( bool success ) {
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end(); ++i)
        (*i)->store_results( success );
}

void Crankshaft::receiveLocalizations(const EngineResult& er) 
{
    for (Clutches::iterator i = clutches.begin(); i != clutches.end(); ++i)
        (*i)->receiveLocalizations( er );
}

void Crankshaft::check_for_duplicate_filenames
    (std::set<std::string>& present_filenames) 
{
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++)
        (*i)->check_for_duplicate_filenames(present_filenames);
}

void Crankshaft::prepare_destruction_() {
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++)
        (*i)->prepare_destruction();
}

void Crankshaft::run_finished_( const RunFinished& info ) {
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end();i++)
        (*i)->run_finished( info );
}

void Crankshaft::attach_ui_( simparm::NodeHandle at ) {
    current_ui = at;
    for (Clutches::iterator i = clutches.begin(); i!=clutches.end(); ++i)
        i->attach_ui( *current_ui );
}

}
}
