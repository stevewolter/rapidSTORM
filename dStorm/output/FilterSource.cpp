#include "debug.h"

#include "FilterSource.h"
#include <simparm/ChoiceEntry.h>
#include <dStorm/outputs/Crankshaft.h>
#include "OutputSource.h"
#include "SourceFactory.h"
#include <dStorm/helpers/clone_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>

namespace dStorm {
namespace output {

class FilterSource::Suboutput {
    boost::clone_ptr<OutputSource> unadorned;
    simparm::Object config_node, removal_node;

public:
    Suboutput(std::auto_ptr< OutputSource > unadorned, std::string ident, simparm::NodeHandle ui )
        : unadorned( unadorned ), config_node( ident, ""),
          removal_node(ident, this->unadorned->getDesc()) 
        { if ( ui ) attach_suboutput_ui(ui); }
    Suboutput* clone() const { return new Suboutput(*this); }

    std::string getName() const { return removal_node.getName(); }
    void attach_ui( simparm::NodeHandle removal_choice_node )
        { removal_node.attach_ui( removal_choice_node ); }
    void detach_ui( simparm::NodeHandle removal_choice_node )
        { removal_node.detach_ui( removal_choice_node ); }
    void attach_suboutput_ui( simparm::NodeHandle at ) {
        unadorned->attach_full_ui( config_node.attach_ui( at ) );
    }

    OutputSource& output() const { return *unadorned; }
};

void FilterSource::attach_children_ui( simparm::NodeHandle at )
{
    my_node = at;

    for ( SuboutputChoice::iterator i = suboutputs.begin(); i != suboutputs.end(); ++i )
        i->attach_suboutput_ui( at );

    if ( factory.get() != NULL ) {
        DEBUG("Registering entries");
        factory->notify_when_output_source_is_available( 
            boost::bind( &FilterSource::add_new_element, boost::ref(*this) ) );
        factory->attach_ui( at );
        DEBUG("Registered entries");
    }
}

FilterSource::FilterSource()
: next_identity(0),
  factory( NULL )
{
}

FilterSource::FilterSource( const FilterSource& o ) 
: OutputSource(o), 
  next_identity(o.next_identity), basename(o.basename),
  factory( NULL ),
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
    std::auto_ptr<Suboutput> so( new Suboutput( src, name, my_node ) );
    so->output().notify_when_destruction_is_desired( boost::bind( &FilterSource::remove_suboutput, this, so.get() ) );
    suboutputs.push_back( so );
}

void FilterSource::remove_suboutput( Suboutput* p ) {
    suboutputs.erase_if( p == &boost::lambda::_1 );
}

void FilterSource::add_new_element() {
    try {
        std::auto_ptr<OutputSource> fresh = factory->make_output_source();
        if ( fresh.get() != NULL ) add( fresh );
    } catch (const std::runtime_error& e) {
        std::cerr << "Unable to set basename: " << e.what() << "\n";
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
    if ( suboutputs.size() == 0 ) {
        // TODO: Re-add help info "#NoOutputSelected"
        throw std::runtime_error(
            "No output selected for module '" + getDesc() + "'");
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
