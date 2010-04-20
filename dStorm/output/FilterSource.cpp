#include "debug.h"

#include "FilterSource.h"
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include "outputs/Crankshaft.h"
#include "OutputSource.h"
#include "SourceFactory.h"
#include "doc/help/context.h"


namespace dStorm {
namespace output {

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
        DEBUG("Resetting state of source factory in filter source for element " << getNode().getName());
        factory->reset_state();

        DEBUG("Registering entries");
        removeSelector->viewable = (removeSelector->numChoices() > 0);
        getNode().push_back( factory->getNode() );
        getNode().push_back( *removeSelector );
        receive_changes_from( factory->getNode() );
        receive_changes_from( removeSelector->value );
        DEBUG("Registered entries");
    }
}

FilterSource::FilterSource(simparm::Node& node)
: OutputSource( node ),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  next_identity(0),
  factory( NULL ),
  removeSelector( new simparm::NodeChoiceEntry<RemovalObject> (
        "ToRemove", "Select output to remove") )
{
    removeSelector->set_auto_selection( false );
    removeSelector->helpID = HELP_ToRemove;
}

FilterSource::FilterSource ( simparm::Node& node, const FilterSource& o)
: OutputSource(node, o),
    simparm::Node::Callback(simparm::Event::ValueChanged),
    next_identity(0),
    factory( NULL ),
    outputs(),
    removeSelector( new simparm::NodeChoiceEntry<RemovalObject> (
        "ToRemove", "Select output to remove") )
{
    DEBUG("Copying " << o.getNode().getName());
    removeSelector->set_auto_selection( false );
    //if ( o.is_initialized() ) initialize(*o.factory);
    for ( const_iterator i = o.begin(); i != o.end(); i++ ) {
        add( std::auto_ptr<OutputSource>( (*i)->clone() ) );
    }
}

FilterSource::~FilterSource() {
        DEBUG("Destroying filter source");
}

void
FilterSource::set_output_file_basename
    (const Basename& basename) 
{
    this->basename = basename;

    for (Outputs::iterator i = 
                    outputs.begin(); i != outputs.end(); i++)
        (*i)->set_output_file_basename(basename);

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
    ( const simparm::Event& e)
{
    if (&e.source == &factory->getNode() )
    {
        try {
            std::auto_ptr<OutputSource> fresh
                                    = factory->make_output_source();
            if ( fresh.get() != NULL ) {
                fresh->set_output_file_basename(basename);
                add( fresh );
                /* To give some kind of visual feedback that the action was
                * performed, we reset the factory ( which means, normally,
                * that no item is selected in the chooser ). */
                factory->reset_state();
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
        }
    } else if (&e.source == &removeSelector->value ) {
        if ( removeSelector->isValid() ) {
            remove( *removeSelector->value().src );
        }
    }
}

void FilterSource::link_transmission
    ( OutputSource* src ) 
{
    std::stringstream nodeName;
    DEBUG("Making node name Output" << next_identity);
    nodeName << "Output" << next_identity++;

    std::auto_ptr<simparm::Object> addNode( 
        new simparm::Object(nodeName.str(), "") );

    simparm::Node::iterator linkit 
        = addNode->insert( addNode->end(), src->getNode() );

    /* Manage the add node with the link. */
    linkit.get_link().delete_up_end_on_link_break();
    simparm::Object& addNodeRef = *addNode.release();
    getNode().push_back( addNodeRef );

    std::auto_ptr<RemovalObject> removeNode( 
        new RemovalObject(nodeName.str(), src, outputs) );
    removalObjects.insert( std::make_pair( src, removeNode.get() ) );
    removeSelector->addChoice( removeNode );

    removeSelector->viewable = true;
}

void FilterSource::set_output_factory (const SourceFactory& o) 
{
    DEBUG("Trying to set output factory");
    if ( factory.get() == NULL ) { 
        DEBUG("No output factory present, setting new factory");
        factory.reset( o.clone() ); 
        DEBUG("Registering entries in factory");
        registerNamedEntries();
        DEBUG("Finished setting output factory");
    }
}

void FilterSource::set_source_capabilities( Capabilities cap ) {
    if ( is_initialized() ) {
        factory->set_source_capabilities( cap );
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
        std::auto_ptr<outputs::Crankshaft> crankshaft
            ( new outputs::Crankshaft() );

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

}
}
