#include "debug.h"

#include <boost/algorithm/string.hpp>
#include <dStorm/input/InputMutex.h>

#include "FileMethod.h"
#include "chain/FileContext.h"

namespace dStorm {
namespace input {

using namespace chain;

FileMethod::FileMethod()
: simparm::Set("FileMethod", "File"),
  chain::Forwarder(),
  simparm::Listener( simparm::Event::ValueChanged ),
  input_file("InputFile", "Input file"),
  children("FileType", "File type", true)
{
    input_file.helpID = "InputFile";
    children.helpID = "FileType";
    DEBUG("Created file method");
    push_back( input_file );
    push_back( children );
    children.set_auto_selection(false);
    children.value = NULL;
    receive_changes_from( input_file.value );
    chain::Forwarder::set_more_specialized_link_element( &children );
}

FileMethod::FileMethod(const FileMethod& o)
: simparm::Set(o),
  chain::Forwarder(),
  simparm::Listener( simparm::Event::ValueChanged ),
  context(o.context),
  input_file(o.input_file),
  children(o.children)
{
    push_back( input_file );
    push_back( children );
    receive_changes_from( input_file.value );
    chain::Forwarder::set_more_specialized_link_element( &children );

    if ( o.children.isValid() )
        children.choose( o.children.value().getNode().getName() );
}

FileMethod::~FileMethod() {}

void FileMethod::operator()( const simparm::Event& )
{
    ost::MutexLock lock( global_mutex() );
    if ( context.get() && context->input_file != input_file() ) {
        DEBUG("Filename changed, notifying with new context");
        children.value = NULL;
        boost::shared_ptr<chain::FileContext> nc(context->clone());
        nc->input_file = input_file();
        context = nc;
        notify_of_context_change( context );
    } else {
        DEBUG("Not changing context");
    }
}

class BasenameApplier {
    std::string basename;
    bool applied;

  public:
    BasenameApplier( std::string filename ) : basename(filename), applied(false) {}
    void operator()( const std::pair<std::string,std::string>& p ) {
        if ( applied ) return;
        std::string suffix = p.second;
        if ( boost::iequals( basename.substr( basename.length() - suffix.length() ), suffix ) ) {
            basename = basename.substr( 0, basename.length() - suffix.length() );
            applied = true;
        }
    }
    const std::string& get_result() const { return basename; }
};

Link::AtEnd FileMethod::traits_changed( TraitsRef traits, Link* from )
{
    Link::traits_changed( traits, from );
    if ( traits.get() == NULL ) return notify_of_trait_change( traits );
    boost::shared_ptr<chain::FileMetaInfo> my_traits( new chain::FileMetaInfo   
            ( dynamic_cast<const chain::FileMetaInfo&>(*traits) ) );
    if ( my_traits->suggested_output_basename.unformatted()() == "" ) {
        BasenameApplier a( input_file() );
        std::string new_basename = 
            std::for_each( my_traits->accepted_basenames.begin(), 
                        my_traits->accepted_basenames.end(),
                        BasenameApplier( input_file() ) ).get_result();
        my_traits->suggested_output_basename.unformatted() = new_basename;
    }
    my_traits->forbidden_filenames.insert( input_file() );
    return notify_of_trait_change( my_traits );
}

Link::AtEnd FileMethod::context_changed( ContextRef context, Link* from )
{
    Link::context_changed( context, from );
    this->context.reset( new FileContext(*context, input_file()) );

    return notify_of_context_change( this->context );
}

}
}
