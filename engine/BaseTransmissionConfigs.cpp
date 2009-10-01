#include "FilterSource.h"
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include "transmissions/Crankshaft.h"
#include "OutputSource.h"
#include "OutputFactory.h"
#include "help_context.h"

namespace dStorm {

struct FilterSource::RemovalObject : public simparm::Object {
    std::auto_ptr<OutputSource> src;
    std::list<OutputSource*>& sources;

    RemovalObject(std::string name, OutputSource* src, 
                  std::list<OutputSource*>& sources)
        : simparm::Object(name, src->getDesc()),
          src(src), sources(sources) {}
    RemovalObject* clone() const { throw std::logic_error("Unclonable.");}

    ~RemovalObject() {
        sources.erase(std::find(sources.begin(), sources.end(), src.get()));
    }

  private:
    RemovalObject( const RemovalObject& o );
};

void FilterSource::registerNamedEntries()
{
    if ( factory.get() != NULL ) {
        factory->reset_state();

        removeSelector->viewable = (removeSelector->numChoices() > 0);
        removeButton.viewable = (removeSelector->numChoices() > 0);
        Node::push_back( *factory );
        Node::push_back( addButton );
        Node::push_back( *removeSelector );
        Node::push_back( removeButton );
        receive_changes_from( addButton );
        receive_changes_from( removeButton );
    }
}

FilterSource::FilterSource()
: next_identity(0),
  factory( NULL ),
  addButton("AddOutput", "Add selected output") ,
  removeButton("RemoveOutput", "Remove selected output"),
  removeSelector( new simparm::NodeChoiceEntry<RemovalObject> (
        "ToRemove", "Select output to remove") )
{
    addButton.helpID = HELP_AddOutput;
    removeButton.helpID = HELP_RemoveOutput;
    removeSelector->helpID = HELP_ToRemove;
    registerNamedEntries();
}

FilterSource::FilterSource (const FilterSource& o)
: simparm::Node(o), OutputSource(o),
    simparm::Node::Callback(),
    next_identity(o.next_identity),
    factory( NULL ),
    outputs(),
    addButton(o.addButton),
    removeButton(o.removeButton),
    removeSelector( new simparm::NodeChoiceEntry<RemovalObject> (
        "ToRemove", "Select output to remove") )
{
    //if ( o.is_initialized() ) initialize(*o.factory);
    registerNamedEntries();
    for ( const_iterator i = o.begin(); i != o.end(); i++ ) {
        add( std::auto_ptr<OutputSource>( (*i)->clone() ) );
    }
}

FilterSource::~FilterSource() {
}

FilterSource::BasenameResult
FilterSource::set_output_file_basename
    (const std::string& basename, std::set<std::string>& avoid) 
{
    this->basename = basename;
    this->avoid = &avoid;

    for (Outputs::iterator i = 
                    outputs.begin(); i != outputs.end(); i++)
    {
        BasenameResult r = 
            (*i)->set_output_file_basename(basename, avoid);
        if ( r == Basename_Conflicted )
            return r;
    }
    if ( is_initialized() ) 
        factory->set_output_file_basename( basename, avoid );

    return Basename_Accepted;
}

void FilterSource::add
    ( std::auto_ptr<OutputSource> src )
{
    if ( src.get() == NULL ) return;
    link_transmission( src.get() );
    outputs.push_back( src.release() );
}

void FilterSource::remove( OutputSource& src ) {
    assert( removalObjects.find( &src ) != removalObjects.end() );
    removeSelector->removeChoice( *removalObjects[&src] );
}

void FilterSource::operator()
    ( simparm::Node& src, Cause c, simparm::Node *)
{
    if (&src == &addButton && c == ValueChanged && 
        addButton.triggered()) 
    {
        addButton.untrigger();
        try {
            std::auto_ptr<OutputSource> fresh
                                    = factory->make_output_source();
            if ( fresh.get() != NULL ) {
                if ( basename != "" )
                    fresh->set_output_file_basename(basename, *avoid);
                add( fresh );
            }
            /* To give some kind of visual feedback that the action was
             * performed, we reset the factory ( which means, normally,
             * that no item is selected in the chooser ). */
            factory->reset_state();
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
        }
    } else if (&src == &removeButton && c == ValueChanged && 
        removeButton.triggered()) 
    {
        removeButton.untrigger();
        if ( removeSelector->isValid() ) {
            remove( *removeSelector->value().src );
        }
    }
}

void FilterSource::link_transmission
    ( OutputSource* src ) 
{
    FilterSource* fwd_child 
        = dynamic_cast<FilterSource*>(src);
    if ( fwd_child != NULL ) {
        fwd_child->focus_immediately = true;
        if ( is_initialized() )
            fwd_child->initialize( *factory );
    }

    std::stringstream nodeName;
    nodeName << "Output" << next_identity++;

    std::auto_ptr<simparm::Object> addNode( 
        new simparm::Object(nodeName.str(), "") );
    addNode->push_back( *src );
    this->push_back( *addNode );
    src->manage( addNode.release() );

    std::auto_ptr<RemovalObject> removeNode( 
        new RemovalObject(nodeName.str(), src, outputs) );
    removalObjects.insert( std::make_pair( src, removeNode.get() ) );
    removeSelector->addChoice( removeNode );

    removeSelector->viewable = true;
    removeButton.viewable = true;
}

void FilterSource::initialize (const OutputFactory& o) 
 
{
    if ( factory.get() == NULL ) { 
        factory.reset( o.clone() ); 
        registerNamedEntries();
    }
}

std::auto_ptr<Output> 
FilterSource::make_output()
{
    if ( outputs.size() == 0 )
        throw std::invalid_argument(
            "No output selected for module '" + getDesc() + "'");
    else if ( outputs.size() == 1 ) {
        std::auto_ptr<Output> o = 
            outputs.front()->make_output();
        if ( o.get() == NULL )
            throw std::invalid_argument(
                "Output for module '" + getDesc() + "' is invalid");
        else
            return o;
    } else {
        std::auto_ptr<Crankshaft> crankshaft( new Crankshaft() );

        Outputs::iterator i;
        for( i = outputs.begin(); i != outputs.end(); i++ ) {
            crankshaft->add( (*i)->make_output() );
        }

        if ( ! crankshaft->empty() )
            return std::auto_ptr<Output>(crankshaft);
        else
            throw std::invalid_argument(
                "Outputs for module '" + getDesc() + "' are invalid");
            
    }
}
};
