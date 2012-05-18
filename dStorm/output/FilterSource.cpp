#include "debug.h"

#include "FilterSource.h"
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/outputs/Crankshaft.h>
#include "OutputSource.h"
#include "SourceFactory.h"
#include <dStorm/helpers/clone_ptr.hpp>
#include <boost/lexical_cast.hpp>

namespace dStorm {
namespace output {

class FilterSource::Suboutput {
    boost::clone_ptr<OutputSource> unadorned;
    simparm::Object config_node, removal_node;

public:
    Suboutput(std::auto_ptr< OutputSource > unadorned, std::string ident, simparm::Node* ui )
        : unadorned( unadorned ), config_node( ident, ""),
          removal_node(ident, this->unadorned->getDesc()) 
        { if ( ui ) attach_suboutput_ui(*ui); }
    Suboutput* clone() const { return new Suboutput(*this); }

    std::string getName() const { return removal_node.getName(); }
    void attach_ui( simparm::Node& removal_choice_node )
        { removal_node.attach_ui( removal_choice_node ); }
    void detach_ui( simparm::Node& removal_choice_node )
        { removal_node.detach_ui( removal_choice_node ); }
    void attach_suboutput_ui( simparm::Node& at ) {
        unadorned->attach_full_ui( config_node );
        config_node.attach_ui( at );
    }

    OutputSource& output() const { return *unadorned; }
};

void FilterSource::attach_source_ui( simparm::Node& at )
{
    OutputSource::attach_source_ui(at);
    assert( ! my_node );

    my_node = &at;

    for ( SuboutputChoice::iterator i = suboutputs.begin(); i != suboutputs.end(); ++i )
        i->attach_suboutput_ui( at );

    if ( factory.get() != NULL ) {
        DEBUG("Registering entries");
        factory->notify_when_output_source_is_available( 
            boost::bind( &FilterSource::add_new_element, boost::ref(*this) ) );
        factory->attach_ui( at );
        suboutputs.attach_ui( at );
        receive_changes_from( suboutputs.value );
        DEBUG("Registered entries");
    }
}

FilterSource::FilterSource()
: simparm::Node::Callback( simparm::Event::ValueChanged ),
  next_identity(0),
  factory( NULL ),
  my_node( NULL ),
  suboutputs( "ToRemove", "Select output to remove" )
{
    suboutputs.set_auto_selection( false );
    suboutputs.helpID = "#ToRemove";
}

FilterSource::FilterSource( const FilterSource& o ) 
: OutputSource(o), simparm::Node::Callback(o),
  next_identity(o.next_identity), basename(o.basename),
  factory( NULL ),
  my_node( NULL ),
  suboutputs(o.suboutputs)
{
}

FilterSource::~FilterSource() {}

void
FilterSource::set_output_file_basename
    (const Basename& basename) 
{
    OutputSource::set_output_file_basename( basename );
    this->basename = basename;

    for (SuboutputChoice::iterator i = 
                    suboutputs.begin(); i != suboutputs.end(); i++)
        i->output().set_output_file_basename(basename);

}

void FilterSource::add
    ( std::auto_ptr<OutputSource> src )
{
    assert( src.get() );
    std::string name = "Output" + boost::lexical_cast<std::string>( next_identity++ );
    src->set_output_file_basename(basename);
    suboutputs.addChoice( new Suboutput( src, name, my_node ) );
}

void FilterSource::add_new_element() {
    try {
        std::auto_ptr<OutputSource> fresh = factory->make_output_source();
        if ( fresh.get() != NULL ) add( fresh );
    } catch (const std::runtime_error& e) {
        std::cerr << "Unable to set basename: " << e.what() << "\n";
    }
}

void FilterSource::operator()
    ( const simparm::Event& e)
{
    if (&e.source == &suboutputs.value ) {
        if ( suboutputs.isValid() ) {
            suboutputs.removeChoice( suboutputs() );
        }
    }
}

void FilterSource::set_output_factory (const SourceFactory& o) 
{
    DEBUG("Trying to set output factory on " << this << " (" << getName() << ")");
    if ( factory.get() == NULL ) { 
        DEBUG("No output factory present, setting new factory");
        factory.reset( o.clone() ); 
        DEBUG("Finished setting output factory");
    }
}

void FilterSource::set_source_capabilities( Capabilities cap ) {
    if ( is_initialized() ) {
        factory->set_source_capabilities( cap );
        SuboutputChoice::iterator i;
        for( i = suboutputs.begin(); i != suboutputs.end(); i++ ) {
            i->output().set_source_capabilities( cap );
        }
    }
}


std::auto_ptr<Output> 
FilterSource::make_output()
{
    if ( suboutputs.size() == 0 )
        // TODO: Re-add help info "#NoOutputSelected"
        throw std::runtime_error(
            "No output selected for module '" + getDesc() + "'");
    else if ( suboutputs.size() == 1 ) {
        DEBUG(this << ": Have only a single output, calling its make_output()");
        std::auto_ptr<Output> o = 
            suboutputs.begin()->output().make_output();
        DEBUG(this << " made output " << o.get());
        if ( o.get() == NULL )
            throw std::invalid_argument(
                "Output for module '" + getDesc() + "' is invalid");
        else
            return o;
    } else {
        DEBUG(this << ": Have multiple outputs, making crankshaft");
        std::auto_ptr<outputs::Crankshaft> crankshaft
            ( new outputs::Crankshaft() );

        SuboutputChoice::iterator i;
        for( i = suboutputs.begin(); i != suboutputs.end(); i++ ) {
            DEBUG(this << ": Calling subordinate make_output()");
            crankshaft->add( i->output().make_output() );
            DEBUG(this << ": Called subordinate make_output()");
        }

        DEBUG(this << ": Finished, returning " << crankshaft.get());
        if ( ! crankshaft->empty() )
            return std::auto_ptr<Output>(crankshaft);
        else
            throw std::invalid_argument(
                "Outputs for module '" + getDesc() + "' are invalid");
            
    }
}

void FilterSource::for_each_suboutput( boost::function1<void,const OutputSource&> f ) const {
    for( SuboutputChoice::const_iterator i = suboutputs.begin(); i != suboutputs.end(); i++ )
        f( i->output() );
}

}
}
